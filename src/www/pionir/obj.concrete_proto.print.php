<?php
/**
 * show protocol in print format
 * @package obj.concrete_proto.print.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   $id
 */
session_start(); 

require_once ('reqnormal.inc');
require_once ("gui/o.PROTO.stepout1.inc");
require_once ("gui/o.PROTO.stepout2.inc");
require_once ("o.S_OBJLINK.subs.inc");
require_once ("f.objview.inc");

/***********************************************************/

$sql = logon2( $_SERVER['PHP_SELF'] );
$sql2 = logon2( $_SERVER['PHP_SELF'] );

$title ="protocol (concrete) print format";
$id = $_REQUEST["id"];

$protoShowObj = new oProtocolShowC();
$protoShowObj->writeJavascript();

if (empty($id)) htmlFoot("ERROR", "no protocol defined!<BR>Please select a protocol first!");

$sqls= "select ABSTRACT_PROTO_ID, notes from CONCRETE_PROTO where CONCRETE_PROTO_ID=".$id;
$sql->query("$sqls");
$sql->ReadRow();

$a_proto_id=$sql->RowData[0];
$this_notes=$sql->RowData[1];
if (!$a_proto_id) {
    htmlFoot("ERROR", "no protocol (abstract) defined for ID:$id!<BR>");
    return (-1);
}

$tablename = "CONCRETE_PROTO";
$i_tableNiceName 	 = tablename_nice2($tablename);
$infoarr=array();
$infoarr["title"] = $title;
$infoarr["form_type"]= "obj";
$infoarr["obj_name"] = $tablename;
$infoarr["obj_id"]   = $id;
$infoarr["show_name"]= 1;

$pagelib = new gHtmlHead();

$pagelib->startPage($sql, $infoarr);
 
$t_rights = tableAccessCheck( $sql, $tablename );
if ( $t_rights["write"] != 1 ) {
	tableAccessMsg( $i_tableNiceName, "write" );
	htmlFoot();
}

$o_rights = access_check( $sql, "CONCRETE_PROTO", $id );
	
if ( !$o_rights["read"] ){
	echo "<B>WARNING:</B> You do not have read permission on this object!<br>\n";
	return -1;
}


$access_data = access_data_get( $sql, "CONCRETE_PROTO", "CONCRETE_PROTO_ID", $id);
echo "<font color=gray>Creator:</font> ". $access_data['owner']. " &nbsp;&nbsp;";
echo "<font color=gray>Date:</font> ". $access_data['crea_date'];
echo "<br />";


/* END: show related/important objects */
$options=array();
$options["out_type"] = "printer";
$protoShowObj->showAll( $sql, $sql2, $id, $a_proto_id, $options );
 
htmlFoot();
