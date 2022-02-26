<?php
/**
 * download PAXML-TAR-file
 * $Header: trunk/src/www/pionir/impexp/partisanxml/paxml.tar.php 59 2018-11-21 09:04:09Z $
 * @package paxml.tar.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */

extract($_REQUEST); 
session_start(); 

require_once ('reqnormal.inc');

function _thisHtmlError($message) {
	
	$title		= 'paxml download';
	
	$infoarr			 = NULL;
	$infoarr['title']    = $title;
	$infoarr['form_type']= 'tool'; // 'tool', 'list'
	$infoarr['design']   = 'norm';
	
	$sqlo=NULL;
	$pagelib = new gHtmlHead();
	$headarr = $pagelib->startPageLight($sqlo, $infoarr);
	echo '<ul>';
	$pagelib->htmlFoot('ERROR', $message);
	
}

// --------------------------------------------------- 
global $error, $varcol;

$error = & ErrorHandler::get();

if ( !glob_loggedin() ) {
	_thisHtmlError('your are not logged in.');
}

$filePure = "pxmlexport.". session_id() . ".tar";
$filename = $_SESSION['globals']['work_path'] . "/".$filePure;
if ( !file_exists($filename) ) {
    _thisHtmlError('paxml-file "'.$filePure.'" does not exist.');
}
    
header("Content-type: application/x-tar");
header("Content-Disposition: attachment; filename=paxml.tar");
    
fpassthru(fopen($filename, "r"));

