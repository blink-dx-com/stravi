<?php
/*MODULE: o.IMG.c_series.inc
  DESCR:  class series subs
  AUTHOR: qbi
  VERSION: 0.1 - 20030705
*/ 
require_once("glob.image.inc");
  
class oIMGseriesC {

var $image_arr;
var $objValues;
var $img_body;

function imgInit( &$sql, $imgid ) {
	// FUNCTION: init image
	global $error, $varcol;
	$FUNCNAME = "imgInit";
	
	$this->objValues = NULL;
	$this->img_body  = NULL;
	
	$sqls = "select NAME, extra_obj_id from IMG where IMG_ID=".$imgid;
	$sql->query($sqls);
	$sql->ReadRow();
	$this->img_body  = $sql->RowData[0];
	$extra_obj_id    = $sql->RowData[1];
	
	if ( !$extra_obj_id ) {
		$error->set( $FUNCNAME, 4, "image $imgid has no class");
		return;
	}
	
	$values_tmp = $varcol->select_by_name ($extra_obj_id);
	$this->objValues     = &$values_tmp["values"];
	
	list ($tmpret, $errtxt, $this->image_arr) = image_series_get( $this->img_body, $values_tmp );
	if ($tmpret<0) {
		$error->set( $FUNCNAME, 2, "problems on getting series:".$errtxt);
		return;
	}

}

function getImgBaseName() {
	$filename = $this->img_body;
	$filename_url = str_replace("\\", "/", $filename);
	$lastStripPos = strrpos($filename_url,"/");
	if ($lastStripPos!==FALSE) $lastStripPos++;
	$fileBase = substr($filename_url, $lastStripPos); // basename($filename_url);
	return ($fileBase);
}

function getImgThumbUrl( 
	$pos 		// pos of image in series, always start with 1
	) {
	// FUNCTION: get image-thumbnail-URL of for ONE image of image-series
	// INPUT: $this->objValues and others
	
	
	$tmpimg   =  $this->getFilenameOfImage( $this->img_body, $this->objValues,$pos);
	$tmpimg_enc = urlencode($tmpimg);
	$imgurl   = $_SESSION['s_sessVars']["loginURL"]."/f.image_show.php?filename=".$tmpimg_enc;
	return ($imgurl); 
   
}

function getFilenameOfImage(
	$basename, 
	$xargu,		// array[param-name] = value
	$imagenum	// pos of image in series, always start with 1
	) {
	//FUNCTION: calculate the imagename from the image parameters
	
	
	$num_of_chars  = strlen($xargu["start-number"]);
	$imagenum_real = $imagenum;
	if ($xargu["name-period"]>1) $imagenum_real = $imagenum * $xargu["name-period"];
	$imagenum_real = ($imagenum_real-1) + $xargu["start-number"];
	
	$fname     = str_pad( $imagenum_real, $num_of_chars, "0", STR_PAD_LEFT ) . $xargu["name-extension"];     
    $imagename = $basename . $fname;
	return ($imagename);
}

}