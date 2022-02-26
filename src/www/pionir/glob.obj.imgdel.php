<?php
/**
 * delete the JPG-image from object on DATA-location
 * currently only used for CONTACT !
 * @package glob.obj.imgdel.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   $id
  		 $tablename
 * @version0 2001-05-21
 */
session_start(); 


require_once ("reqnormal.inc");
require_once ("f.data_dir.inc");

$id = $_REQUEST["id"];
$tablename=$_REQUEST['tablename'];

$backurl = "edit.tmpl.php?t=".$tablename."&id=$id";
$sql     = logon2( $_SERVER['PHP_SELF'] );

$i_tableNiceName = tablename_nice2($tablename);


$title       = "Delete attached image from object";

$infoarr			 = NULL;

$infoarr["title"] = $title;
$infoarr["form_type"]= "obj";
#$infoarr['help_url'] = 'o.EXAMPLE.htm';
$infoarr["obj_name"] = $tablename;
$infoarr["obj_id"]   = $id;
$infoarr["show_name"]= 1;


$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);

echo " Delete image now ...<br><br>";

$o_rights = access_check($sql, $tablename, $id);
if ( !$o_rights["write"]) htmlFoot("ERROR", "You do not have write permission on this ".$i_tableNiceName."!");


$pfilename = datadirC::datadir_filename( $tablename, $id ,"jpg" );
if ( file_exists($pfilename) ) {
	unlink ($pfilename);
}

echo " Image deleted ...<br><br>";


if ( $backurl == "" ) htmlFoot("<hr>");
?>
<script>
	location.href="<?echo $backurl?>";
</script>
<?

htmlFoot();