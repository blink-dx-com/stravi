<?php
/**
 * delete selected ($sel[REL_ID]) attachments of an object
 * first Version: VERSION: 0.1 - 20020904
 * @package obj.sattach.del.php
 * @swreq   SREQ:0002033: o.SATTACH loeschen
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param   
 * 	$t   ( tablename ) 
  	$id  (e.g. ARRAY_LAYOUT_ID)
	$sel[REL_ID] = 1
 */
 
session_start(); 


require_once ("reqnormal.inc"); // includes all normal *.inc files
require_once ("visufuncs.inc");
require_once ("o.SATTACH.subs.inc");
require_once ( "javascript.inc" );
require_once ("o.SATTACH.mod.inc");

global $error, $varcol;

$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot();
$t =$_REQUEST['t'];
$id=$_REQUEST['id'];
$sel=$_REQUEST['sel'];

$tablename = $t;

$title       = "Delete attachments of object";
$infoarr=array();
$infoarr["title"]    = $title;
$infoarr["title_sh"]    =  "Delete attachments";
$infoarr["form_type"]= "obj";
$infoarr["obj_name"] = $tablename;
$infoarr["obj_id"]   = $id;
$infoarr["checkid"]  = 1;


$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);
echo '<ul>';
$pagelib->do_objAccChk($sql);
$pagelib->chkErrStop();

if ( !sizeof($sel) ) {
	$pagelib->htmlFoot('WARN', 'No attachment selected');
}

$atSubLib = new cSattachSubs();
$atmodLib = new oSattachMod();
$atmodLib->setObj($tablename, $id);

$cnt=0;
$suberror=0;
foreach( $sel as $rel_id=>$val) {
	$infoarr = $atSubLib->getEntryByRelid( $sql, $tablename, $id, $rel_id);
	
	// SUBREQ: 001: check for ARCHIVE
	if ($infoarr['ARCHIVE']>0) {
		echo "<b>ERROR</b> attachment with rel_id: ".$rel_id." not allowed to delete, is_archive.<br>";
		$suberror=1;
		continue;
	}
	$atmodLib->delAttach($sql, $rel_id);
	if ($error->Got(READONLY))  {
		$error->printAllEasy();
		$error->reset();
		continue;
	}
	$cnt++;
}

echo $cnt.' attachments were deleted'."<br />\n";

if ($suberror) {
	$pagelib->htmlFoot();
}

$newurl = "obj.sattach.view.php?t=$tablename&id=$id";
js__location_replace($newurl, 'attachment list' ); 
 
$pagelib->htmlFoot();

