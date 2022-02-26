<?php
/**
 * functions to create a time diagram for table RESX
 * resource versus time versus device
 * @namespace core::gui::img
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @package f.grTimeDia.inc
 */


require_once ("object.subs.inc");
require_once ("date_funcs.inc");

class timeDiaC{

var $conflarr;	// save conflicts
var $dateArr;	// dates in Unixstamps ...
				// array ( [resource_id] => array(
				//		"s"=>start, 
				//		"e"=>end, 
				//		"n"=>name, 
				//		"p"=>projid, NOT_USED ...
				//		"c"=> conflict (>0 == resource-id)
				//      "r"=> resourceid)
				//		"d"=> device id
				//		"a"=> accept flag 0,1,2,3 => if == 0 : gray line 
var $colorLegArr; // array[ID] = array ( "c"=> color_id_of img_handle for substance legend, ID = [0,1,2,...]
				  //					 "u" => 0,1 is used ??? )
var $devColorID;  // device color array [DEV_ID] = color_id_of img_handle

function timeDiaC(
	&$sql,
	$factor,    // in minutes
	$startori, 	// Unix-stamp
	$endori,	// Unix-stamp
	$dateArr,	// see above
	$optx = NULL // "substid"
				//  "mode" => "", "multi" (more substances)
				//  "debug"
				//  "title" => title of diagram
	) {
	
	$colorArr = 
		array(array(140,80,  190),
		array(0,  255,0  ), // 2
		array(0,  0,  255),
		array(128,0,  0  ),
		array(0,  255,255), // 5
		array(0  ,0  ,0 ),
		array(128,128,0  ),
		array(240, 240, 0),
		array(255,145,145), // 9
		array(0  ,128,0  ), // 10
		array(255,0  ,255), // 11
		array(0  ,0  ,150), // 12
		array(0, 80,80),
		array(128, 0,128),
		array(0, 155,155),
		array(200,255,108), 
		array(170, 0, 170),
		array(128,128,128)  // 18
			);
	
	$this->debug = $optx["debug"];
	$this->conflarr = NULL;
	$this->infox["UnitPix"] 	= 20;
	$this->infox["txtdY"]   	= 14;
	$this->infox["linesdY"] 	= 15;   // step between resource lines
	
	$this->infox["substLegX"] 	= 130;
	if ($factor<=0) $factor 	= 60;
	$this->infox["factor"]  	= $factor;
	$this->dateArr 				= $dateArr;
	$this->infox["numLines"] = sizeof($this->dateArr);
	// $this->devColorIDcurr   = 0;  // current color ID for $colorLegArr
	$this->colorLegArr	= array();
	$this->devColorID   = array();
	
	
	
	$this->infox["facNorm"] = 1.0/($factor*60)*$this->infox["UnitPix"];
	
	if ($factor<=61) $start = mktime( date("H",$startori)-1, 0, 0, date("m",$startori)  , date("d",$startori), date("Y",$startori));
	else $start = mktime(0, 0, 0, date("m",$startori)  , date("d",$startori)-1, date("Y",$startori));
	
	if ($factor<=61) $end = mktime(date("H",$endori)+1, 0, 0, date("m",$endori)  , date("d",$endori), date("Y",$endori));
	else $end = mktime(0, 0, 0, date("m",$endori)  , date("d",$endori)+1, date("Y",$endori));
	
	$this->infox["start"] = $start;
	$this->infox["end"]   = $end;
	$this->infox["tNow"]  = time();
	
	$this->infox["startHum"] = date_unix2datestr($start,2);
	$this->infox["endHum"]   = date_unix2datestr($end,  2);
	$this->infox["timeDiffUnx"] = $end - $start;
	
	$this->infox["timeStepUnx"] = $factor * 60;
	$this->infox["units"] =  $this->infox["timeDiffUnx"] / $this->infox["timeStepUnx"];
	$substid = $optx["substid"];
	if (is_numeric($substid)) $this->infox["substname"] = obj_nice_name($sql, "CONCRETE_SUBST", $substid);
	
	$this->infox["titleDY"] = 40;
	// legend for resource description
	$tmpLegendYsize = $this->infox["txtdY"] * $this->infox["numLines"] + $this->infox["txtdY"] + 15;
	$this->infox["yaxLegYdiff"] =  100; // Y-axis: legend Y diff
	
	$tmpLineDiffY = $this->infox["numLines"] * $this->infox["linesdY"];
	$img_y_size   = $this->infox["yaxLegYdiff"] + $this->infox["titleDY"] + $tmpLineDiffY;
	$this->infox["legendYoff"]  = $img_y_size + 10;
	$img_y_size   = $this->infox["legendYoff"] + $tmpLegendYsize + 2; // new value
	
	$tmpLegend2Ysize = 3 * $this->infox["txtdY"]; // 3 legend entries for: red, gray, black lines
	$this->infox["legend2Yoff"]  = $img_y_size + 10;
	$img_y_size   = $this->infox["legend2Yoff"] + $tmpLegend2Ysize + 2; // new value
	
	$this->infox["offsY"] = $this->infox["titleDY"] + $tmpLineDiffY;  // for X-Axis legend
	$this->infox["sizeY"] = $img_y_size;
	$this->infox["diffX"] = $this->infox["units"] * $this->infox["UnitPix"];
	if ($this->infox["diffX"]<300)  $this->infox["diffX"] = 300;
	// $this->infox["diffY"] = 30; 
	
	$this->infox["devColOffsX"] = 25;
	$this->infox["offsX"] = $this->infox["devColOffsX"] + 5;  // (25) resource-ID + device color (20)
	if ( $optx["mode"]=="multi") $this->infox["offsX"] = 10 + $this->infox["substLegX"];
	
	$this->infox["maxX"]  = $this->infox["offsX"] + $this->infox["diffX"];
	$this->infox["maxY"]  = $this->infox["titleDY"];
	$this->infox["legendXoff"]  = $this->infox["offsX"];
	
	$this->infox["legendXdiff"] = $this->infox["maxX"]-$this->infox["legendXoff"];  // common Legend
	$img_x_size = $this->infox["maxX"] + 5;
	
	$this->imgH = imagecreate($img_x_size, $img_y_size);
	$imgHandle = &$this->imgH;
	
	$color["white"] =  imagecolorallocate($imgHandle, 255,255,255);
	$color["black"] =  imagecolorallocate($imgHandle, 0,  0,  0  );
	$color["red"]   =  imagecolorallocate($imgHandle, 255,0,  0  ); 
	$color["orange"]=  imagecolorallocate($imgHandle, 250, 180, 180  );
	$color["gray"]  =  imagecolorallocate($imgHandle, 128,128, 128); 
	$color["graylite"]=imagecolorallocate($imgHandle, 200,200, 200);
	$color["graylitx"]=imagecolorallocate($imgHandle, 245,245, 245);
	$color["yellow"]=  imagecolorallocate($imgHandle, 220,220, 80); 
	$color["greenlt"]= imagecolorallocate($imgHandle, 200, 255, 200); 
	$this->colors = $color;
	

    foreach( $colorArr as $dummy=>$tmparr) {
        $this->colorLegArr[] = array( "c"=> imagecolorallocate($imgHandle,$tmparr[0],$tmparr[1], $tmparr[2]) ,
									  "u"=> 0 // used ???
									  );
	}
	
	$titles["title"] = "Time diagram for resources: ".$this->infox["substname"]." ". $optx["title"];
	$titles["x"] = "Time";
	$titles["y"] = "Resource";
	$titles["t2"] = "Start: ". date_unix2datestr($start,2)." End: ".date_unix2datestr($end,2);
	$this->graph_titles($titles);
	
	$xaxis[0] = date_unix2datestr($start,2);
	if ($factor<=60)    $xaxis[0] = date ("H", $start);
	if ($factor>=60*24) $xaxis[0] = date ("d.M.", $start);
	$xdata=array("x"=>$xaxis);
	$this->graph_axis($xdata);
}


function graph_axis( $data = NULL // ["x"], ["y"] [0]=min [1]=max
     ) {
   
   // X- axis
   $thisFontSize = 3;
   $fontSizMin   = 2;
   $gray    = $this->colors["gray"];
   $colLite = $this->colors["graylite"];
   $colYel  = $this->colors["yellow"];
   $infox = &$this->infox;
   
   $x0= $infox["offsX"];
   $x1= $infox["maxX"];
   $y = $infox["offsY"];
   $timeStringY    = $infox["offsY"] + $infox["yaxLegYdiff"] -20;
   $timeDayStringY = $infox["offsY"] + $infox["yaxLegYdiff"] -15;
   imageLine ( $this->imgH, $x0, $y, $x1, $y,  $gray);
   
   $yoff = $infox["offsY"];
   $y0   = $infox["offsY"]+3;
   $y1   = $infox["offsY"]-3;
   $yl   = array();
   $yl[0] = $infox["offsY"]+6;
   $yl[1] = $infox["offsY"]-6;
   $daylast = 0;
   $xnow = $this->getPixFromDate($infox["tNow"]);
   if ($xnow>0) {
   		// time line TODAY
   		imageLine ( $this->imgH, $xnow, $infox["offsY"], $xnow, $infox["maxY"],  $this->colors["greenlt"]);
		imageLine ( $this->imgH, $xnow+1, $infox["offsY"], $xnow+1, $infox["maxY"],  $this->colors["greenlt"]);
   } 
   
   while ($i<$this->infox["units"]) {
   
   		$xcolor = $gray;
		$x = $infox["offsX"] + $this->infox["UnitPix"] * $i;
		$flagLong = 0;
		
		$timed1 = $infox["start"] + $infox["factor"]*60*$i;
		$day     = date("d",$timed1);
		$hour    = date("H",$timed1);
		$month   = date("m",$timed1);
		$weekday = date("w",$timed1); // 0..6
		if ($infox["factor"]<=60) {
			
			if ($hour>18 OR $hour<8)  $xcolor = $colLite;
			$timeOut = $day.". ".$hour."h";
			if ((int)$hour==0)  $flagLong = 1;
		
		} elseif ($infox["factor"]<24*60)  {
		 	$timeOut = $day.".".$month." ".$hour."h";
			if ($daylast != $day)  $flagLong = 1; 
			if ($weekday==0 OR $weekday==6) $xcolor = $colYel;
		} else {
			if ($weekday==1)  $flagLong = 1;
			$timeOut = $day.".".$month.".";
			if ($weekday==0 OR $weekday==6) $xcolor = $colYel;
		}
		if ($flagLong) imageLine ( $this->imgH, $x, $yl[0], $x, $yl[1],  $gray);
		else imageLine ( $this->imgH, $x, $y0, $x, $y1,  $gray);
		
		imagestringup( $this->imgH, $thisFontSize, $x-4, $timeStringY , $timeOut, $xcolor); // $yoff+70
		if ($daylast != $day) {
			$weekdayNic = date("D",$timed1); 
			imagestring( $this->imgH, $fontSizMin, $x-4, $timeDayStringY, $weekdayNic, $xcolor); //$yoff+75
		}
		
		$daylast = $day;
		$i++;
   }
   

  
} 
	
function graph_titles($titles // "title", "t2"
	) {

	$thisFontSize = 3;
    $letterSize = 7;
    $color = $this->colors["black"];
	$imgHandle = &$this->imgH;
     
    if ( $titles["title"] != "" ) {
        imageString( $imgHandle, $thisFontSize, 5, 3, $titles["title"], $color);
    }
	if ( $titles["t2"] != "" ) {
        imageString( $imgHandle, $thisFontSize, 5, 3+$this->infox["txtdY"], $titles["t2"], $this->colors["gray"]);
    }

}

function setSubstArr($substarr) {
	$colx = $this->colors["gray"];
	$thisFontSize = 2;
	$infox = &$this->infox;
	$index = 0;
	foreach( $substarr as $key=>$xname) {
		$y = $index * 20 + $infox["maxY"];
		imageString( $this->imgH, $thisFontSize, $x0+2, $y-5, $xname, $colx);
		$index++;
	}
}

function setDeviceColors( 
	$colorArr // array[DEVICE-ID] = ID of  $this->colorLegArr
	) {
	// FUNCTION: // set predefined device colors
	
	// set color-IDs
	foreach( $colorArr as $deviceid=>$colorLegArrId) {
		$this->devColorID[$deviceid] = $this->colorLegArr[$colorLegArrId]["c"];
		$this->colorLegArr[$colorLegArrId]["u"] = 1; // color is used now
	}
	reset ($colorArr);
}

function getDevColor($devid) {
	// RETURN: color_id of image handle
	if ( !isset($this->devColorID[$devid]) ) {
	
		// get new color-ID, search for a free value
		foreach( $this->colorLegArr as $loopid=>$colarr) {
			if (!$colarr["u"]) break;
		}
		reset ($this->colorLegArr);
		
		// $nowID = $this->devColorIDcurr;
		$this->devColorID[$devid] = $this->colorLegArr[$loopid]["c"];
		$this->colorLegArr[$loopid]["u"]=1; // color is used now
		
	}
	$colorImgHndl = $this->devColorID[$devid];
	return ($colorImgHndl);
}

function setArrow($x, $y, $color, 
	$direct // 0  : |==>
			// 1  : ==>|
	) {
	// FUNCTION: set graph ARROWS for actions, which have not a START or a END date
	$pxarr = array(
		 0,0,
		-1,0,
		-1,1,
		-1,-1,
		-2,0,
		-2,2,
		-2,-2,
		-3,3,
		-3,-3
	);
	$xdiff = 15;
		
	if (!$direct) {
		$dirfac= 1;
		
	} else {
		$dirfac= -1;
		
	}
	
	// imageLine ( $this->imgH, $x, $y, $x+$xdiff, $y, $color);
	$i=0;
	$xnow  = $x;
	$steps = 7;
	while ( $i<$steps ) {
		$xnow = $xnow+2*$dirfac;
		imagesetpixel( $this->imgH, $xnow, $y, $color);
		$i++;
	}
	
	$i=0;
	$xnew = $x+$xdiff;
	if ($direct) $xnew = $x-1;
	while ( $i<9 ) {
		$xl = $pxarr[$i*2];
		$yl = $pxarr[$i*2+1];
		imagesetpixel( $this->imgH, $xnew+$xl, $y+$yl, $color);
		$i++;
	}
}
	//

// 1 unit = 20 pixels
function calculate(  
	$index	    // index of substance 0....n	
	) {
	// FUNCTION:- draw time lines
	//			- suport also time-events, where START=NULL or END=NULL
	$infox = &$this->infox;
	
	
	$colx 		 = $this->colors["black"];
	$colorText   = $this->colors["black"];
	$colConflict = $this->colors["red"];
	$colorLite   = $this->colors["graylite"];
	$colorGray   = $this->colors["gray"];
	$thisFontSize= 2;
	$stripeY     = 3;
	
	$drec = array();
	$drec[0] = $this->infox["devColOffsX"];
	$drec[2] = $this->infox["devColOffsX"]+5;
	
	if ( !sizeof($this->dateArr) ) return; 
	$style = array( $colorGray, $this->colors["white"] );
	imagesetstyle( $this->imgH, $style );
	
	$i=0;
	foreach( $this->dateArr as $key=>$tarr) {
		$lineFlag = 1;
		$y 		  = $i * $this->infox["linesdY"] + $infox["maxY"];
		$colx1    = $colx;
		$startori = $tarr["s"];
		$end      = $tarr["e"];
		if (!$tarr["a"] ) $colx1    = $colorGray; // not accepted ???
		if ($tarr["c"]>0) {
			$colx1 = $colConflict;
			// $this->conflarr[$key] = $tarr["c"]; // save the conflict
		}
		#echo "$i. endbef:$end  stbef:$start \n";
		$resStringOutX = 2; // RESOURCE PLAN ID
		$start=$startori;
		if ($startori < $infox["start"]) $start=$infox["start"];
		if ($end  >$infox["end"])   	 $end=  $infox["end"];
		if ($end  <$infox["start"]) 	 $end=  $infox["start"];
		
		$x0 = ($start-$infox["start"]) * $infox["facNorm"] + $infox["offsX"];
		$x1 = ($end  -$infox["start"]) * $infox["facNorm"] + $infox["offsX"];
		
		imageLine ( $this->imgH, $infox["offsX"], $y, $infox["offsX"] + $infox["diffX"], $y, $colorLite); // help line
		
		if ($tarr["s"]=="") {
			// $x0 = $x1 - 15;
			$this->setArrow($x1, $y, $colx1, 1);
			// imageLine ( $this->imgH, $x0, $y,   $x1, $y,  IMG_COLOR_STYLED);
			//imageLine ( $this->imgH, $x0, $y-2, $x0, $y+2, $colx1);
			imageLine ( $this->imgH, $x1, $y-$stripeY, $x1, $y+$stripeY, $colx1);
			$lineFlag = 0;
		}
		if ($tarr["e"]=="") {
			//$x1 = $x0 + 15;
			$this->setArrow($x0, $y, $colx1, 0);
			//imageline ( $this->imgH, $x0, $y,   $x1, $y, IMG_COLOR_STYLED);
			imageLine ( $this->imgH, $x0, $y-$stripeY, $x0, $y+$stripeY, $colx1);
			//imageLine ( $this->imgH, $x1, $y-2, $x1, $y+2, $colx1);
			$lineFlag = 0;
		}
		
		if ($this->debug>1) {
			echo "$i. startHum: ".date_unix2datestr($startori,2)." stOri:$startori endNew:$end  0:$x0 1:$x1 y:$y:$colx flag:$lineFlag<br>\n";
		} 
		
		if ( $lineFlag ) {
			imageLine ( $this->imgH, $x0, $y,   $x1, $y,   $colx1);
			imageLine ( $this->imgH, $x0, $y-$stripeY, $x0, $y+$stripeY, $colx1);
			imageLine ( $this->imgH, $x1, $y-$stripeY, $x1, $y+$stripeY, $colx1);
		}
		imageString( $this->imgH, $thisFontSize, $resStringOutX, $y-5, $tarr["r"], $colorText); // resource-ID
		
		// device-ID
		if ($tarr["d"]) {
			$devid = $tarr["d"];
			$drec[1]  = $y - 3;
			$drec[3]  = $y + 3;
			$colorDev = $this->getDevColor($devid);
			imageFilledRectangle( $this->imgH, $drec[0], $drec[1], $drec[2], $drec[3], $colorDev); // resource-ID
			
		}
		$i++;
	}
	reset($this->dateArr);
	
}

function getPixFromDate($dateUnix) {
	// returns x
	$infox = &$this->infox;
	if ($dateUnix > $infox["end"]   ) return;
	if ($dateUnix < $infox["start"] ) return;
	
	$x = ( $dateUnix-$infox["start"] ) * $infox["facNorm"] + $infox["offsX"];
	return ($x);
}


function legend( $legendarr  // array($id => name)
	//  $this->infox["legendXoff"]
	//  $this->infox["legendXdiff"] +
	){
	$infox = &$this->infox;
	$colx = $this->colors["gray"];
	$thisFontSize = 2;
	
	$drec = array();
	$drec[0] = $infox["legendXoff"]+3;
	$drec[2] = $infox["legendXoff"]+8;
	
	imageFilledRectangle( $this->imgH, 
			$infox["legendXoff"], $infox["legendYoff"], 
			$infox["legendXoff"] + $infox["legendXdiff"], $this->infox["sizeY"]-2, $this->colors["graylitx"]);
	imageRectangle( $this->imgH, 
			$infox["legendXoff"], $infox["legendYoff"], 
			$infox["legendXoff"] + $infox["legendXdiff"], $this->infox["sizeY"]-2, $this->colors["graylite"]);
			
	$i=0;
	$yoff = $infox["legendYoff"] + 3;
	$x0   = $infox["legendXoff"] + 17;
	$x1   = $x0+20;
	
	imageString( $this->imgH, 3, $x0, $yoff, "Legend: Resource-ID, Device, Plan name", $colx);
	$yoff = $yoff + $infox["txtdY"] + 5;
	
	foreach( $legendarr as $res_id=>$xval) {
	
		$y = $infox["txtdY"]*$i + $yoff;
		imageString( $this->imgH, $thisFontSize, $x0, $y, $res_id.": ".$xval, $colx);
		$devid = $this->dateArr[$res_id]["d"];
		
		if ( $devid ) {
			$drec[1]  = $y - 3 +6;
			$drec[3]  = $y + 3 +6;
			$colorDev = $this->getDevColor($devid);
			imageFilledRectangle( $this->imgH, $drec[0], $drec[1], $drec[2], $drec[3], $colorDev); // resource-ID
		}
		$i++;
	}
	
	$y  = $this->infox["legend2Yoff"];
	$x0 = $infox["legendXoff"]+3;
	$x1 = $x0 + 10;
	$xText = $x1 + 5;
	$colorBlack = $this->colors["black"];
	$tmparr = array("black" => "reviewed", "gray" => "not reviewed", "red" => "usage conflict");
	foreach( $tmparr as $key=>$xval) {
		$colx1 = $this->colors[$key];
		imageLine ( $this->imgH, $x0, $y,   $x1, $y,   $colx1);
		imageLine ( $this->imgH, $x0, $y-2, $x0, $y+2, $colx1);
		imageLine ( $this->imgH, $x1, $y-2, $x1, $y+2, $colx1);
		imageString( $this->imgH, $thisFontSize, $xText, $y-6, $xval, $colx);
		$y = $y + $infox["txtdY"];
	}
}

function show() {
	
	if ( $this->debug < 2 ) {
        Header("Content-type: image/png");
    } else {
	    echo 'DEBUG-Level: '.$this->debug.'<br>';
		echo "INFOX:<pre>";
		print_r($this->infox);
		echo "</pre>";
    }  
	imagePNG($this->imgH);
	ImageDestroy($this->imgH);
}
}