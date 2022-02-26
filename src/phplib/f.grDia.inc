<?php
require_once ('f.grImgSub.inc');

/**
 * - common diagram-graphic class
   - lines, scatter
 * @package f.grDia.inc
 * @swreq   UREQ:0001674: g > diagramm creation lib (scatter,line) 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com) 
 */
class fgrDiaC {
var $imgH;
var $colors;
var $colorDef; // default
var $legend;   /* "st", "is" */
var $infox; /*
		"numVals" : number of data-tupels
	*/
var $dataArr; // values, index starts at 0 !!!
var $xAx;     // X-axis-values, index starts at 0 !!!

function __construct(
	$titleArr, // array("title, "x", "y")
	&$dataArr, // - can be NULL for scatter plot ...
			   // "type" = "line"   : array[0..n] = value
			   // "type" = "scatter": array[0] = $x_array
			   //					: array[1] = $y_array
			   // "type" = "bar"    : array[0..n] = value
	$grOpt,    // "type" = ["line"], "scatter", "bar" )
			   // "maxX" => scale maxX
			   // "minY"
			   // "maxY" => scale maxY
			   // "axisYLine" => 0,1 draw horizonal help lines
			   // "titleDY" => Y-offset TOP border
			   // "drawAreaPix" => Y-pixelsize of draw area
			   // "UnitPix"    => 5 ...
			   // "LegendMain" => Pixels: extra-legend on UpperRight Side
			   // "multiSet"   => 0,1 : is a multi dataArr (for color-Index
	$xAx = NULL       // array[0..n] = x_value
	) {
	global $error;
	
	$nowdebug = $_SESSION["userGlob"]["g.debugLevel"];
	if ( $nowdebug<=1 ) {
    	Header("Content-type: image/png");
	}
	
	if ($grOpt["type"]=="") $grOpt["type"] = "line";
	$this->grOpt = $grOpt;
	
    $infox["numVals"] = sizeof($dataArr);
    $infox["XmaxVal"] = 1;
	$infox["YminVal"] = 0;
	$infox["YmaxVal"] = 1;
	$infox["fontXpixel"] = 7; // pixel size
	$obj_max_len = 6; // default number-type
	
	if ($grOpt["type"]=="line" OR $grOpt["type"]=="bar") {
		$this->xAx = $xAx;
		if (!sizeof($dataArr) ) $infox["numVals"] =  sizeof($this->xAx);
		if ( sizeof($dataArr) ) $infox["YmaxVal"] = max($dataArr);
		$obj_max_len = $this->_getxAxExtreme(); // for x-axis-unit-names
	}
	if ($grOpt["type"]=="scatter") {
		if ($grOpt["maxX"]=="") $infox["XmaxVal"] = max($dataArr[0]);
		if ($grOpt["maxY"]=="") $infox["YmaxVal"] = max($dataArr[1]);
	}
	if ($grOpt["maxX"]>0)  $infox["XmaxVal"] = $grOpt["maxX"];
	if ($grOpt["maxY"]>0)  $infox["YmaxVal"] = $grOpt["maxY"];
	if ($grOpt["minY"]>0)  $infox["YminVal"] = $grOpt["minY"];
	
	if ($infox["YmaxVal"]<=0) {
		$error->set("fgrDiaC",1, "Y-value is <= ZERO");
		return;
	} 
	if ( $infox["YmaxVal"] < $infox["YminVal"] ) {
		$error->set("fgrDiaC",1, "YMinVal > YMaxVal");
		return;
	} 
	$this->dataArr = &$dataArr;
	
	
	$infox["YDiaAreaMax"] = 300; // 300 Pixels diagram-data-area in Y
	$infox["XDiaAreaMax"] = 310; // 300 Pixels diagram-data-area in X
	$infox["XDiaAreaMin"] = 300; // 300 Pixels diagram-data-area in X
	
	if ( $grOpt["drawAreaPix"] > 0 ) $infox["YDiaAreaMax"] = $grOpt["drawAreaPix"];
	$infox["UnitPix"] 	    = 5;
	if ( $grOpt["UnitPix"] > 0 ) $infox["UnitPix"] = $grOpt["UnitPix"];
	
	$infox["YxaxTitleDiff"] = 30; // pixel height of Xax title
	$infox["YxaxTitleDiff"] = 30 + $obj_max_len * $infox["fontXpixel"];
	$infox["YValDiff"] = $infox["YmaxVal"] -  $infox["YminVal"];
	$infox["YvalFact"] = $infox["YDiaAreaMax"] / $infox["YValDiff"];
	
	$infox["offsX"]   = 70;
	$infox["titleDY"] = 20;	// Y-offset for diagram from TOP
	if ( $grOpt["titleDY"] > 0 ) $infox["titleDY"] = $grOpt["titleDY"];
	
	if ($grOpt["type"]=="line" OR $grOpt["type"]=="bar") {
    	$infox["maxX"]    = $infox["offsX"] + $infox["UnitPix"] * $infox["numVals"];
		if ($infox["maxX"]<$infox["XDiaAreaMin"] ) {
			$infox["maxX"]=$infox["XDiaAreaMin"];
			$infox["UnitPix"] = ($infox["maxX"]-$infox["offsX"]) / $infox["numVals"];
		}
	}
	if ($grOpt["type"]=="scatter") {
		$infox["XvalFact"] = $infox["YDiaAreaMax"] / $infox["XmaxVal"];
		$infox["maxX"]     = $infox["offsX"] + $infox["YDiaAreaMax"];
	}
	
    $infox["maxY"]    = $infox["titleDY"] ;
	$infox["offsY"]   = $infox["maxY"] + $infox["YDiaAreaMax"];
	$infox["imgXlen"] = $infox["maxX"] + $grOpt["LegendMain"] + 5;
	$infox["imgYlen"] = $infox["offsY"] + $infox["YxaxTitleDiff"];
	$this->legend=NULL;
	$this->legend ["st"] = 15;
	$this->legend ["is"] = 0;
	
	if ($nowdebug>1) {
		echo "DEBUG: infox[]:<pre>";
		print_r($infox);
		echo "</pre>\n";
	}
	
	
	
	$this->imgH = imagecreate( $infox["imgXlen"], $infox["imgYlen"] );
	if ( !$this->imgH )  {
		$error->set("fgrDiaC",2, "Could not create an image.");
		return;
	}
	// $imNow = &$this->imH;
	$this->colors["white"] = ImageColorAllocate($this->imgH, 255,255,255); 
	$this->colors["black"] = ImageColorAllocate($this->imgH, 0,0,0);
	$this->colors["gray"]  = ImageColorAllocate($this->imgH, 130,130,130);
	$this->colors["gray2"]  = ImageColorAllocate($this->imgH, 200,200,200);
	$this->colors["gray3"]  = ImageColorAllocate($this->imgH, 240,240,255);
	$this->colors["blue"]  = ImageColorAllocate($this->imgH, 0,0,255);
	$this->colors["bluelite"]=ImageColorAllocate($this->imgH, 100,100,255);
	$this->colors["green"] = ImageColorAllocate($this->imgH, 0,200,0);
	$this->colors["red"]   = ImageColorAllocate($this->imgH, 255,0,0);
	$this->colorDef = $this->colors["black"];
	$this->infox = $infox;
	
	$this->grImgSublib = new fImageCreaC( $this->imgH );
	if ( $this->grOpt["multiSet"] ) $this->setMultiColors();
	
	$this->graph_axis();
	$this->graph_titles( $titleArr );

	// axis
	// title
}

function drawLegendOne(
	$textLine /* array of text
			       array("t" => text, "ci"=> color-index)
			*/
	) {
	$step = 10;
	$thisFontSize = 3;
	
	$x = $this->infox["maxX"] + 8;
	$y = $this->legend ["st"] + $this->legend ["is"]*$step;
	$color = $this->colorDef;
	$textout = $textLine["t"];
	if ($textLine["ci"]!="") $color = $textLine["ci"];
	
	imagestring( $this->imgH, $thisFontSize, $x, $y, $textout, $color);
	
	$this->legend ["is"] = $this->legend ["is"] + 1;
	
}

function drawLegend(
	$textarr, /* array of text
			     or array of array("t" => text, "c"=> color)
			  */
	$fontSize = 3
	) {
	
	$yStep        = 12;
	
	$x = $this->infox["maxX"] + 10;
	$y = $this->infox["maxY"];
	
	$ylegMax  = $y + sizeof($textarr)*$yStep + 4;
	$reccoord = array( $x-4, $y-4, $this->infox["imgXlen"]-4, $ylegMax);
	imageFilledRectangle( $this->imgH, $reccoord[0],$reccoord[1],$reccoord[2],$reccoord[3], 
						  $this->colors["gray3"] );
	
	foreach( $textarr as $dummy=>$text) {
		$color = $this->colors["black"];
		if (is_array($text)) {
			 $textout = $text["t"];
			 if ($text["c"]!="") $color   = $this->colors[$text["c"]];
		} else $textout = $text;
		imagestring( $this->imgH, $fontSize, $x, $y, $textout, $color);
		$y=$y+$yStep;
	}
	reset($textarr);
}

function xAxInvalid($pos, $showStrip=0) {
	// red cross
	$infox  = &$this->infox;
	$xcolor = $this->colors["red"];
	$x = $infox["offsX"] + $pos * $this->infox["UnitPix"];
	$x0 = $x-3;
	$x1 = $x+3;
	$yoff = $infox["offsY"];
	$y0   = $infox["offsY"]+3;
	$y1   = $infox["offsY"]-3;
	imageLine ( $this->imgH, $x0, $y1, $x1, $y0, $xcolor); // cross
	imageLine ( $this->imgH, $x0, $y0, $x1, $y1, $xcolor); // cross
	if ( $showStrip ) {
		imageLine ( $this->imgH, $x, $yoff, $x, $infox["maxY"], $this->colors["gray2"]); // y-line
	}
}

function _getxAxExtreme() {
	// FUNCTION: - get object name length
	//			 - analyse $this->xAx
	// OUTPUT:  $this->infoxz["obj_max_len"];
	
	if ( !sizeof($this->xAx)) {
		return (20); // default
	}
	
	$maxall = 0;
	foreach( $this->xAx as $dummy=>$val) {	
		$namelen = strlen($val); // analyse first element
		$maxall  = max($namelen, $maxall); 
	}
	reset($this->xAx);
	return ($maxall);
}

function _getYpix($yval) {
	$ypix = $this->infox["offsY"] - ($yval-$this->infox["YminVal"]) * $this->infox["YvalFact"];
	return ($ypix);
}

function graph_axis() {
   
   
	$thisFontSize = 3;
	$fontSizMin   = 2;
	$xcolor= $this->colors["gray"];
	
	$infox = &$this->infox;
	
	$x0= $infox["offsX"];
	$x1= $infox["maxX"];
	$y = $infox["offsY"];
	$y1= $infox["maxY"];
	imageLine ( $this->imgH, $x0, $y, $x1, $y,  $xcolor); // X-axis
	imageLine ( $this->imgH, $x0, $y, $x0, $y1, $xcolor); // Y-axis

	
	imageLine ( $this->imgH, $x0, $y1, $x1, $y1,  $xcolor); // X2-axis
	imageLine ( $this->imgH, $x1, $y, $x1, $y1, $xcolor); // Y2-axis
	
	
	// X- axis
	if ($this->grOpt["type"]=="scatter") {
	
		$i=0;
		$step = ($infox["XmaxVal"]*0.1);
		$y0 = $infox["offsY"]-3;
		$y1 = $infox["offsY"]+3;
		while ( $i < $infox["XmaxVal"] ) {
			$x = $infox["offsX"] + $i * $infox["XvalFact"];
			imageLine ( $this->imgH, $x, $y0, $x, $y1,  $xcolor); // tick
			$numout = sprintf("%.2f", $i);
			imagestringup( $this->imgH, $thisFontSize, $x-5, $y0+40, $numout, $xcolor);
			$i=$i+$step;
		}
	} else {
		// normal line of bar graph
		$yoff = $infox["offsY"];
		$y0   = $infox["offsY"]+3;
		$y1   = $infox["offsY"]-3;
		$yText= $infox["imgYlen"]-20;
		$i=0;
		while ( $i < $this->infox["numVals"] ) {
			if ($this->xAx[$i]!="" ) {
				$x = $infox["offsX"] + $this->infox["UnitPix"] * $i;
				imageLine ( $this->imgH, $x, $y0, $x, $y1,  $xcolor); // tick
				imagestringup( $this->imgH, $thisFontSize, $x-4, $yText, $this->xAx[$i], $xcolor); // $yoff+70
			}
			$i++;
		}
	}
	
	// y-axis
	
	$yval = $infox["YminVal"];
	$step = ( $infox["YValDiff"] * 0.1 );
	$x0 = $infox["offsX"]-3;
	$x1 = $infox["offsX"]+3;
	$xHelpEnd = $infox["maxX"]; // $infox["XmaxVal"];
	$xHelpC1  = $this->colors["gray2"];
	$xHelpC2  = $this->colors["gray3"];
    $cnt=0;
	
	while ( $yval < $infox["YmaxVal"] ) {
		$y =  $this->_getYpix($yval);
		imageLine ( $this->imgH, $x0, $y, $x1, $y,  $xcolor); // tick
		$numout = sprintf("%.2f", $yval);
		imagestring( $this->imgH, $thisFontSize, $x0-40, $y, $numout, $xcolor);
		
		if ($this->grOpt["axisYLine"] and $cnt>0) {
			$xHelpCol = $cnt & 1 ? $xHelpC1: $xHelpC2;
			imageLine ( $this->imgH, $x0, $y, $xHelpEnd, $y, $xHelpCol); // tick
		}
		$yval=$yval+$step;
		$cnt++;
	}
   

  
} 

/**
 * draw titles on axis and main
 * @param array $titles : "title", "x", "y", 
 * 	"titlearr"=>array( of subtitle-strings ) but set: grOpt["titleDY"]
 */
function graph_titles( $titles 
	) {

	$thisFontSize = 3;
    $letterSize   = 7;
    $letterHeight = 12;
    $color = $this->colors["black"];
	$imgHandle = &$this->imgH;
     
    if ( $titles["title"] != "" ) {
        imageString( $imgHandle, $thisFontSize, 5, 3, $titles["title"], $color);
    }
	if ( $titles["y"] != "" ) {
        imageStringup( $imgHandle, $thisFontSize, 5, $this->infox["imgYlen"]*0.5 , $titles["y"], $this->colors["gray"]);
    }
	if ( $titles["x"] != "" ) {
		$strPixLen = strlen($titles["x"]) * $letterSize * 0.5;
        imageString( $imgHandle, $thisFontSize, $this->infox["maxX"]*0.5-$strPixLen, $this->infox["imgYlen"] - 15 , $titles["x"], $this->colors["gray"]);
    }
    
    if (is_array($titles["titlearr"])) {
    	reset ($titles["titlearr"]);
    	$yloop = $letterHeight + 5;
    	foreach( $titles["titlearr"] as $dummy=>$oneTitle) {
    		 imageString( $imgHandle, $thisFontSize, 5, $yloop, $oneTitle, $color);
    		 $yloop = $yloop + $letterHeight;
    	}
    	 
    }

}

function setMultiColors() {
	/*FUNCTION: for multi-color-modes */
	
	$posarr = range(0,15);
	$this->colorIdx = $this->grImgSublib->assign_colors( $posarr );
	// glob_printr( $this->colorIdx, "colorIdx info" );
}

function setDataArr( &$dataarr, 
	$color, // "color"
	$name	// -- name of dataArray
	) {
	$this->dataArr  = &$dataarr;
	$this->colorDef = $this->colorIdx[$color];
	$this->dataName = $name;
	//glob_printr( $dataarr, "name:$name  info" );
}

/**
 * set data by color-name
 * @param  $dataarr
 * @param  $color -- name of color; e.g. "gray"
 * @param  $name  -- name of dataArray
 */
function setDataArrColor( &$dataarr, $color,$name) {
	$this->dataArr  = &$dataarr;
	$this->colorDef = $this->colors[$color];
	$this->dataName = $name;
}

/**
 * draw data-lines
 * - global input: $this->dataArr (index start at 0)
 */
function dataDrawLine(  ) {
	$i=0;
	$infox   = &$this->infox;
	$numvals = $infox["numVals"];
	$x 		 = $infox["offsX"];
	$YvalFact= $infox["YvalFact"];
	$xcolor  = $this->colorDef;
	$valxLast= NULL;
	$xLast   = $infox["offsX"]; 
	$YminVal = $infox["YminVal"];
	
	while ( $i < $numvals ) {
	
   		$valx = $this->dataArr[$i];
		// imageStringUp( $this->imgH, 3, $x,  $infox["offsY"]-5, $valx, $this->colors["gray"] );
  
		if ( $valx!==NULL ) {
			$yPix = $infox["offsY"] - ($valx-$YminVal) * $YvalFact;
			if ($valxLast!==NULL) { 
				imageLine ( $this->imgH, $xLast, $yLast, $x, $yPix,  $xcolor); // line
			} else {
				// cross
				imageLine ( $this->imgH, $x, $yPix-3, $x, $yPix+3,  $xcolor);
				imageLine ( $this->imgH, $x-3, $yPix, $x+3, $yPix,  $xcolor);
			}
		}
		$valxLast = $valx;
		$yLast = $yPix;
		$xLast = $x;
		
		$x = $x + $this->infox["UnitPix"];
		$i++;
   }
   
   if ( $this->grOpt["multiSet"] ) 
   $this->drawLegendOne( array("t"=>$this->dataName, "ci"=>$this->colorDef));
}

/**
 * draw bargraph
 * @param  $pos
 * @param  $dataarr
 * @param  $option
 */
function dataDrawBar(
			$pos=0,         // give pos, if more than 1 bar per position
			$dataarr= NULL,  // optional dataarr
			$option = NULL  // "color" => "black", "green", "red"
			) {
	// FUNCTION draw
	$i=0;
	$infox   = &$this->infox;
	$numvals = $infox["numVals"];
	$x 		 = $infox["offsX"] + $pos*5;
	$YvalFact= $infox["YvalFact"];
	$xcolor  = $this->colors["black"];
	if ($option["color"]!="")  $xcolor  = $this->colors[$option["color"]];
	$valxLast= NULL;
	$xLast   = $infox["offsX"];
	$YminVal = $infox["YminVal"]; 
	
	$useData = &$this->dataArr;
	if ($pos>0) $useData = &$dataarr;
	
	while ( $i < $numvals ) {
   		$valx = $useData[$i];
		// imageStringUp( $this->imgH, 3, $x,  $infox["offsY"]-5, $valx, $this->colors["gray"] );
  
		if ( $valx !== "" ) {
			$yPix = $infox["offsY"] - ($valx-$YminVal) * $YvalFact;
			imageFilledRectangle ( $this->imgH, $x-3, $yPix, $x+3, $infox["offsY"], $xcolor); // tick
		}
		
		$x = $x + $this->infox["UnitPix"];
		$i++;
   }
}

/**
 * - SCATTER plot
   - can be called more than once
 * @param  $option 
 * 		"color" => "black", "green", "red"
		"optdata" => 0,1 : use $dataarr instead of $this->dataArr
		"scatLine" => 0: do cross, 1: do linees between points
 * @param  $dataarr - optional dataarr
 */
function dataDrawScatter(
	$option = NULL,
	$dataarr= NULL  
	) {
	
	$i=0;
	$infox   = &$this->infox;
	$action='cross';
	if ($option["scatLine"]>0) $action='line';
	
	$crosslen=1; // length of cross line
	
	if ($option["optdata"]>0) {
		$usedata = $dataarr;
	} else {
		$usedata = &$this->dataArr;
	}
	$numvals = $infox["numVals"];

	$YvalFact= $infox["YvalFact"];
	$XvalFact= $infox["XvalFact"];
	$xcolor  = $this->colors["black"];
	if ($option["color"]!="") $xcolor  = $this->colors[$option["color"]]; 
	$YminVal = $infox["YminVal"]; 
	
	if (!sizeof($usedata[0])) return;
	
	//glob_printr( $usedata, "usedata_XXX" );
	// echo "DEBUG_XX_sizeData:".sizeof($usedata[0])."<br>";
	if ($action=='line') {
		list($pos, $valx)= each( $usedata[0] );
		$valy = $usedata[1][$pos];
		$lastX = $infox["offsX"] + $valx * $XvalFact;
		$lastY = $infox["offsY"] - ($valy-$YminVal) * $YvalFact;
	}
	
	reset($usedata[0]);
	foreach( $usedata[0] as $pos=>$valx) {
	
   		$valy = $usedata[1][$pos];
		
		if ( $valx != "" ) {
			$xPix = $infox["offsX"] + $valx * $XvalFact;
			$yPix = $infox["offsY"] - ($valy-$YminVal) * $YvalFact;
	
			// draw a cross
			if ($action=='cross') {
				imageLine ( $this->imgH, $xPix, $yPix-$crosslen, $xPix, $yPix+$crosslen,  $xcolor); // tick
				imageLine ( $this->imgH, $xPix-$crosslen, $yPix, $xPix+$crosslen, $yPix,  $xcolor); // tick
			} else  {
				imageLine ( $this->imgH, $lastX, $lastY, $xPix, $yPix,  $xcolor); 
				$lastX = $xPix;
				$lastY = $yPix;
			}
		}
		
		$i++;
   }
   reset ( $usedata[0] );
}

function imgOut() {
	Imagepng    ( $this->imgH );
	
}

function close() {
	ImageDestroy( $this->imgH );
	$this->imgH = NULL;
}

function errorOut($text) {
	// FUNCTION: show error text as graphics
	$imgH = imagecreate( 500, 40 );
	if ( !$imgH )  {
		echo "ERROR: $text\n";
	}
	// $imNow = &$this->imH;
	$white = ImageColorAllocate($imgH, 255,255,255); 
	$red   = ImageColorAllocate($imgH, 255,0,0);
	imagestring( $imgH, 3, 4, 15, "ERROR: $text", $red);
	Imagepng    ( $imgH );
	ImageDestroy( $imgH );	
	$imgH = NULL;
}

}
