<?php
/**
 * set variables of user preferences $_SESSION['s_formState']
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @package f.s_formState.set.php
 * @param $key ( $_SESSION['s_formState'][$variable] )
		 [$subkey]
	     $val ( new value)
 * @param [$backurl] encoded backurl
 */
session_start(); 
require_once ('reqnormal.inc');
require_once("javascript.inc");

$key= $_REQUEST["key"];
$subkey= $_REQUEST["subkey"];
$val = $_REQUEST["val"];
$backurl = $_REQUEST["backurl"];

$pagelib = new gHtmlHead();
$pagelib->PageHeadLight('formState');

if (!glob_loggedin()) {
	htmlFoot('ERROR', 'not logged in!');
}
		
if ( $subkey !="" ) {
	if ($val == "") unset($_SESSION['s_formState'][$key][$subkey]);
	else $_SESSION['s_formState'][$key][$subkey] = $val;
} else {
		
	if ($val == "") unset($_SESSION['s_formState'][$key]);
	else $_SESSION['s_formState'][$key] = $val;
}

echo "change preference of $key=".$_SESSION['s_formState'][$key]."\n";

if ($backurl!=NULL) {
	$backnow = urldecode($backurl);
	js__location_replace($backnow);
	return;
} 

js__history_back2();

$pagelib->htmlFoot();
