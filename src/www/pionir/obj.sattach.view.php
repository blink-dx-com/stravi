<?php
/**
 * list functions for attach
 * @package obj.sattach.view.php
 * @swreq UREQ:0001074: o.SATTACH > show attachments of an object 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param $t   ( tablename ) 
  		  $id   (e.g. ARRAY_LAYOUT_ID)
 */
session_start(); 


require_once ("reqnormal.inc"); // includes all normal *.inc files
require_once ("visufuncs.inc");
require_once ("o.SATTACH.subs.inc");
require_once ('gui/o.SATTACH.ed.inc');

global $error;

$error = & ErrorHandler::get();
$sqlo  = logon2( $_SERVER['PHP_SELF'] );
//$sqlo2 = logon2(  );
if ($error->printLast()) htmlFoot();
//$varcol = & Varcols::get();
$t =$_REQUEST['t'];
$id=$_REQUEST['id'];

$tablename = $t;
$attachtab = "SATTACH";
$editallow = 1;
$i_tableNiceName =  tablename_nice2($tablename);

$title       = "Attachments to ".$i_tableNiceName;
$infoarr=array();
$infoarr["title"] = $title;
$infoarr["title_sh"] = "Attachments";
$infoarr["form_type"]= "obj";
$infoarr["obj_name"] = $tablename;
$infoarr["obj_id"]   = $id;
$infoarr["icon"]     = "images/icon.SATTACH.gif";
$infoarr["checkid"]  = 1;
$infoarr['help_url'] = 'o.SATTACH.html';

$pagelib = new gHtmlHead();
$pagelib->startPage($sqlo, $infoarr);

// 
//   check   R I G H T S
//
$t_rights = tableAccessCheck( $sqlo, $tablename );
if ( $t_rights["read"] != 1 ) {
	tableAccessMsg( $i_tableNiceName, "read" );
	htmlFoot();
}
if ( $t_rights["write"] != 1 ) {
	$editallow=0;
}
$o_rights = access_check($sqlo, $tablename, $id);
if ( !$o_rights["read"] ) htmlFoot("ERROR", "no read permissions on object!");
if ( !$o_rights["write"])  $editallow=0;


$attach_lib = new o_SATTACH_ed_sub();
$attach_lib->show_form($sqlo, $tablename, $id, $editallow);


htmlFoot();