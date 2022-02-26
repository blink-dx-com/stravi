<?php
/**
 * download an image
 * @package obj.img.download.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   $id (IMG_ID)
 */
session_start(); 


require_once ("reqnormal.inc");
require_once ("down_up_load.inc");
require_once ("glob.image.inc");

function this_error( &$sql, $id, $errortext ) {
	$tablename ="IMG";

	$title       = "Download an image";

$infoarr=array();
$infoarr["title"] = $title;
$infoarr["form_type"]= "obj";
	$infoarr["obj_name"] = $tablename;
	$infoarr["obj_id"]   = $id;
	$infoarr["show_name"]= 1;
	
	
	$pagelib = new gHtmlHead();
	$pagelib->startPage($sql, $infoarr);
	
	htmlFoot("Error", $errortext);
	
}

global $error;
$error = &ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot();
$id = $_REQUEST["id"];

if (!$id)  this_error( $sql, $id, "No image-ID given." );
$img_path = imgPathFull( $id );

if ( !file_exists($img_path) )  this_error( $sql, $id, "Image-file not found on server." );

$img_rights = access_check($sql, 'IMG', $id);

if ( $img_rights["read"]==0) {
	this_error( $sql, $id, "No access-read-right for this image" );
}

$filesizex = filesize($img_path);

$sqls = "select name, MIME_TYPE from IMG where IMG_ID=".$id;
$sql->query($sqls);
$sql->ReadRow();
$imgname    = $sql->RowData[0];
$mimetypeDb = $sql->RowData[1];

$imgname = str_replace("\\", "/", $imgname); // convert back-slashes
$imgname = basename($imgname); // remove slashes ...
$mime_type  = $mimetypeDb;
if ($mime_type=="")  {
	// guess mime_type
    $pointpos = strrpos($imgname, ".");
	if ($pointpos>0) {
	    $extension = substr($imgname,$pointpos+1);
		$mime_type = "image/".$extension;
	}
}
if ($imgname=="") $imgname=$id;
$mimeopt=array("size"=>$filesizex);
set_mime_type ( $mime_type, $imgname, $mimeopt );

readfile( $img_path ,"r");

// ready