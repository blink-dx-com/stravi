<?php
/**
 * RcodeExecuter
 * - execute Rcode,   stored as attachment-file in object LINK ($id)
 * - input-data-file: $datafile
 * - produces various output files on USER_SESSIONDIR/R.pack
 * 
 * $Header: trunk/src/www/pionir/obj.link.c_Rcode.exe.php 59 2018-11-21 09:04:09Z $
 * @package obj.link.c_Rcode.exe.php
 * @author  Steffen Kube
 * @param int    $id: 	document ID of RScript
 * @param string $datafile: name data file for RInput (relative to USER_SESSIONDIR )
 * @param array  $opt
 *   'images_no_show' = 0,1 do NOT autoamtically show images in results
 * @swreq UREQ:0010165 FS-LIM07-A01 Provide a basic R interface 
 * @unittest EXISTS => see obj.link.c_Rcode.exe.inc
 */

session_start(); 

require_once ("reqnormal.inc");
require_once ("obj.link.c_Rcode.exe.inc");



global $error, $varcol;

$error = & ErrorHandler::get();
$sqlo  = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();

$id 		= $_REQUEST["id"];
$datafile	= $_REQUEST["datafile"];
$sc_options = $_REQUEST["opt"];
$tablename	= "LINK";
$title		= "RcodeExecuter";
$infoarr			 = NULL;
$infoarr["scriptID"] = "obj.link.c_Rcode.exe.php";
$infoarr["title_sh"] = $title;
$infoarr["title"]    = $title. ' (version:19.10.2017)';
$infoarr["form_type"]= "obj"; 
$infoarr["obj_name"] = $tablename;
$infoarr["obj_id"]   = $_REQUEST["id"];
$infoarr["checkid"]  = 1;
$infoarr['help_url'] = 'f.RInterface.html';

$pagelib = new gHtmlHead();
$headarr = $pagelib->startPage($sqlo, $infoarr);
$pagelib->chkErrStop();

if ( !$datafile ) {
	htmlFoot("ERROR", "data file unknown");
}

if ($_SESSION["userGlob"]["g.debugLevel"]>1) {
	echo 'Input-Session-file: '.$datafile."<br />\n";
}

$RObj = new oLINK_cRcodeExe( $sqlo, $datafile, $id );
$pagelib->chkErrStop();

$RObj->checkR( $sqlo );
$pagelib->chkErrStop();

$RObj->execR( $sqlo );
$pagelib->chkErrStop();

$RObj->buildFileTable($sc_options);

$pagelib->htmlFoot();