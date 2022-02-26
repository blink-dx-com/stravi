<?
/*MODULE: img.segupload.php
  DESCR:  - upload an segmentation-image as attachment
  		  - SATTACH.KEY="SEG"
  AUTHOR: qbi
  RETURN:
  INPUT:  
		$img_id, 
		$_FILES['s_file']
		[$sess_id] for IconoClust XML-RPC
  OUTPUT:
  DB_MODIFIED:
  * @version $Header: trunk/src/www/pionir/xmlrpc/img.segupload.php 59 2018-11-21 09:04:09Z $
  */

	require_once('rest_support.inc');
	require_once("db_access.inc");
	
	$rest_lib = new rest_support();
	$error = & ErrorHandler::get();

	 $sess_id  = $_REQUEST['sess_id'];
	 $img_id   = $_REQUEST['img_id'];

     if (!isset($img_id)) { 
     	$rest_lib->errorout('No image id provided !', 2);
     };
     if ($_FILES['s_file']['tmp_name']==NULL) { 
     	$rest_lib->errorout('No file data provided !', 3);
     };
     if ( isset($sess_id) ) session_id($sess_id); /* due to XML-RPC */


	 session_start(); 

    
     require_once("globals.inc");
     require_once("access_check.inc");
	 require_once("o.SATTACH.subs.inc");
	 require_once ("o.SATTACH.mod.inc");
	 require_once("insert.inc");
	


	 if( !glob_loggedin() ) {
	 	$rest_lib->errorout('Invalid session ID !', 4);
	 };

     header("session_id:".$sess_id);
     header("img_id:".$img_id);
     # $img_name = $_SESSION['globals']["img_path"]. "/Org_".$img_id.".tif";
	 $imgseg_name = "";
     # header("image_name:".$img_name);

     $sql = logon2( $_SERVER['PHP_SELF'] );
	 
     $img_rights = access_check( $sql,"IMG",$img_id);

	if ($img_rights["write"]==0) {
		$rest_lib->errorout('No permissions to modify the image !', 5);
	}  
	 
	$tablename = "IMG";
	$attachObj = new cSattachSubs();
	$rel_id = $attachObj->getRelIDbyKey( $sql, $tablename, $img_id, "SEG" );
	
	if (!$rel_id) {
		$atmodLib = new oSattachMod();
		$atmodLib->setObj($tablename, $img_id);
		
		$argu = array();
		$argu["NAME"]       = "segimg.png";
		$argu["MIME_TYPE"]  = "image/png";
		$argu["KEY"]        = "SEG";
		$rel_id = $atmodLib->insertAtt( $sql, $argu );
	 
	}
	
	$imgseg_name = $attachObj->getDocumentPath($tablename, $img_id, $rel_id);
	if ($error->Got(READONLY))  {
		$rest_lib->errorout('Error on Attachment actions', 6);
		return;
	}
	
	copy($_FILES['s_file']['tmp_name'],$imgseg_name);
	header("icono-err-code: 0");
	header("icono-err-text: Everything ok !");
	header("Content-type: text/html");
	echo("OK");

