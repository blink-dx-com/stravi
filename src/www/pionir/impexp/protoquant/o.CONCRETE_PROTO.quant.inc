<?php
require_once ('o.CS_HAS_PR.subs.inc');
require_once ('o.PROTO.quant.inc');
require_once 'o.AS_HAS_PR.subs.inc';

/**
 * supporting class for  plugin o.CONCRETE_PROTO.pq.inc
 * @package o.CONCRETE_SUBST.quant.inc
 * @swreq UREQ:0001578: o.PROTO > ProtoQuant : Analyse protocol steps 
 * @author  Steffen Kube 
 *
 */
class oCONCRETE_PROTO_Quant extends fQuant_obj_ABS {

	function __construct() {
	    $this->set_table('CONCRETE_PROTO');
		// $this->substProtoLib = new oCS_HAS_PR_subs();
	}

	function get_columns() {
	    
	    $cols=array();
// 	    $cols = array(

// 	        array(
// 	            'name'  => 'EXP_ID',
// 	            'col_nice'=> 'Related Experiment',
// 	            'nice'   => 1,
// 	            'dty'    => 'objtrc2',        // object tracking extreme
// 	            'ty'     => 'objtrc2',        // object tracking extreme
// 	            'sh_col' => -1,       // do NOT show in this columns ...
// 	            't'      => 'EXP',       // the destination table
// 	            'fkt'    => 'CONCRETE_SUBST', // the table for the object tracking
// 	            'notes'  => 'Used in Cart-Batch',
                
// 	        ),
	        
	       
// 	    );
	    
	    return $cols;
	    
	}

	function getAbsProtos( $sql ) {
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    $sqls= "select c.ABSTRACT_PROTO_ID from CONCRETE_PROTO c ".
	   	    " where c.CONCRETE_PROTO_ID in (select x.CONCRETE_PROTO_ID from ".$this->sqlAfterNoOrd.")".
	   	    "  ORDER by c.ABSTRACT_PROTO_ID";
	    $a_proto_arr=array();
	    $sql->query($sqls);
	    while ( $sql->ReadRow() ) {
	        $a_proto_arr[] = $sql->RowData[0];
	    }
	    
	    debugOut('a_proto_arr: '.print_r($a_proto_arr,1), $FUNCNAME, 2);
	    
	    $this->tmp_abstractProtos = $a_proto_arr;
	    return $a_proto_arr;
	}


	function doLoop( &$sqlo, &$sqlo2) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;

		$showAll = 1;
		$sqlsel = "CONCRETE_PROTO_ID from ". $this->sqlAfter;
		$sqlo2->Quesel($sqlsel);
		while ( $sqlo2->ReadRow() ) {
			$prc_id = $sqlo2->RowData[0];
			$this->protoquant_lib->show_one_row($sqlo, $prc_id, $showAll);
		}
	}

}


