<?php

require_once ('table_access.inc');
require_once ('insert.inc');
/**
 * @DEPRECATED
 * support extra creation
 * @package o.EXP.crea.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class oEXPcreaC {

function getArguByTmpl( &$sql, $exp_tmpl_id ) {
	// FUNCTION: get exp_tmpl arguments for experiment
	global $error;
	$FUNCNAME = "getArguByTmpl";
	
	$error = & ErrorHandler::get();
	
	// create exp
    $stmt =  "SELECT * FROM exp_tmpl WHERE exp_tmpl_id = ".$exp_tmpl_id;
	$tmpl_data = NULL;
    $sql->Query($stmt);
    if ($sql->ReadArray()) {
        $tmpl_data = $sql->RowData;
	} else {
		$error->set($FUNCNAME, 1, "Experiment template $exp_tmpl_id not found.");
        return;
	}
   
    // in this function unchecked: abstract array of concrete array equal to
    // abstract array of experiment templates

    // copy data
	
	$copyArr = array( 
		"EXP_TMPL_ID", "H_KIND_OF_INTERACT_ID", "H_BASE_OF_PROOF_ID", "H_EXP_RAW_DESC_ID"
		 );
	$argu=NULL;
    foreach( $copyArr as $dummy=>$key) {
		 $argu[$key] = $tmpl_data[$key];
	}
	reset ($copyArr); 
		 
    return ($argu);
}


}