<?php
/**
 * - upload an iconoclust script or package
   - only used by ICONOCLUST !!!
   - downgrade compatibility (mime and hash optional)
 * @package save_link.php
 * @swreq UREQ:xxxxxxxxxxxx
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param $link_id, 
  		  $s_file
		  # if $mime AND $hash
		    [$mime]  : ["application/iconoclust-run"], "application/iconoclust-pck"
		    [$hash]  : hash-code of package (can only be calculated by ICONOCLUST)
  	 	  $sess_id for IconoClust XML-RPC
 * @version $Header: trunk/src/www/pionir/xmlrpc/save_link.php 59 2018-11-21 09:04:09Z $
 * @unittest exists
 */


function this_errorHandle($errnum, $errtext) {
	header("icono-err-code: $errnum");
	header("icono-err-text: $errtext");
	echo  ("icono-err-text: $errtext");
	exit($errtext); 
}

$sess_id = $_REQUEST['sess_id'];
$link_id = $_REQUEST['link_id'];
$hash    = $_REQUEST['hash'];
$mime    = $_REQUEST['mime'];

if (!isset($link_id)) { 
	this_errorHandle(2, "No link id provided !");
};
if ($_FILES['s_file']['tmp_name']==NULL) { 
	this_errorHandle(3, "No file data provided !");
};
if ( isset($sess_id) ) session_id($sess_id); /* due to XML-RPC */


session_start(); 


     require_once("db_access.inc");
     require_once("globals.inc");
     require_once("access_check.inc");
     require_once("f.modulLog.inc");

     require_once("o.LINK.subs.inc");

     if( !glob_loggedin() ) { 
	 	this_errorHandle(4, "Invalid session ID !"); 
	 }; //invalid session id

     header("session_id:".$sess_id);
     header("link_id:".$link_id);
     $link_name = linkpath_get( $link_id );
     header("link_name:".$link_name);

	 
	 global $error, $varcol;

	 $error   = & ErrorHandler::get();
	 $hashUse = NULL;
     $sql 	  = logon2( $_SERVER['PHP_SELF'] );
   
     
     $modLogLib = new fModulLogC();
     $sqldummy  = NULL;
     $modLogLib->logModul($sqldummy);
	 
     if ( !gObject_exists ($sql, "LINK", $link_id) ) {
     	this_errorHandle(7, "Object LINK ID:".$link_id." not exists!");
     }
     $img_rights = access_check( $sql, "LINK", $link_id);
	 
	 $mimeUse = "application/iconoclust-run"; // default
	 if ($mime!="" or $hash!="")  {
	 	if ($mime=="" or $hash=="")  {
			this_errorHandle(6, "mime and hash must be given !");
		}
	 	$mimeUse = $mime;
		$hashUse = $hash;
	 }
     if ($img_rights["write"]==0) {
	 	this_errorHandle(5, "No permissions to modify the link !");  
	 } else {
	 
	 	$upopt = NULL;
		$upopt["extHash"] =$hashUse;
		$upopt["iconoapp"]=1;
		
	 	$linkUpObj = new oLinkUpload();
		$linkUpObj->link_cp_file( $sql, $_FILES['s_file']['tmp_name'],
					$link_id, $_FILES[$s_file]['name'], $mimeUse, $upopt);
		if ($error->Got(READONLY)) {
			$errLast   = $error->getLast();
			$error_txt = $errLast->text;
			$error_id  = $errLast->id;
			header("icono-err-code: ".$error_id);
			header("icono-err-text: error during upload !");
			header("Content-type: text/html");
		}
		
		header("icono-err-code: 0");
		header("icono-err-text: Everything ok !");
		header("Content-type: text/html");
		echo("OK");
     };
