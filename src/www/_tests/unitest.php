<?php
/**
 * unit test main entrance
 * $Header: trunk/src/www/_tests/www/test/unitest.php 59 2018-11-21 09:04:09Z $
 * @package unitest.php
 * @author  qbi
 * @version 1.0
 * @param int $go 0,1,2
 * @param string $parx[modpath] 
 * @param string $parx[subTest] name of sub-test for module
 * @param int $parx[infolevel] 
 * @example see unittest_example.php
 */

extract($_REQUEST); 
session_start(); 



$usePath = "../../phplib/";
require_once ($usePath."reqnormal.inc"); // includes all normal *.inc files
require_once ($usePath.'func_form.inc');
require_once ($usePath.'f.msgboxes.inc');  
require_once (dirname(__FILE__).'/../../_test/misc/unittest_onetest.inc');



// ------------------------------------------------------

function formshow($parx) {

	

	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Set parameters";
	$initarr["submittitle"] = "Submit";
	$initarr["tabwidth"]    = "AUTO";

	$hiddenarr = NULL;


	$formobj = new formc($initarr, $hiddenarr, 0);

	$fieldx = array (
			"title" => "module-path",
			"name"  => "modpath",
			"fsize" => 80,
			"req"   => 1,
			"object"=> "text",
			"val"   => $parx["modpath"],
			"notes" => "path of module (e.g. phplib/globals.inc)"
	);
	$formobj->fieldOut( $fieldx );

	$fieldx = array (
			"title" => "subtest",
			"name"  => "subTest",
			"fsize" => 40,
			 
			"object"=> "text",
			"val"   => $parx["subTest"],
			"notes" => "optional name of subtest"
	);
	$formobj->fieldOut( $fieldx );
	
	$fieldx = array (
			"title" => "Infolevel",
			"name"  => "infolevel",
			"fsize" => 3,
	
			"object"=> "text",
			"val"   => $parx["infolevel"],
			"notes" => "1-silent, 2-normal, 3-high"
	);
	$formobj->fieldOut( $fieldx );
	


	$formobj->close( TRUE );
}

// --------------------------------------------------- 
global $error, $varcol;

$error = & ErrorHandler::get();
$sqlo  = logon2( $_SERVER['PHP_SELF'] ); // URL-link for the first db-login; e.g. for object ($_SERVER['PHP_SELF']."?id=".$id)
$sqlo2 = logon2( );
if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();

$go   = $_REQUEST["go"];
$parx = $_REQUEST["parx"];

$parx["modpath"] = trim($parx["modpath"]);

$javascript = '
var InfoWin = null;

function xopenwin( url ) {

    url_name= url;
    InfoWin = window.open( url_name, "TESTWIN", "scrollbars=yes,status=yes,width=650,height=500");
    InfoWin.focus();
    
}
';

$title       		 = "UniTest - Single Test";
$infoarr			 = NULL;
$infoarr["title"]    = $title;
$infoarr["scriptID"] = "unitest";
$infoarr["form_type"]= "tool";
$infoarr["locrow"]   =  array( array("./index.php", "root") );
$infoarr['javascript'] = $javascript;
$infoarr['help_url'] = 'ad.UnitTest.html'; 

$pagelib = new gHtmlHead();
$headarr = $pagelib->startPage($sqlo, $infoarr);

echo "<ul>";

$role_right_name = 'sys.unittest';
$role_right = role_check_f($sqlo , $role_right_name );
if ( $role_right !='execute' ) {
     htmlErrorBox( "Error",   
     "Sorry , you must have role right ". $role_right_name .
	 " to use this tool." );
     htmlFoot();
}

$mainlib = new gUnitTestC($sqlo, $sqlo2, $parx, $go);

// if ( !$go ) {
	formshow($mainlib->parx);
	echo "<br>";
	if ( !$go ) {
		htmlFoot("<hr>");
	}


echo "Module: ". htmlspecialchars($parx["modpath"]) . "<br>\n";
if ($parx['subTest']!=NULL) {
	echo "Subtest: ". htmlspecialchars($parx['subTest']) . "<br>\n";
}

try {
    $mainlib->dotest($sqlo);
} catch (Exception $e) {
    $trace_string = $e->getTraceAsString();
    $trace_string = str_replace("\n", "<br>", $trace_string);
    $error->set( 'MAIN', 1, $e->getMessage().'; TRACE: '.$trace_string );
}



if ( $error->Got(READONLY)  ) {
    $error->printAll();
} else {
	echo "<br>";
	cMsgbox::showBox("ok", "UnitTest o.k.");
	
	if ( $error->Got(CCT_WARNING_READONLY)  ) {
	    echo "<p />\n";
	    $error->printAll();
	} 
}

htmlFoot("<hr>");
