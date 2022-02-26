<?php
/**
 * upload of a file to "data_path" as $tablename.$id.$ext
 * @package data.upload.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   $_FILES['userfile'] - uploaded file
  	 $tablename (table name)
  	 $id	(primary key)
	 $ext   (extension for file)
	 [$backurl]   optional BACKURL
 * @version0 2001-06-08
 */
session_start(); 


require_once ('reqnormal.inc'); // includes all normal *.inc files
require_once ("f.data_dir.inc");


$title = "Upload a data file";
$MAX_FILE_SIZE = 100000;

$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot();

$id = $_REQUEST["id"];
$tablename=$_REQUEST['tablename'];
$ext= $_REQUEST["ext"];
$backurl= $_REQUEST["backurl"];

$infoarr=array();
$infoarr["title"] = $title;
$infoarr["form_type"]= "obj";
$infoarr["obj_name"] = $tablename;
$infoarr["obj_id"]   = $id;
$infoarr["show_name"]= 1;

$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);
echo "<ul>";

if ($tablename=="" or $id=="") {
	htmlFoot("Error", "Please give table and ID");
} 

echo "Extension: '<B>$ext</B>'<br>";


$i_tableNiceName = tablename_nice2($tablename);


if ($tablename=="DB_USER") { // for DB_USER you can upload a file !!!
	if ($_SESSION['sec']['db_user_id'] != $id AND ($_SESSION['sec']['appuser'] != "root") ) {
		 htmlFoot("ERROR", "no write permissions to this user!");
	} 
} else {
	// 
	//   check   R I G H T S
	//
	$t_rights = tableAccessCheck( $sql, $tablename );
	if ( $t_rights["write"] != 1 ) {
		tableAccessMsg( $i_tableNiceName, "write" );
		htmlFoot();
	}
	
	$o_rights = access_check($sql, $tablename, $id);
	if ( !$o_rights["write"]) htmlFoot("ERROR", "You do not have write permission on this ".$i_tableNiceName."!");
}


$userfile_size = $_FILES['userfile']['size'];
$userfile  = $_FILES['userfile']['tmp_name'];
$userfile_name = $_FILES['userfile']['name'];
$userfile_type = $_FILES['userfile']['type'];

echo " userfile_size: $userfile_size bytes<br>";
//echo " userfile: $userfile <br>";
echo " userfile_name: $userfile_name<br>";
echo " userfile_type: $userfile_type <br><br>\n";

$filename=$userfile;

if ( $filename=="" ) htmlFoot("Error", "Give a File.");

if ( !file_exists($filename) ) {
	htmlFoot("Error", "File:$filename not exists!");
}

if ( filesize($filename)> $MAX_FILE_SIZE ) {
	htmlFoot("Error", "File size too big. Please choose a file which is smaller than $MAX_FILE_SIZE bytes!");
}

$dest_name = datadirC::datadir_filename( $tablename, $id ,$ext );

echo "New file: '<B>$dest_name</B>'<br>";

if ( !copy( $filename, $dest_name ) ) {

	echo "failed to copy $filename to $dest_name <br>\n";
	htmlFoot();
	
} else {

	echo "File imported ! Original filename was: $userfile_name  <br>";
	?>
	<script>
		if ("<? echo $backurl?>" != "") location.href='<? echo $backurl?>';
		else history.back();
	</script>
	<hr>
	<?
	
}


htmlFoot();
