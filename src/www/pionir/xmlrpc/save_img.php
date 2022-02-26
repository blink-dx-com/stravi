<?php
/**
 * upload an image
 * @package save_img.php
 * @swreq UREQ:-
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param    $img_id, 
 * @param    $_FILES['s_file'] - the uploaded file
 * @param   [$sess_id] for IconoClust XML-RPC
 * @version $Header: trunk/src/www/pionir/xmlrpc/save_img.php 59 2018-11-21 09:04:09Z $
 * @unittest exists
 */

	$img_id  = $_REQUEST['img_id'];
	$sess_id = $_REQUEST['sess_id'];

    if (!isset($img_id)) { header("icono-err-code: 2"); header("icono-err-text: No image id provided !"); echo("icono-err-text: No image id provided !"); exit(2); };
    if (!isset($_FILES['s_file']['tmp_name'])) { header("icono-err-code: 3"); header("icono-err-text: No file data provided !"); echo("icono-err-text: No file data provided !") ;exit(3); };
    if ( isset($sess_id) ) session_id($sess_id); /* due to XML-RPC */

session_start(); 


     require_once("db_access.inc");
     require_once("globals.inc");
     require_once("access_check.inc");
     require_once('glob.image.inc');
     
     /**
      * throw an error and log error
      * @param unknown $errnum
      * @param unknown $text
      */
     function throwError($errnum, $text) {
     	global $error;
     	$FUNCNAME= 'main';
     	
     	header("icono-err-code: ".$errnum);
     	header('icono-err-text: '.$text);
     	echo('icono-err-text: '.$text);
     	$error->set( $FUNCNAME, $errnum, $text );
     	$error->logError();
     	exit($errnum);
     }

     if( !glob_loggedin() ) {
     	throwError(4, 'Invalid session ID !');
     };

     header("session_id:".$sess_id);
     header("img_id:".$img_id);

     $img_name = imgPathFull( $img_id );
     
     header("image_name:".$img_name);

     global $error;
     $error = & ErrorHandler::get();
     $FUNCNAME= 'main';
     $sql = logon2( $_SERVER['PHP_SELF'] );
     $img_rights = access_check( $sql,"IMG",$img_id);

     if ($img_rights["write"]==0) {
     	throwError(5, 'No permissions to modify the image !');
     }
     
     if (!is_writable($_SESSION['globals']["img_path"])) {
     	throwError(6, 'Image-Serv-Path is not writable !');
     }
 
     copy($_FILES['s_file']['tmp_name'],$img_name);
     header("icono-err-code: 0");
     header("icono-err-text: Everything ok !");
     header("Content-type: text/html");
     echo("OK");

