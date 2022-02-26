<?php
require_once ("object.subs.inc");
require_once ("date_funcs.inc");


/**
 * functions to create a time diagram for many events of one object; e.g. REA_LOG
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @package f.grTimeEventsDia.inc
 * @version $Header: trunk/src/phplib/f.grTimeEventsDia.inc 59 2018-11-21 09:04:09Z $
 */
class fTimeEventsDiaC{

var $dateArr;	
/* save many events of an object: dates in Unixstamps ...
  array[OBJ_ID] =  (_oneDeviceAllLine_STRUCT) array(
	[POS] => array(
		"s"=> start time, 
		"e"=> end time, 
		"k"=> key, 	
		"x"=> "notes",
		"c"=> color: 'r':red, 'g':green, 'b':blue, 'bc':black
	 )
	)
*/
				 
var $colorLegArr; // array[ID] = array ( "c"=> color_id_of img_handle for substance legend, ID = [0,1,2,...]
				  //					 "u" => 0,1 is used ??? )
var $devColorID;  // device color array [DEV_ID] = color_id_of img_handle
private $legendarr=NULL; // array of DEVID => NAME

/**
 * 
 * @param unknown $factor // in minutes
 * @param unknown $startori // Unix-stamp
 * @param unknown $endori // Unix-stamp
 * @param  $dateArr is array of (DEV_ID => _oneDeviceAllLine_STRUCT)
 * @param array $optx
	"mode"    => "", "multi" (more substances)
	"debug"
	"title"   => title of diagram
	"noLegend" [0],1
 */
function __construct($factor, $startori, $endori, &$dateArr,	$optx = NULL ) {
	
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
	
	$this->infox["UnitPix"] 	= 20;	// X-diff between two TIME-ticks
	$this->infox["txtdY"]   	= 14;   // Y-diff between two text lines
	$this->infox["linesdY"] 	= 30;   // Y-step between DEVICE lines (20)
	
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
	else $start = mktime(0, 0, 0, date("m",$startori)  , date("d",$startori), date("Y",$startori));
	
	if ($factor<=61) $end = mktime(date("H",$endori)+1, 0, 0, date("m",$endori)  , date("d",$endori), date("Y",$endori));
	else $end = mktime(0, 0, 0, date("m",$endori)  , date("d",$endori), date("Y",$endori));
	
	$this->infox["start"] = $start;
	$this->infox["end"]   = $end;
	$this->infox["tNow"]  = time();
	
	$this->infox["startHum"] = date_unix2datestr($start,2);
	$this->infox["endHum"]   = date_unix2datestr($end,  2);
	$this->infox["timeDiffUnx"] = $end - $start;
	
	$this->infox["timeStepUnx"] = $factor * 60;
	$this->infox["units"] =  $this->infox["timeDiffUnx"] / $this->infox["timeStepUnx"];

	
	$this->infox["titleDY"] = 60;
	// legend for resource description
	$tmpLegendYsize = $this->infox["txtdY"] * $this->infox["numLines"] + $this->infox["txtdY"] + 15;
	if ($optx["noLegend"]>0) {
		$tmpLegendYsize = 0;
	}
	
	
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
	
	$this->infox["devColOffsX"] = 210; //OLD:30
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
	$color["orange"]=  imagecolorallocate($imgHandle, 250,180,180  );
	$color["gray"]  =  imagecolorallocate($imgHandle, 128,128,128); 
	$color["graylite"]=imagecolorallocate($imgHandle, 200,200,200);
	$color["graylitx"]=imagecolorallocate($imgHandle, 245,245,245);
	$color["yellow"] = imagecolorallocate($imgHandle, 220,220,80); 
	$color["green"]  = imagecolorallocate($imgHandle, 0  ,128,0); 
	$color["greenlt"]= imagecolorallocate($imgHandle, 200,255,200);
	$color["blue"]   = imagecolorallocate($imgHandle, 13, 141, 195); 
	$this->colors = $color;
	
	$this->colorKeys = NULL;
	$this->colorKeys['r'] = $color["red"];
	$this->colorKeys['g'] = $color["green"];
	$this->colorKeys['b'] = $color["blue"];
	$this->colorKeys['bc']= $color["black"];
	

    foreach( $colorArr as $dummy=>$tmparr) {
        $this->colorLegArr[] = array( "c"=> imagecolorallocate($imgHandle,$tmparr[0],$tmparr[1], $tmparr[2]) ,
									  "u"=> 0 // used ???
									  );
	}
	
	$titles["title"] = "Time diagram for object(s): ".$this->infox["substname"]." ". $optx["title"];
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


private function graph_axis( $data = NULL // ["x"], ["y"] [0]=min [1]=max
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
   
   // mark a time border
   $yl   = array();
   $yl[0] = $infox["maxY"];
   $yl[1] = $infox["offsY"]+3;
   $daylast = 0;
   $xnow = $this->getPixFromDate($infox["tNow"]);
   if ($xnow>0) {
   		// time line TODAY
   		
   		$yNow1=$infox["offsY"]+10;
   		$yNow2=$infox["maxY"]-10;
   		imageLine ( $this->imgH, $xnow,   $yNow1, $xnow,   $yNow2,  $this->colors["green"]);
		
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
		$year    = date("Y",$timed1);
		if ($infox["factor"]<=60) {
			if ($hour>18 OR $hour<8)  $xcolor = $colLite;
			$timeOut = $day.".".$month." ".$hour."h";
			if ((int)$hour==0)  $flagLong = 1;
		
		} elseif ($infox["factor"]<24*60)  {
		 	$timeOut = $day.".".$month." ".$hour."h";
			if ($daylast != $day)  $flagLong = 1; 
			if ($weekday==0 OR $weekday==6) $xcolor = $colYel;
		} else {
			if ($weekday==1)  $flagLong = 1;
			$timeOut = $day.".".$month.".".$year;
			if ($weekday==0 OR $weekday==6) $xcolor = $colYel;
		}
		if ($flagLong) imageLine ( $this->imgH, $x, $yl[0], $x, $yl[1],  $colLite);
		else imageLine ( $this->imgH, $x, $y0, $x, $y1,  $colLite);
		
		imagestringup( $this->imgH, $thisFontSize, $x-4, $timeStringY , $timeOut, $xcolor); // $yoff+70
		if ($daylast != $day) {
			$weekdayNic = date("D",$timed1); 
			imagestring( $this->imgH, $fontSizMin, $x-4, $timeDayStringY, $weekdayNic, $xcolor); //$yoff+75
		}
		
		$daylast = $day;
		$i++;
   }
   

  
} 
	
private function graph_titles($titles // "title", "t2"
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



private function setDeviceColors( 
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

private function getDevColor($devid) {
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

// get infox value
private function getInfoVal($key) {
	return $this->infox[$key];
}

// slim arrow
private function setSlimArrow($x0, $y, $color, $direct) {
	if (!$direct) {
		$dirfac= 1;
	} else {
		$dirfac= -1;
	}
	imageLine ( $this->imgH, $x0+$dirfac, $y-1, $x0+$dirfac, $y+1, $color);
	imagesetpixel( $this->imgH, $x0+2*$dirfac, $y, $color);
}

private function setArrow($x, $y, $color, 
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

// get Y position by device-index
private function _getY_byIndex($index) {
	$y 	= $index * $this->infox["linesdY"] + $this->infox["maxY"];
	return $y;
}


/**
 * create time event-stripes for ONE device in on horizontal line
 * @return 
 * @param array $oneObjArr
 * @param int $lineId
 * @param object $opt[optional]
 * 	'noKey' => 0,1 : do NOT write string of KEYs
 *  'slim' => 0,1
 */
private function oneObjArrOut(&$oneObjArr, $lineId, $opt=NULL) {
	
	if (!sizeof($oneObjArr)) return;
	
	$infox = &$this->infox;
	$colx 		 = $this->colors["black"];
	$colorText   = $this->colors["black"];
	$colorGray   = $this->colors["gray"];
	$cKeys 		 = &$this->colorKeys;
	$stripeY     = 5;
	$smallFont   = 2;
	$txt_Y_offset= 9;
	$txt_Y_offset2= $txt_Y_offset*2;
	$keyY_pos    = 1;	// Y-Position of key; -1,1 - up,down
	$y = $this->_getY_byIndex($lineId);
	
	$printKey = 1;
	if ($opt['noKey']>0) $printKey = 0;
	$slimOpt=0;
	if ($opt['slim']>0) $slimOpt = 1;
	
	foreach( $oneObjArr as $pos=>$tarr) {	
		$lineFlag = 1;
		$startori = $tarr["s"];
		$endTime  = $tarr["e"];
		$colx1    = $colx;
		if ($tarr['c']!=NULL) {
			$colx1 = $cKeys[$tarr['c']];
			if (!$colx1) $colx1=$colx;
		}

		$start = $startori;
		if ($startori < $infox["start"]) $start=$infox["start"];
		if ($endTime  >$infox["end"])   	 $endTime=  $infox["end"];
		if ($endTime  <$infox["start"]) 	 $endTime=  $infox["start"];
		
		$x0 = intval(($start-$infox["start"]) * $infox["facNorm"] + $infox["offsX"]);
		$x1 = intval(($endTime  -$infox["start"]) * $infox["facNorm"] + $infox["offsX"]);
		
		
		if ($tarr["s"]=="") {
			if ($slimOpt) $this->setSlimArrow($x1, $y, $colx1, 1);
			else $this->setArrow($x1, $y, $colx1, 1);
			imageLine ( $this->imgH, $x1, $y-$stripeY, $x1, $y+$stripeY, $colx1);
			
			$lineFlag = 0;
		}
		if ($tarr["e"]=="") {
			if ($slimOpt)  $this->setSlimArrow($x0, $y, $colx1, 0);
			else $this->setArrow($x0, $y, $colx1, 0);
			imageLine ( $this->imgH, $x0, $y-$stripeY, $x0, $y+$stripeY, $colx1);
			$lineFlag = 0;
		}
		
		if ($this->debug>1) {
			echo "-oneLine: $i. startHum: ".date_unix2datestr($startori,2)." endHum: ".date_unix2datestr($endTime,2).
				"stOri:$startori endNew:$endTime ".
				" x0:$x0 x1:$x1 y:$y color:key:".$tarr['c'].",id:$colx1 flag:$lineFlag<br>\n";
		} 
		
		if ( $lineFlag ) {
			imagefilledrectangle ( $this->imgH, $x0, $y-3,   $x1, $y+3,   $colx1 );
			//imageLine ( $this->imgH, $x0, $y,   $x1, $y,   $colx1);
			//imageLine ( $this->imgH, $x0, $y-1,   $x1, $y-1,   $colx1);
			imageLine ( $this->imgH, $x0, $y-$stripeY, $x0, $y+$stripeY, $colx1);
			imageLine ( $this->imgH, $x1, $y-$stripeY, $x1, $y+$stripeY, $colx1);
		}
		
		if ($printKey) {
			$keytxt = $tarr["k"]; // substr($tarr["k"],0,8);
			if ( strlen($keytxt)>20 ) $keytxt = substr($tarr["k"],0,20). '...';
			
			if ($keyY_pos<0) $ykey = $y - $txt_Y_offset2;
			else  $ykey = $y+ $txt_Y_offset;
			
			imageString( $this->imgH, $smallFont, $x0+2, $ykey, $keytxt, $colorText); // key
			
			$keyY_pos = $keyY_pos*-1; // toggle Y
		}
		
	}
	reset ($oneObjArr);
		
}


/**
 * - draw time lines (1 unit = 20 pixels)
   - suport also time-events, where START=NULL or END=NULL
 * @param unknown $index  // index of substance 0....n	
 */
public function calculate(  $index	) {
	
	$infox = &$this->infox;
	
	$colx 		 = $this->colors["black"];
	$colorText   = $this->colors["black"];
	$colConflict = $this->colors["red"];
	$colorLite   = $this->colors["graylite"];
	$colorGray   = $this->colors["gray"];
	$thisFontSize= 2;
	
	
	$drec = array();
	$drec[0] = $this->infox["devColOffsX"];
	$drec[2] = $this->infox["devColOffsX"]+5;
	
	if ( !sizeof($this->dateArr) ) return; 
	$style = array( $colorGray, $this->colors["white"] );
	imagesetstyle( $this->imgH, $style );
	$objId_xOffs = 2;
	
	if ( $this->debug >2 ) {
		echo "DEBUG_legendarr:".print_r($this->legendarr,1)."<br>\n";
	}
	
	$i=0;
	foreach( $this->dateArr as $objid=>$oneObjArr) {
		
		$lineFlag = 1;
		$y 		  = $this->_getY_byIndex($i);
		$objname  = 'ID:'.$objid;
		if ($this->legendarr[$objid]!=NULL) $objname .= ' '.$this->legendarr[$objid];
		
		imageLine  ( $this->imgH, $infox["offsX"], $y, $infox["offsX"] + $infox["diffX"], $y, $colorLite); // help line
		imageString( $this->imgH, $thisFontSize, $objId_xOffs, $y-5, $objname, $colorText); // ID of device
		
		// device-ID
		$drec[1]  = $y - 3;
		$drec[3]  = $y + 3;
		$colorDev = $this->getDevColor($objid);
		// imageFilledRectangle( $this->imgH, $drec[0], $drec[1], $drec[2], $drec[3], $colorDev); // resource-ID
			
		if (sizeof($oneObjArr)) {
	 		$this->oneObjArrOut($oneObjArr, $i);
		}
		$i++;
	}
	reset($this->dateArr);
	
}

private function getPixFromDate($dateUnix) {
	// returns x
	$infox = &$this->infox;
	if ($dateUnix > $infox["end"]   ) return;
	if ($dateUnix < $infox["start"] ) return;
	
	$x = ( $dateUnix-$infox["start"] ) * $infox["facNorm"] + $infox["offsX"];
	return ($x);
}


/**
 * build legend below the diagram
 * @param array $legendarr array($device-id => name)
 * @param array $colorStripDesc (colorkey => description), e.g. ("red", "green", "blue")
 * @return -
 */
public function legend( $legendarr, $colorStripDesc=NULL ){
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
	
	imageString( $this->imgH, 3, $x0, $yoff, "Legend: ...", $colx);
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
	$tmparr = array("red" => "-", "green" => "-", "blue" => "-");
	
	foreach( $tmparr as $key=>$dummy) {
		$colx1 = $this->colors[$key];
		if (!$colx1) $colx1=$colorBlack;
		$colortext = $colorStripDesc[$key];
		imageLine ( $this->imgH, $x0, $y,   $x1, $y,   $colx1);
		imageLine ( $this->imgH, $x0, $y-2, $x0, $y+2, $colx1);
		imageLine ( $this->imgH, $x1, $y-2, $x1, $y+2, $colx1);
		imageString( $this->imgH, $thisFontSize, $xText, $y-6, $colortext, $colorBlack);
		$y = $y + $infox["txtdY"];
	}
}

public function set_Obj_Names(&$legarr) {
	$this->legendarr = $legarr;
}

public function show() {
	
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