<?php
/*MODULE: obj.jour_entry.ed1.php 
  DESCR: - create / edit one lab journal entry
  		 - after create  go to view mode ?
  AUTHOR:  qbi
  INPUT:   $id ( JOUR_ENTRY_ID )
  		   $action = "" | "create"
		   $go : 0,1
		   $params
		   	 ["NOTES"]
		   	 ["NAME"] -- can be the FULL name
		   	 ["EXEC_DATE"]
		   $parx[PROJ_ID]
		   		
	objp[page]
  VERSION: 0.1 - 20061019
*/

session_start(); 
require_once ("reqnormal.inc"); // includes all normal *.inc files
require_once ("javascript.inc"); 
require_once ("subs/obj.jour_entry.ed.inc");

// --------------------------------------------------- 
global $error;

$id = $_REQUEST['id'];
$action = $_REQUEST['action'];
$go= $_REQUEST['go'];
$params= $_REQUEST['params'];
$parx= $_REQUEST['parx'];


$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF']."?id=".$id ); // give the URL-link for the first db-login
// $sql2  = logon2( );
if ($error->printLast()) htmlFoot();


if ($action=="")  $action="create";

$tablename	 = "JOUR_ENTRY";
$tablenice   = tablename_nice2($tablename);

$title       = "NEW Lab journal entry";

$infoarr	 = array();

$infoarr["title"] = $title;
$infoarr["obj_name"] = $tablename;
if ( $action == "create" ) {
    $infoarr["form_type"]= "list";
} else {
    $infoarr["form_type"]= "obj";
    $infoarr["obj_id"]   = $id;
    $infoarr["show_name"]= 1;
}
$forwardStopped = 0;

$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);
echo "<ul>\n";
$labObj = new oEXPlabjour1($id, $action);



if ( $action == "create" ) {

	$t_rights = tableAccessCheck( $sql, $tablename );
	if ( $t_rights["insert"] != 1 ) {
		tableAccessMsg( $tablenice, "insert" );
		htmlFoot();
	}

	
	
	// create entry
	if ( $go < 1 ) {
	    $labObj->setParamsNew($sql, $params, $parx);
	} else {
	    $labObj->setParams($sql, $params, $parx, $go);
		$newid = $labObj->createEntry($sql);
		if ($error->printAllEasy()) {
			$forwardStopped = 1;
			$stopReason = "creation problem.";
		}
		
		// forward
		if ($newid) {
			$newurl = "edit.tmpl.php?t=$tablename&id=".$newid."&xmode=labview";
			echo "<br>[<a href=\"".$newurl."\">... next page &gt;&gt;</a>]<br>";
		}
		if ($forwardStopped) {
			echo "<font color=gray>... Automatic forward stopped; reason: ".$stopReason."</font><br><br>\n";
		} else {
		    js__location_replace($newurl);
			exit;
		}
	}
}

$labObj->showform($sql);

$labObj->Hints();


htmlFoot();