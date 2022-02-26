<?php
/**
 * Set key,val of $_SESSION['s_sessVars']
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @package sessVars_set.php
 * @param $variable first key name
 * @param [$key2]   second key
 * @param $val 	   new value
 * @param [$backurl] encoded backurl
 */
session_start(); 

require_once ('reqnormal.inc');
require_once("javascript.inc");

$pagelib = new gHtmlHead();
$pagelib->PageHeadLight('set s_sessVars');

$variable=$_REQUEST['variable'];
$key2=$_REQUEST['key2'];
$val=$_REQUEST['val'];
$backurl=$_REQUEST['backurl'];

if (!glob_loggedin()) {
	htmlFoot('ERROR', 'not logged in!');
}

if ($key2!=NULL) {
	$_SESSION['s_sessVars'][$variable][$key2] = $val;
} else $_SESSION['s_sessVars'][$variable] = $val;

if ($backurl!="") {
	$backnow = urldecode($backurl);
	js__location_replace($backnow);
	return;
} 

js__history_back2();
	
$pagelib->htmlFoot();
