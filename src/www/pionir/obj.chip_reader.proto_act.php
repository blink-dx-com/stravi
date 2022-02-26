<?php
/**
 * create, delete concrete_proto for table "CHIP_READER" 
 
 * @package obj.chip_reader.proto_act.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param $obj_id : ID of object
  		  $t      : tablename : "CHIP_READER",
  		               DEPRECATED:"CONCRETE_SUBST"
  	      $xact   : "new", "delete", "new_abstract"
		  $go ( for "delete" )
 * @errors -4: no abstract_protocol
  	       -5: delete failed
 */

session_start(); 


require_once ('reqnormal.inc');
require_once ('g_delete.inc');
require_once ('insert.inc');
require_once ("func_formSp.inc");
require_once ("date_funcs.inc");
require_once ("insertx.inc");
require_once 'lev1/glob.obj.create_subs.inc';
require_once ("glob.obj.update.inc");

$FUNCNAME='MAIN';
$obj_id = $_REQUEST['obj_id'];
$t      = $_REQUEST['t'];
$xact   = $_REQUEST['xact'];
$go     = $_REQUEST['go'];

$table 	= $t;
$pk_name   = $table ."_ID";

if ($table == "CHIP_READER") $absTable  = "A_CHIP_READER";
$a_colname = $absTable."_ID";
$tablenice = tablename_nice2($table);


global $error;

$error = & ErrorHandler::get();
$sql  = logon2( $_SERVER['PHP_SELF'] );
//$sql2 = logon2( );
if ($error->printLast()) htmlFoot();

$title   = $xact." protocol";
$infoarr = NULL; 
$infoarr["title"] = $title;
$infoarr["form_type"]= "obj";
$infoarr["obj_name"] = $table;
$infoarr["obj_id"]   = $obj_id;


$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);
echo '<ul><br>';

if ($table!='CHIP_READER') {
    htmlFoot( "ERROR","This table is not allowed for this tool.");
}

echo "action: <b>".$xact."</B><br>";

$o_rights = access_check( $sql, $table, $obj_id );
	
if ( !$o_rights["write"] ){
	htmlFoot( "WARNING","You do not have write permission on this substance!");
}

$sqls_main="select ".$pk_name.", CONCRETE_PROTO_ID, ".$a_colname." from $table where ".$pk_name."=".$obj_id;
$sql->query("$sqls_main");
$sql->ReadRow();
$c_proto    = $sql->RowData[1]; 
$abs_obj_id = $sql->RowData[2];

if ($xact=="new_abstract") {
	/* BLUE-print: $table -> ".$absTable." -> abstract_proto */
	
	$sqls="select ABSTRACT_PROTO_ID from ".$absTable." where ".$a_colname."=".$abs_obj_id;
	$sql->query("$sqls");
	$sql->ReadRow();
	$a_proto_id    = $sql->RowData[0];
	if ( !$a_proto_id ) {
		htmlFoot( "ERROR",  "creation of protocol failed: substance (abstract) has no protocol (abstract). [-4]");
	}
	
	$execdate = date_unix2datestr( time(), 1 ); // in seconds ...
	$argu=array();
	$argu["ABSTRACT_PROTO_ID"]=$a_proto_id;
	$argu["EXEC_DATE"]  = $execdate; // today
	$insopt = array('types'=>array('EXEC_DATE'=>'DATE1'));
	
	$args = array('vals'=>$argu);
	$inslib = new insertC();
	$c_proto_new = $inslib->new_meta($sql, "CONCRETE_PROTO", $args, $insopt);
	//$c_proto_new = insert_row( $sql, "CONCRETE_PROTO", $argu, $insopt );
	
	if ($c_proto_new<=0) {
		htmlFoot( "ERROR",  "creation of protocol failed during insert process. [-5]");
	} 
	
	$workflowLib = new gObjCreaSubs();
	$workflowLib->addUserWorkflow($sql, "CONCRETE_PROTO", $c_proto_new, 0, $a_proto_id);
	if ($error->Got(READONLY))  {
		$pagelib->chkErrStop();
		return;
	}
	
	$args=array(
	    'vals'=>array(
	        'CONCRETE_PROTO_ID'=>$c_proto_new
	    )
	);
	$UpdateLib = new globObjUpdate();
	$UpdateLib->update_meta( $sql, $table, $obj_id, $args );

}

if ($xact=="new") {

	/* BLUE-print: $table -> ".$absTable." -> abstract_proto */
	/* Copy from : $table -> concrete_proto */
}

if ( ($xact=="delete") AND $c_proto) {
	
	if (!$go) {
		
		$formLib = new formSpecialc();
		$params = array( "obj_id"=>$obj_id, "t"=>$table, "xact"=>"delete", "go"=>1 );
		$formLib->deleteForm( "Delete Protocol", "Do you want to delete the protocol ($c_proto) "
				, $_SERVER['PHP_SELF'], $params );
		htmlFoot("<hr>");
		
	} 
	

	$args=array(
	    'vals'=>array(
	        'CONCRETE_PROTO_ID'=>NULL
	    )
	);
	$UpdateLib = new globObjUpdate();
	$UpdateLib->update_meta( $sql, $table, $obj_id, $args );
	if ($error->Got(READONLY))  {
	    $error->set( $FUNCNAME, 1, 'Error on update.' );
	    return;
	}
	
	$dellib = new fObjDelC();
	$tmpretval = $dellib->obj_delete ( $sql, "CONCRETE_PROTO", $c_proto );
	if ($tmpretval<0) {
		htmlFoot( "ERROR", "during delete of protocol (error-code: $tmpretval).\n");
	}
	
}

if ( $error->printAll() ) {
	htmlFoot();
} else {
	$nexturl="edit.tmpl.php?t=".$table."&id=".$obj_id;
	js__location_replace( $nexturl, $tablenice );
}

echo "</ul>";
htmlFoot('<hr>');
