<?php
/*MODULE: glob.image.search.inc
  DESCR: provide functions to search images
  AUTHOR: qbi
  VERSION: 0.1 - 20030705
*/ 
require_once('glob.image.inc');
  
class fImgScanDirC {

function image_scanDir(
    $img_name_body, // must contain SLASHES 
    $img_ext,
	$option = NULL  // "ignoreFile" => name
    ) {
	// FUNCTION: scan directory for imag files
	// RETURN: array[] = filename
	$FUNCNAME="image_scanDir";
	
    global $error;
    
    $imgarr  = array();
    $retval  = 0;   
    $filecnt = 0;
    
    $img_name_body_new = netfile2serverpath( $img_name_body );
	$this->img_name_body_new = $img_name_body_new;
	
	if ( !file_exists($img_name_body_new) ) {
		$error->set($FUNCNAME, 1, "Can't open path of images '$img_name_body'. NET-dir:'".$img_name_body_new."'");
        return;
	}
	
			
    $otherarr = NULL;
    if ( ($dh = opendir ( $img_name_body_new )) == false ) {
		$error->set($FUNCNAME, 2, "Can't open path of images '$img_name_body'. ".
				"Please check, if read-rights are set. NET-dir:'".$img_name_body_new."'");
        return;
	}
		
	$extlen = strlen($img_ext);
	
	while (false !== ($file = readdir ($dh))) {
		$flen = strlen ($file);
		$fext = substr ($file, $flen-$extlen);
		
		if ($file=="." OR $file=="..") continue;
		
		if ($fext==$img_ext) $imgarr[] = $file;
		else {
			if ( $option["ignoreFile"]==$file ) {
				$dummy=1;
			} else {
				$otherFilecnt++;
				if ($otherFilecnt<5) $otherarr[] = $file;
			}
		}
	}
	
	if ( sizeof($imgarr) ) {
		sort ($imgarr);
		reset ($imgarr);
	}
	closedir($dh); 
   
	
    if (!sizeof($imgarr) AND $otherFilecnt) {
		$msgplus = implode(", ", $otherarr) . " ...";
    	$error->set($FUNCNAME, 3, "No images found with extension '$img_ext', but found ".$otherFilecnt." other file(s): ".$msgplus);
    } 
    
    return ($imgarr);
}

function analysePathError($img_path_url) {
	// FUNCTION: analyse directory-sub-paths of a complex path
	global $error;
	
	$img_name_body_new = netfile2serverpath( $img_path_url );
	
	$posend = strlen($img_name_body_new);
	$pos    = 0;
	$lastslash = -1;
	$errorinfo = "";
	$partpath  = "";
	$lastGoodpath = "";
	
	while ($pos < $posend) {
		$found=0;
		do {
			if (substr($img_name_body_new,$pos,1)=="/") {
				if ($pos<=1) { // "/" or "//"
					$found=1;
					break; 
				}
				
				// normal path
				$partpath = substr($img_name_body_new,0, $pos);
				if ( !file_exists($partpath) ) {
					$errorinfo = "Sub-path '".$partpath."' does not exist.";
					break;
				}
				
				if ( !is_readable($partpath) ) { 
					$errorinfo = "Sub-path '".$partpath."' is not readable.";
					break;
				}
				
			} 
			
		} while (0);
		
		if ($errorinfo!="") {
			$error->set('analysePathError', 1, "Last good path: '<b>".$lastGoodpath."</b>'. ".
			$errorinfo);
			break;
		} 
		$lastGoodpath = $partpath;
		$pos++;
	}
	$error->printAll();
}

}


