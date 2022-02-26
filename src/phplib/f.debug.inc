<?php


/**
 * debug functions
 * @author Steffen
 * @package  f.debug.inc
 */
class fDebugC {

/**
 * print HEX info of $strx 
 * @param $strx
 * @param $option
 *   'cdx': show char, decimal, hex
 * @return string
 */
static function str2Hexinfo( $strx, $option=NULL ) {
	//echo "fDebugC::str2Hexinfo(): ";
	
	$output=NULL;
	$lenx   = strlen($strx);
	$output = "len: ".$lenx. " str:[".htmlEntities($strx) ."] ";
	
	$showFull=0;
	if ($option['cdx']>0) $showFull=1;
	
	$i=0;
	while ( $i<$lenx ) {
		$onechar = substr($strx,$i,1);
		$charcode = ord($onechar);
		$charcodeHex = sprintf("%x", $charcode);
		if (!$showFull) {
			$output .= $charcodeHex." ";
		} else {
			$output .=' c:'.$onechar.' d:'. 
				str_pad($charcode,3," ") .' x:'.$charcodeHex."|";
		}
		$i++;
	}
	return $output;

}

/**
 * other display option
 *  qwert.rereer 34 32 10 20 32 32
 * @param  $strx
 * @param  $option
 */
static function str2Hexinfo2( $strx, $option=NULL ) {
	//echo "fDebugC::str2Hexinfo(): ";
	
	$output=NULL;
	$lenx   = strlen($strx);
	$output = "length: ".$lenx;
	
	$pageMax = ceil($lenx/16);
	$i=0;
	while ( $page<$pageMax ) {
		
		$onePart  = substr($strx,$page*16,16);
		$partOut  = str_replace("\n", ".", $onePart);
		$output  .= "\n" . $partOut." | ";
		$i=0;
		while ( $i<strlen($onePart) ) {	
			
			$onechar  = substr($onePart,$i,1);
			$charcode = ord($onechar); 
			$charcodeHex = sprintf("%02x", $charcode);
			$output .= $charcodeHex." ";
			
			$i++;
		}
		$page++;
	}
	$output  .= "\n";
	return $output;

}

/**
 * store debug message on file ...
 * @param unknown $funcname
 * @param unknown $text
 */
static function save_debug($funcname, $text) {
    $datafile='/var/log/gozilla_debug.log';
    $fp = fopen($datafile, 'a');
    if (!$fp) return;
    $retVal = fputs( $fp, date('Y-m-d H:i:s')."\t".$funcname. "\t" . 'INFO: '.$text. "\n" );
    fclose( $fp );
}

}