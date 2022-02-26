<?
/**
 * download a document
 * @package get_link.php
 * @swreq  
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param   $link_id ID of document
            [$mime] show/download with special mime-type (overwrite mime of the document)
            $sess_id - Session-ID
 * @version $Header: trunk/src/www/pionir/xmlrpc/get_link.php 59 2018-11-21 09:04:09Z $
 */
require_once("db_access.inc");

/**
 * link_errorout('',);
 * @param unknown $errnum
 * @param unknown $text
 */
function link_errorout($text, $errnum) {
	global $error;
	header("icono-err-code: ".$errnum);
	header("icono-err-text: ".$text);
	echo("icono-err-text: ".$errnum.':'.$text);

	$error->set(__FUNCTION__, $errnum, $text);
	$error->logError();

	exit($errnum);
}


$sess_id  = $_REQUEST['sess_id'];
$link_id  = $_REQUEST['link_id'];
$mime     = $_REQUEST['mime'];

$error = & ErrorHandler::get();
if ( isset($sess_id) ) session_id($sess_id); 
session_start(); 

if ( !$link_id or !is_numeric($link_id) ) { link_errorout("No link id provided !",2); };

require_once("globals.inc");
require_once("access_check.inc");
require_once("o.LINK.subs.inc");
require_once("down_up_load.inc");


if( !glob_loggedin() ) {
  link_errorout(' Invalid session ID!',3);
} //invalid session id

if (isset($sess_id))  header("session_id:".$sess_id);
if (isset($link_id))  header("link_id:".$link_id);
if (!isset($mime)) $mime = "";

$sql = logon2( $_SERVER['PHP_SELF'] );

$tmp_rights = access_check($sql,"LINK",$link_id);

if ($tmp_rights["read"]==0) {
	link_errorout('No read rights!',5);
}

$link_name = linkpath_get( $link_id );

if( !file_exists($link_name) ) {
	link_errorout( "$link_name: No File Found !",4 );
}

$ret = $sql->query("SELECT name, mime_type FROM link WHERE link_id = $link_id");
$sql->ReadRow();

$tmp_filename = $sql->RowData[0];
$link_mime    = $sql->RowData[1];
if ($mime!="") $link_mime=$mime; // overwrite mime-type from document

$file_size     = filesize($link_name);

if (!empty($link_mime)) {
  set_mime_type($link_mime, $tmp_filename);
} else {
  set_mime_type("application/octet-stream", $tmp_filename);
}

header("icono-err-code: 0");
header("icono-err-text: Everything OK !");
header('Content-Length: '.$file_size);
readfile( $link_name ,"r");
