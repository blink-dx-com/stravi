<?php
/**
 * Delete a document attachment
 * @package glob.obj.delete.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   $id
 * @param   $go
 */

// extract($_REQUEST); 
session_start(); 


require_once("db_access.inc");
require_once("globals.inc");
require_once("access_check.inc");
require_once("o.LINK.subs.inc");
require_once ('func_head.inc');
require_once("o.LINK.versctrl.inc");

$retval=1;
$sql = logon2( $_SERVER['PHP_SELF'] );


$title       = 'Delete document attachment';

$id=$_REQUEST['id'];
$go=$_REQUEST['go'];

$infoarr=array();
$infoarr["title"] = $title;
$infoarr["form_type"]= "obj";
$infoarr["obj_name"] = "LINK";
$infoarr["obj_id"]   = $id;
$infoarr["checkid"]  = 1;


$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);
echo "<ul>";

$o_rights = access_check( $sql, "LINK", $id );

if (!$o_rights["write"]) {
	echo "ERROR: no right to write.<br>";
	return (-1);
}
$back_url="edit.tmpl.php?t=LINK&id=".$id;

$filename = linkpath_get( $id );

$versionObj = new oLINKversC();
if ( $versionObj->isPossible() ) {
	if ($versionObj->isUnderControl($sql, $id) ) {
		htmlInfoBox( "Delete denied", "The attached document can not be deleted.<br>Reason: The document is under version control!", "", "WARN" );
		htmlFoot();
	}
}

if ( file_exists($filename) ) {

	if ( !$go ) {
		$iopt = array();
		$iopt["icon"] = "ic.del.gif";
		htmlInfoBox( "Delete uploaded document", "", "open", "INFO", $iopt );
		echo "<center>";
		echo "<br><B>Do you want to delete the uploaded document?</B><br><br>";	  
		echo "<a href=\"".$_SERVER['PHP_SELF']."?id=".$id."&go=1\" > <b>YES</B> </a> &nbsp;|&nbsp;";
		echo "<a href=\"".$back_url."\">NO</a><br>";
		htmlInfoBox( "", "", "close" );
		return (0);
	} else {
		$retval = unlink ($filename);
	}

} else {
	echo "ERROR: file not exists on server.<br>";
	$retval=-1;
}	

if ($retval>0) {
	?>
	<script language="JavaScript">
		location.replace('<?echo $back_url?>');
	</script>
	<?
} else {
	echo "<br><br><a href=\"".$back_url."\">BACK</a>";
}


htmlFoot();
