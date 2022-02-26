<?php
/**
 * - main sub functions for images
 * - create image 
 * $Header: trunk/src/phplib/f.grImgSub2.inc 59 2018-11-21 09:04:09Z $
 * @package f.grImgSub2.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */

class fgrImgSub2C {

var $CLASSNAME="fgrImg2C";

var $GR;
var $GRCO; // color array
var $imgH; // handle of image
var $infoxz;	// "obj_max_len", "value_min", "value_max"

/**
 * 
 * @return 
 * @param array $GR : "img_x_size","img_y_size"
 * 					  'GRCO'=> array('colorname'=>array(r,g,b) )
 * @param array $optarr  
 * 					"debug" : 0,1,2
 */
function  init ( &$GR, $optarr ) {
	//global $error;
	//$FUNCNAME = $this->CLASSNAME.":init";
	
	$this->tmp_debug_lev = $optarr["debug"];
	$this->errorIntern   = NULL;
	
	if ( $this->tmp_debug_lev>1 ) {
		echo "DEBUG active!<br>\n";
	}
	
	$this->imgH = imagecreate($GR["img_x_size"], $GR["img_y_size"]);
  	if (!$this->imgH) die('GD error: Image creation error');
	
	$this->GRCO["white"] =  imagecolorallocate($this->imgH, 255,255,255);
  	$this->GRCO["black"] =  imagecolorallocate($this->imgH, 0,  0,  0  );
  	$this->GRCO["red"]   =  imagecolorallocate($this->imgH, 255,0,  0  ); 
	
	// register colors
	$colorPoi = &$GR['GRCO'];
	foreach( $colorPoi as $key=>$valarr) {
		$this->GRCO[$key] = imagecolorallocate($this->imgH, $valarr[0],$valarr[1],$valarr[2] );
	}
	
}

/**
 * create an error image
 * @return 
 * @param object $err_text
 * @param object $err_type[optional] : "INFO", 'ERROR'
 */
function errorImage( $textArr, $err_type=NULL ) {

	if ($this->imgH) ImageDestroy($this->imgH);
	
	$ydiff = 14;
	$txtLines  = sizeof($textArr);
	$img_y_pix = $ydiff * $txtLines + 6;
	$img_x_pix = 600;
	$this->imgH    = imagecreate($img_x_pix, $img_y_pix);
	$GRCO=array();
	$GRCO["red"]   = imagecolorallocate($this->imgH, 255,0,0);   
	$GRCO["gray"]  = imagecolorallocate($this->imgH, 128,128,128);
	$white = imagecolorallocate($this->imgH, 255,255,255);
	imageFill   ($this->imgH, 0, 0, $white);
	
	if ( $err_type == "INFO" ) { 
		$txtcolor = $GRCO["gray"];
		$err_pre = "INFO: ";
	} else {    
		$txtcolor = $GRCO["red"];  
		$err_pre = "ERROR: ";
	} 
	imageRectangle ( $this->imgH, 0, 0, $img_x_pix-1, $img_y_pix-1,  $GRCO["gray"] );
	// write out text lines to image
	$ytmp = 2;
	$i=0;
	foreach( $textArr as $txttmp) {
		if (!$i) $txttmp = $err_pre . $txttmp;    // PRE text like "INFO: "
		imageString ($this->imgH, 3, 10, $ytmp, $txttmp, $txtcolor);
		$ytmp = $ytmp + $ydiff;
		$i++;
	}
	
  
}

function &getImgHandle() {
	return ($this->imgH);
}

function getGRCO() {
	return $this->GRCO;
}

function imgOut($filename=NULL) {
	if ($filename==NULL) {
		Imagepng ( $this->imgH );
	} else {
		Imagepng ( $this->imgH, $filename );
	}
}

function close() {
	ImageDestroy( $this->imgH );
	$this->imgH = NULL;
}
	
}

