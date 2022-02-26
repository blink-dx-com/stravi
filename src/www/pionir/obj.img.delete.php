<?php
/**
 * delete image attachment file
 * @package  obj.img.delete.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   $id   (image ID)
 *     int $go
 */ 
session_start(); 


require_once ("reqnormal.inc");
require_once("glob.image.inc");
require_once ( "javascript.inc" );

$id = $_REQUEST["id"];
$go = $_REQUEST["go"];

$back_url="edit.tmpl.php?t=IMG&id=".$id;

$sql = logon2( $_SERVER['PHP_SELF'] );
$tablename = "IMG";

$title       = "Delete attached image file";

$infoarr			 = NULL;

$infoarr["title"] = $title;
$infoarr["form_type"]= "obj";
$infoarr['help_url'] = 'o.EXAMPLE.htm';
$infoarr["obj_name"] = $tablename;
$infoarr["obj_id"]   = $id;
$infoarr["show_name"]= 1;

// $infoarr["version"] = '1.0';	// version of script

$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);
echo "<ul><br>\n";

$o_rights = access_check( $sql, "IMG", $id );
if (!$o_rights["write"]) {
	htmlFoot("ERROR", "No write-right  on this object.");
}


$filename= imgPathFull( $id );

if ( !file_exists($filename) ) {
	$pagelib->htmlFoot("ERROR", "Delete failed. File not exists on server."  );
}

if ( !$go ) {
	echo "<B>Do you really want to delete the uploaded image?</B><br><br>";	  
	echo "<a href=\"".$_SERVER['PHP_SELF']."?id=".$id."&go=1\" > <b>YES</B> </a> &nbsp;|&nbsp;";
	echo "<a href=\"".$back_url."\">NO</a><br>";
	echo "</ul>";
	$pagelib->htmlFoot();
} else {
	$retval = unlink ($filename);
}




js__location_replace($back_url);  

htmlFoot();
