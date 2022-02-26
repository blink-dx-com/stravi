<?php
/**
 * analyse serialized object from file
 * $Header: trunk/src/www/pionir/rootsubs/f.SerialObjAna.php 59 2018-11-21 09:04:09Z $
 * @package 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param string file 
 */

extract($_REQUEST); 
session_start(); 

require_once ('reqnormal.inc');

// --------------------------------------------------- 
global $error, $varcol;

$error = & ErrorHandler::get();
$sqlo  = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();

$filename 		= $_REQUEST['file'];

$title		= 'analyse serialized object from file';

$infoarr			 = NULL;
$infoarr['scriptID'] = '';
$infoarr['title']    = $title;
$infoarr['form_type']= 'tool'; // 'tool', 'list'
$infoarr['design']   = 'norm';
$infoarr['locrow']   = array( array('rootFuncs.php', 'home') );

$pagelib = new gHtmlHead();
$headarr = $pagelib->startPage($sqlo, $infoarr);


if ( !glob_isAdmin() ) {
     htmlErrorBox( "Error",   
     "Only root can execute this!",
     "For security reason it is not allowed for common users" );
     htmlFoot();
}

if ( $filename==NULL ) {
	$pagelib->htmlFoot("ERROR", 'Need a file name');
}

$LINE_LENGTH = 32000000;
$FH = fopen($filename, 'r');
if ( !$FH ) {
  echo "<B>Error:</B> Can't open file '$filename'<br>\n";
  return;
}
 
$line = fread( $FH, filesize($filename) );
$outputArr = unserialize($line);
echo "<pre>";
print_r($outputArr);
echo "</pre>";
$pagelib->htmlFoot();
