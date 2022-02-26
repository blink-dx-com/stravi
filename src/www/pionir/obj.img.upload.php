<?php
/**
 * upload an image to ID=$id
 * @namespace core::obj::IMG
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @package obj.img.upload.php
 * @param $id = img.img_id 
 * @param $_FILES['userfile'] - uploaded file
 */
session_start(); 

require_once ('reqnormal.inc');
require_once ("glob.image.inc");
require_once ('f.update.inc');

// --------------------------------------------------- 
global $error, $varcol;

$error = & ErrorHandler::get();
$sqlo   = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();


$userfile_size = $_FILES['userfile']['size'];
$userfile  = $_FILES['userfile']['tmp_name'];
$userfile_name = $_FILES['userfile']['name'];
$userfile_type = $_FILES['userfile']['type'];
$filename = $userfile;

$id 		= $_REQUEST['id'];
$tablename	= 'IMG';
$title = "Image Upload";

$infoarr			 = NULL;
$infoarr['scriptID'] = '';
$infoarr['title']    = $title;
$infoarr['form_type']= 'obj';
$infoarr['design']   = 'norm';
$infoarr['obj_name'] = $tablename;
$infoarr['obj_id']   = $id;
$infoarr['checkid']  = 1;

$pagelib = new gHtmlHead();
$headarr = $pagelib->startPage($sqlo, $infoarr);


$o_rights = access_check( $sqlo, "IMG", $id );
	
if ( !$o_rights["write"] ){
	echo "<B>WARNING:</B> You do not have write permission on this image!<P>\n";
	return -1;
}


if (($userfile == "none") && ($userfile_name == "")) {
  echo "<p>";
  info_out("ERROR","You did not specify an image for upload.<br>Got back to do so.");
  return;
} 

if ($_SESSION['userGlob']['g.debugLevel'] > 0) {
  echo " userfile_size:$userfile_size <br>\n";
  echo " temp-file:$userfile <br>\n";
  echo " userfile_name:$userfile_name <br>\n";
  echo " MIME-type:$userfile_type <br>\n";
  echo " file-full-name: ".$_FILES["userfile"]["name"]."<br>\n";
}

if ($userfile == "none") {
  echo "<p>";
  info_out("ERROR", "Your image is probably too big for upload.<br>Please make the image smaller and come here again.");
  echo " userfile_size:$userfile_size <br>temp-file:$userfile <br>";
  echo " userfile_name:$userfile_name <br>";
  echo " MIME-type:$userfile_type<br>\n";
  return;
}

if ( filesize($filename)>0 ) {

  $dest_name = imgPathFull( $id );
	
  if (!move_uploaded_file($filename, $dest_name)) {
    info_out("ERROR", "Failed to move $filename to $dest_name");
  } else {
    $urlStr="edit.tmpl.php?tablename=IMG&id=$id";
    echo "File successfully uploaded! Original filename was: '$userfile_name' <br>\n";
    $argu=array();
    $argu["IMG_ID"]=$id;
	$argu["MIME_TYPE"]=$userfile_type;
	gObjUpdate::update_row( $sqlo, "IMG", $argu);
	
	require_once ( "javascript.inc" );
	js__location_replace( $urlStr,'image' );
	
  }
} else {
  info_out("ERROR", "File: '$userfile_name' not found or size=0. Also temporary file '$filename' does not exist.<br><br>");
}
?>
