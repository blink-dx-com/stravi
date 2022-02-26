<?php

/**
 * - add attachment to object of a table
 * - attach file with key via XML-RPC
 * @swreq UREQ: 0001423
 * @package obj.sattach.upload.php
 * @param  $sqlo
 * @param  	$table_name (name of table)
 * 			$obj_id (id of object)
 * 			$sess_id (XML-RPC Session ID)
 * 			$_FILES['s_file'] (data of uploaded file)
 * 			$key (key of the attachment)
 * @return 	$rel_id (position of the attachment on the object)
 */	
require_once("o.SATTACH.mod.inc");
require_once("o.SATTACH.subs.inc");
require_once("db_access.inc");
require_once("globals.inc");
require_once("access_check.inc");
require_once('rest_support.inc');

$error = & ErrorHandler::get();
$rest_lib = new rest_support();

$sess_id  = $_REQUEST['sess_id'];
$obj_id   = $_REQUEST['obj_id'];
$table_name   = $_REQUEST['table_name'];
$key      = $_REQUEST['key'];

if (!isset($table_name)) $rest_lib->errorout("No table name provided.", 2);
if (!isset($obj_id) or !is_numeric($obj_id) )     $rest_lib->errorout("No object id provided." , 3);
if ($_FILES['s_file']['tmp_name']==NULL)  $rest_lib->errorout("No file provided.",4);

if ( isset($sess_id) ) session_id($sess_id); /* due to XML-RPC */

session_start(); 


$atmodLib = new oSattachMod();
$atsubLib = new cSattachSubs();

$atmodLib->setObj($table_name, $obj_id);


if ( !glob_loggedin() ) $rest_lib->errorout("Invalid session ID !", 5); //invalid session id
  
$error  = & ErrorHandler::get();
$sqlo   = logon2();
$attach_rights = access_check( $sqlo,"SATTACH",$obj_id);

if ($attach_rights["insert"]==0) {
	$rest_lib->errorout("ERROR: No permissions to attach a file!", 6);
}
else {
	$rel_id = $atsubLib->getNextRelID($sqlo, $table_name, $obj_id);
	$dest_path = $atsubLib->getDocumentPath($table_name, $obj_id, $rel_id);
	if (!copy($_FILES['s_file']['tmp_name'], $dest_path)){
		$rest_lib->errorout("Copy of file failed", 7);
	};
	
	$argu["NAME"] 		= $_FILES['s_file']['name'];
	$argu["KEY"]		= $key;
	$argu["MIME_TYPE"] 	= $_FILES['s_file']['type'];
	$rel_id = $atmodLib->insertAtt( $sqlo, $argu );
	echo("OK");
}