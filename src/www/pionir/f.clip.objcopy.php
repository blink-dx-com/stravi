<?php
/**
 * Copy object-IDs of a table ($tablename) in xs[] to clipboard
 * @namespace core::misc
 * @package f.clip.objcopy.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param $xs[]  ( array xs[ID] = 1 )
 		  $tablename
 * @param [$backurl] encoded backurl
 */
session_start(); 
require_once ('reqnormal.inc');
global $error, $varcol;

$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();

$xs = $_REQUEST["xs"];
$tablename=$_REQUEST['tablename'];
$backurl=$_REQUEST['backurl'];

$title       = 'Copy selected objects to clipboard';
$infoarr=array();
$infoarr["title"]    = $title;
$infoarr["form_type"]= "list";
$infoarr["obj_name"] = $tablename;

$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);
echo "<ul>";

if (!sizeof($xs)) {
	htmlFoot("Error", "No objects selected!");
}

$_SESSION['s_clipboard'] = NULL;
foreach( $xs as $key=>$val) {
	$ids = array('tab' => $tablename, 'ida' => $key);
	$_SESSION['s_clipboard'][] = $ids;
}

echo "<B>".sizeof($_SESSION['s_clipboard'])."</B> objects copied to clipboard.";

if ($backurl!="") {
	$backnow = urldecode($backurl);
	js__location_replace($backnow);
	return;
} 

$pagelib->htmlFoot();
