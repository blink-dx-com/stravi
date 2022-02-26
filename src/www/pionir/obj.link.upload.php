<?php
/**
 *  - upoad a single document file
  	- can handle version control management
 * @package obj.link.upload.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param    $id of LINK
 * @param array $argu
 */
session_start(); 


require_once ("o.LINK.subs.inc");
require_once ("reqnormal.inc"); 
require_once("o.LINK.versctrl.inc");

$error = & ErrorHandler::get();
$sql 	  = logon2( $_SERVER['PHP_SELF'] );

$id 		= $_REQUEST["id"];
$argu = $_REQUEST["argu"];

$o_rights = access_check( $sql, "LINK", $id );
$tablename="LINK";
$title       = "Upload a document";

$infoarr=array();
$infoarr["title"] = $title;
$infoarr["form_type"]= "obj";
$infoarr["obj_name"] = $tablename;
$infoarr["obj_id"]   = $id;
$infoarr["checkid"]  = 1;


$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);
echo "<ul>";

if (!$o_rights["insert"]) {
	htmlFoot( "ERROR", "No 'insert' right on this document.");
}

$versionObj = new oLINKversC();
$newurl = "edit.tmpl.php?t=LINK&id=".$id;

$userfile_size = $_FILES['userfile']['size'];
$userfile  = $_FILES['userfile']['tmp_name'];
$userfile_name = $_FILES['userfile']['name'];
$userfile_type = $_FILES['userfile']['type'];

if ( $_SESSION["userGlob"]["g.debugLevel"]>0 ) {
    echo "<B>INFO:</B> DEBUG-mode: <br>\n";
	echo " userfile_size:$userfile_size <br>temp-file:|$userfile| <br>";
	echo " userfile_name:$userfile_name <br>";
	echo " MIME-type:$userfile_type<br><br>";
}
$filename = $userfile;

if ( $versionObj->isPossible() ) {
	if ( $versionObj->isUnderControl($sql, $id) ) {
		// do control things ...
		echo "... create a new version<br>\n";	
		
		$newpos = $versionObj->create( $sql, $id, $argu);
		
		if ($error->printAll())  {
			htmlFoot();
		}

		$newurl="obj.link.versedit.php?id=".$id."&pos=".$newpos;
		
	}
}

$linkUpObj = new oLinkUpload();
$linkUpObj->link_cp_file( $sql, $filename, $id, $userfile_name, $userfile_type );
if ($error->Got(READONLY))  {
	$error->set("main",1, "Upload or object info update failed. Forwarding stopped.");
	$error->printAllEasy();
	htmlFoot();
}
echo "... uploaded file successfully copied to server.<br>\n";




if ( $_SESSION["userGlob"]["g.debugLevel"]>0 ) {
    echo "<B>DEBUG-mode</B><br>\n";
	echo "... automatic page forward stopped due to debug-level.<br>";
	echo "[<a href=\"".$newurl."\">next page &gt;&gt;</a>]<br>";
	htmlFoot();
}
?>
<script language="JavaScript">
  	location.href="<?echo $newurl?>";
</script>

