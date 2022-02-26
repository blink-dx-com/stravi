<?php
/**
 * generate a 90grad rotatatet PNG with text
 * @package f.imgtxt.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   $t   (text)
         $bg  ["dark"] (backcolor)
		 $f   [12]   (font size) 10, 14
		 $a   [90]   (angle)
		 [$help] 0,1,2,3 
 * @version0 2002-09-04
 */
session_start(); 

require_once('utilities.inc');

class fImgTxtC {

function __construct( $t, $bg, $fontSize, $angle, $help ) {
	$this->t=$t;
	$this->bg=$bg;
	$this->fontSize=$fontSize;
	$this->angle=$angle;
	$this->help=$help;
}

function _allocColors() {

	if ( $this->bg =="dark" ) $this->col["white"] = ImageColorAllocate($this->im, 128,128,128);  
	else                $this->col["white"] = ImageColorAllocate($this->im, 255,255,255); 
	$this->col["black"] = ImageColorAllocate($this->im, 0,0,0);
	$this->col["blue"]  = ImageColorAllocate($this->im, 0,0,255);
	// imagefilledrectangle  ( $this->im , int $x1  , int $y1  , int $x2  , int $y2  , int $color  )
	imagecolortransparent ( $this->im,$this->col["white"] );
}

function ttfOut() {
	
	
	$t  = $this->t;
	$bg = $this->bg;
	$fontSize=$this->fontSize;
	$angle=$this->angle;
	
	$fontdir     = $_SESSION['s_sessVars']["DocRootDir"]."/../phplib/fonts";
	$fontfile    = $fontdir."/arial.ttf";
	
	$size     = imagettfbbox( $fontSize, $angle, $fontfile, $t );
	$dx       = abs($size[4]);
	$dy       = abs($size[5]); 
	$dx0      = $size[0];
	$dxrest   = abs($size[5]-$size[3]);
	
	$xlen = $dx+3 + $dx0;
	$ylen = $dy+2;
	$this->im = imagecreate( $xlen, $ylen );  
	if ( $this->help>1 ) {
		echo "<br>\nDEBUG: text:'".$t."' len:".(strlen($t))." dx:$dx dy:$dy   xlen:$xlen, ylen:$ylen <br>\n";
		echo "TTF-box:";
		print_r($size);
		echo "<br>";
	}
	
	$this->_allocColors();
	
	ImageTTFText( $this->im, $fontSize, $angle, $dx+1, $dy+1, $this->col["blue"], $fontfile, $t);
	Imagepng    ( $this->im );
	ImageDestroy( $this->im ); 
}

function courierOut() {
	// FUNCTION: normal text out
	//			 Text = 90 grad
	
	$this->fontID = 3;
	if ( $this->fontSize < 12 ) $this->fontID = 2;
	if ( $this->fontSize < 11 ) $this->fontID = 1;
	if ( $this->fontSize > 12 ) $this->fontID = 4;
	if ( $this->fontSize > 13 ) $this->fontID = 4;
	
	$border = 2;
	$lenx = strlen($this->t);
	$letterFeat=array();
	$letterFeat["y"]= imagefontheight( $this->fontID );
	$letterFeat["x"]= imagefontwidth ( $this->fontID );
	
	$xlen = $letterFeat["y"] + $border*2;
	$ylen = $letterFeat["x"]*$lenx + $border*2;
	
	$this->im = imagecreate( $xlen, $ylen ); 
	$this->_allocColors();
	ImageStringUp( $this->im, $this->fontID, $border, $ylen-$border, $this->t, $this->col["blue"] );
	Imagepng    ( $this->im );
	ImageDestroy( $this->im ); 
}

}

if ( !is_partisan_open () ) {
    echo "ERROR: session is not open.\n";
    return;
}    

$t= $_REQUEST["t"];
$bg = $_REQUEST["bg"];
$f= $_REQUEST["f"];
$a = $_REQUEST["a"];
$help= $_REQUEST["help"];


if ( $help<2 ) {
    Header("Content-type: image/png");
}

$fontSize    = 12;
$angle       = 90;

if ($a!="") $angle = $a; 
if ( $f )   $fontSize = $f;

$imgObj = new fImgTxtC( $t, $bg, $fontSize, $angle, $help );

if ( $angle != 90) $imgObj->ttfOut();
else {
	$imgObj->courierOut();
}

