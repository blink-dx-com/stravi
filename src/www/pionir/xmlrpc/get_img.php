<?
/**
 *  download an image as binary
 * @package get_img.php
 * @swreq UREQ:xxxxxxxxxxxx
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param   $img_id
  	 [$sess_id] for IconoClust XML-RPC
 * @version $Header: trunk/src/www/pionir/xmlrpc/get_img.php 59 2018-11-21 09:04:09Z $
 */


require_once("db_access.inc");
require_once("globals.inc");
require_once("access_check.inc");
require_once('rest_support.inc');
require_once 'o.IMG.file.inc';

$error = & ErrorHandler::get();
$rest_lib = new rest_support();

$sess_id  = $_REQUEST['sess_id'];
$img_id   = $_REQUEST['img_id'];

if (!isset($img_id)) { $rest_lib->errorout('No image id provided !', 2); };
if ( isset($sess_id) ) session_id($sess_id); /* due to XML-RPC */


session_start(); 


if( !glob_loggedin() ) { 
  $rest_lib->errorout("Invalid session ID !",3);
} 

if (isset($sess_id)) header("session_id:".$sess_id);
if (isset($img_id))  header("img_id:".$img_id);

$img_name = oIMG_fileC::imgPathFull($img_id);

header("image_name:".$img_name);
if(!file_exists($img_name)) { //image does not exist
  $rest_lib->errorout('Image file does not exist !',4);
}


$sql = logon2( $_SERVER['PHP_SELF'] );
$img_rights = access_check($sql, 'IMG', $img_id);

if ($img_rights["read"]==0) {
	$rest_lib->errorout('No permissions for the image !',5);
} else {
  header("icono-err-code: 0");
  header("icono-err-text: Everything ok !");
  header("Content-type: image/tiff");
  readfile( $img_name ,"r");
}
