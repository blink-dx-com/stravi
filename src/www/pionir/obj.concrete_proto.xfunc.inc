<?php
require_once('gui/glob.xfunc_abs.inc');

/**
 * 
 * @author skube
 * @package obj.concrete_proto.xfunc.inc
 */
class cCONCRETE_PROTO extends xfunc_ABS {
    var $ident="CONCRETE_PROTO";
    
function __construct( ) {
    parent::__construct();
}

function inProjShowAtt(&$sqlo, $prim_id, $options=NULL ) { 
    
    $result = $this->get_abs_object_name($sqlo, $prim_id);
    return $result;
}

function inFormShowAtt(&$sql, $prim_id ) {
/* FUNCTION: show following attributes in project*/
    
	$sqls="select a.NAME from ABSTRACT_PROTO a, CONCRETE_PROTO c where c.CONCRETE_PROTO_ID=".$prim_id. " AND
	a.ABSTRACT_PROTO_ID=c.ABSTRACT_PROTO_ID";
	$sql->query($sqls);
	$sql->ReadRow();
	$tmp_optinfo="abstract: ".$sql->RowData[0];
	
	return $tmp_optinfo;
}

}


