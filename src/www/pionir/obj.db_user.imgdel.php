<?php
/**
 * delete user image
 * @package obj.db_user.imgdel.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param  $id (user id)
 * @version0 2001-05-21 
 */

session_start(); 

require_once ('reqnormal.inc');
require_once("f.data_dir.inc");


$sql    = logon2( $_SERVER['PHP_SELF'] );

$id = $_REQUEST["id"];
$backurl = "edit.tmpl.php?tablename=DB_USER&id=$id";
$tablename='DB_USER';
$title = "Delete user image";
$infoarr			 = NULL;
$infoarr["title"]    = $title;
$infoarr["form_type"]= "obj"; // "tool", "list"
$infoarr["obj_name"] = $tablename;
$infoarr["obj_id"]   = $_REQUEST["id"];

$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);


$updateAllow=0;
if (($_SESSION['sec']['db_user_id'] == $id) || ($_SESSION['sec']['appuser']=="root")) {
  $updateAllow=1;
}

if (!$updateAllow) {
	echo "ERROR: no permission.<br>";
	return;
}

$pfilename = datadirC::datadir_filename( "DB_USER", $id ,"jpg" );
if ( file_exists($pfilename) ) {
	unlink ($pfilename);
}

 ?>
  <script>
	location.href="<?echo $backurl;?>";
  </script>
 <?

 $pagelib->htmlFoot();
