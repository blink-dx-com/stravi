<?php 
/**
 * show version history
 * @package obj.link.verhist.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   $id of LINK
 */
session_start(); 


require_once ("o.LINK.subs.inc");
require_once ("reqnormal.inc"); 
require_once ("o.LINK.versctrl.inc");
require_once ("visufuncs.inc");
require_once ("o.LINK.vershist.inc");


// --------------------------------------------------- 
global $error, $varcol;

$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] );
$sql2  = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();

$id 		= $_REQUEST["id"];
$tablename="LINK";

$title       = "Version history of the document";

#$infoarr['help_url'] = 'o.EXAMPLE.htm';
$infoarr=array();
$infoarr["title"] = $title;
$infoarr["form_type"]= "obj";
$infoarr["obj_name"] = $tablename;
$infoarr["obj_id"]   = $id;
$infoarr["checkid"]  = 1;


$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);
echo "<ul>";

$versionObj = new oLINKversC();
if ( !$versionObj->isPossible() ) {
	htmlFoot("Info", "Version control not implemented on the system. Please ask the admin!");
}

if ( !$versionObj->isUnderControl($sql, $id) ) {
	htmlFoot("Info", "This document is not under version corntrol!");
}

$versHistObj = new oLINKvershist($versionObj);
$versHistObj->show( $sql, $sql2, $id);

htmlFoot();