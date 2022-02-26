<?php
/**
 * Partisan-Frankenstein
 * 
 * - generate new database structure SQL-strings for PARTISAN
 * $Header: trunk/src/www/pionir/rootsubs/db_transform/f.dbStructCrea.php 59 2018-11-21 09:04:09Z $
 * @package f.dbStructCrea.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param $go : defines the action
			"0"=>"Define type",
			1=>"Give columns", 
			2=>"Create structure", 
			3=>"Edit a column",
			4=>"Delete a column",
			5=>"Create SQL-commands",
			6=>"Remove all structures",
			7=> "drop table"
			9=> do_exportArray
			10=> import array
		  $subact - subaction
		    'view'
		    'save'
 * @param $parx
 */


extract($_REQUEST); 
session_start(); 


require_once ("reqnormal.inc"); // includes all normal *.inc files
require_once ("f.dbStructGui.inc"); 

// --------------------------------------------------- 
global $error, $varcol;

$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] ); // URL-link for the first db-login; e.g. for object ($_SERVER['PHP_SELF']."?id=".$id)
$sql2  = logon2( );
if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();


$title       		 = "Partisan-Frankenstein - Database structure creator for PARTISAN";
$infoarr			 = NULL;

$infoarr["title"] = $title;
$infoarr["title_sh"] = 'Partisan-Frankenstein';
$infoarr["form_type"]= "tool";

$infoarr["locrow"]= array( array("index.php", "Transform DB versions" ) );
#$infoarr['help_url'] = "o.EXAMPLE.htm";


$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);
echo "<img src=\"ic.f.dbStructCrea.png\">";
echo "<ul>";

if ( !glob_isAdmin() ) {
     htmlErrorBox( "Error",   
     "Only root can execute this!",
     "For security reason it is not allowed for common users" );
     htmlFoot();
}

$mainLib = new fCbStructGui('f.dbStructCrea');

$mainLib->init($go, $parx, $_REQUEST['subact']);

$mainLib->doit($sql);

if ($error->Got(READONLY))  {
    $error->printAll();
} else {
	 
}

htmlFoot();
