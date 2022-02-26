<?php
/**
 *   create a version control entry
  		 - create an entry
		 - copy LAST document-attachment to last entry
		 - get new uploaded document
 * @package obj.link.vernew.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @version
 * @param   $id (LINK_ID)
 */
session_start(); 


require_once ("reqnormal.inc"); // includes all normal *.inc files
require_once("o.LINK.versctrl.inc");

// --------------------------------------------------- 
global $error, $varcol;

$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] );
//$sql2  = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();

$id 		= $_REQUEST["id"];


$tablename="LINK";
$i_tableNiceName="document";

$title = "Create a version control entry";

#$infoarr['help_url'] = 'o.EXAMPLE.htm';
$infoarr=array();
$infoarr["title"] = $title;
$infoarr["form_type"]= "obj";
$infoarr["obj_name"] = $tablename;
$infoarr["obj_id"]   = $id;
$infoarr["checkid"]  = 1;

$infox = NULL;
$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);

// 
//   check   R I G H T S
//
$t_rights = tableAccessCheck( $sql, $tablename );
if ( $t_rights["write"] != 1 ) {
	tableAccessMsg( $i_tableNiceName, "write" );
	htmlFoot();
}

$o_rights = access_check($sql, $tablename, $id);
if ( !$o_rights["insert"]) htmlFoot("ERROR", "You do not have insert permission on this ".$i_tableNiceName."!");

$versionObj = new oLINKversC();

$argu = NULL;
$newpos = $versionObj->create( $sql, $id, $argu);
		
if ($error->printAll())  {
	htmlFoot();
}

// put new document to the current place ???
$newurl = "edit.tmpl.php?t=LINK&id=".$id;
if ( $_SESSION['userGlob']["g.debugLevel"]>0 ) {
	echo "... automatic page forward stopped due to debug-level.<br>";
	echo "[<a href=\"".$newurl."\">next page &gt;&gt;</a>]<br>";
	htmlFoot();
} 
?>
<script language="JavaScript">
	location.replace("<?php echo $newurl?>");            
</script>
<? 	 


