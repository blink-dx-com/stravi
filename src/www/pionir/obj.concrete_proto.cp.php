<?php
/**
 * Paste steps to protocol
 * @package obj.concrete_proto.cp.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   $id (destination protocol)
 * @version0 2002-01-21 
 */ 
session_start(); 

require_once ('reqnormal.inc');
require_once ("f.clipboard.inc");

$sql    = logon2( $_SERVER['PHP_SELF'] );
$error  = & ErrorHandler::get();
$id = $_REQUEST["id"];

$title  = "Paste protocol steps";
$tablename = 'CONCRETE_PROTO';
$infoarr= NULL;

$infoarr["title"] = $title;
$infoarr["form_type"]= "obj";
$infoarr["scriptID"] = "obj.concrete_proto.cp.php";
$infoarr["obj_name"] = $tablename;
$infoarr["obj_id"]   = $_REQUEST["id"];
$infoarr["checkid"]  = 1;
$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);
$pagelib->do_objAccChk($sql);
$pagelib->chkErrStop();
 
		
$clipobj = new clipboardC();
$elemarr = $clipobj->obj_get ( $tablename );
$id_src  = $elemarr[0];

if ( !$id_src ) {
	$pagelib->htmlFoot('ERROR', "no protocol (concrete) in clipboard. (2)");
}


$sqls= "select count(1) from CONCRETE_PROTO_STEP where CONCRETE_PROTO_ID=".$id;
$sql->query("$sqls");
$sql->ReadRow();
$num_steps=$sql->RowData[0];
if ( $num_steps>0 ) {
	$pagelib->htmlFoot('ERROR', "destination protocol contains $num_steps steps.<br>Please remove them first!");

}

$sqls= "select ABSTRACT_PROTO_ID from concrete_proto where CONCRETE_PROTO_ID=".$id_src;
$sql->query("$sqls");
$sql->ReadRow();
$abs_proto_src=$sql->RowData[0];

$sqls= "select ABSTRACT_PROTO_ID from concrete_proto where CONCRETE_PROTO_ID=".$id;
$sql->query("$sqls");
$sql->ReadRow();
$abs_proto_dest=$sql->RowData[0];

if ($abs_proto_dest != $abs_proto_src) {
	$pagelib->htmlFoot('ERROR', "protocols (abstract) must be the same.");
}

$desturl = 'glob.obj.assocpaste.php?tablename=CONCRETE_PROTO&id='.$id.'&parx[src_id]='.$id_src .
			'&parx[assoc_name]=CONCRETE_PROTO_STEP&parx[mode]=single&go=2';
js__location_replace( $desturl, 'paste steps' ); 

$pagelib->htmlFoot();

