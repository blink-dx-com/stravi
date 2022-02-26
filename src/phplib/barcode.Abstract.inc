<?php

/**
 * ABSTRACT functions for barcode-classes
 * @package barcode.Abstract.inc
 * @swreq   UREQ:0001137: g > support Code128 barcode image gen 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class barcodeAbstractC {    
	var $params; /**
	  @var $params
		"charExpect" : e.g. for Code128: 3
		"IMG_Y_LEN"  : e.g. 120
		"txt_noZero" : on label TEXT: no leading zeros for numbers
						  e.g. code: 003456; print below label: 3456
		"txt_font"  : 8,9,10,12
		*/
    var $im; // image object
	var $IMG_X_LEN;
	var $IMG_Y_LEN;

/**
 * new instance
 * @param  $params see $this->params
 */
function __construct( $params=NULL ) {
	if ( $params!=NULL ) {
		$this->setInitParams( $params );
	}
}

/**
 * set params
 * @param  $params
 */
function setInitParams( $params ) {
	$this->params = $params;
}

function buildImage( 
	$code, 
	$option=NULL //
	) {}
		
function getImgDim() {
	return (array($this->IMG_X_LEN, $this->IMG_Y_LEN));
}		
	
function htmlOut() {
    // Sending Headers
    header ("Content-type: image/png");
    header ("Expires: Mon, 01 Apr 1969 08:00:00 GMT");    // Date in the past
    header ("Last-Modified: " . gmdate("D, d M Y H:i:s") . " GMT"); // always modified
    header ("Cache-Control: no-cache, must-revalidate");  // HTTP/1.1
    header ("Pragma: no-cache");                          // HTTP/1.0   
    
    ImagePng ($this->im);
} 

function saveImagePng( $filename ) { 
    ImagePng ( $this->im, $filename );
}

function saveImageJpg( $filename ) {
    $jpgQuality = 75; 
    ImageJpeg ( $this->im, $filename, $jpgQuality );
} 
		
function destroyx() {
	ImageDestroy($this->im);
}

}