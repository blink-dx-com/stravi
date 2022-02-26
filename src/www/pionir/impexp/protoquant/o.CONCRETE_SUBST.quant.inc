<?php
require_once ('o.CS_HAS_PR.subs.inc');
require_once ('o.PROTO.quant.inc');
require_once 'o.AS_HAS_PR.subs.inc';

/**
 * supporting class for ProtoQuant
 * @package o.CONCRETE_SUBST.quant.inc
 * @swreq UREQ:0001578: o.PROTO > ProtoQuant : Analyse protocol steps 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 *
 */
class oCONCRETE_SUBST_Quant extends fQuant_obj_ABS {

	function __construct() {
	    $this->set_table('CONCRETE_SUBST');
		$this->substProtoLib = new oCS_HAS_PR_subs();
	}

	function get_columns() {
	    
	    $cols = array(

	        array(
	            'name'  => 'EXP_ID',
	            'col_nice'=> 'Related Experiment',
	            'nice'   => 1,
	            'dty'    => 'objtrc2',        // object tracking extreme
	            'ty'     => 'objtrc2',        // object tracking extreme
	            'sh_col' => -1,       // do NOT show in this columns ...
	            't'      => 'EXP',       // the destination table
	            'fkt'    => 'CONCRETE_SUBST', // the table for the object tracking
	            'notes'  => 'Used in Cart-Batch',
                
	        ),
	        
	       
	    );
	    
	    return $cols;
	    
	}
	

	


	function getAbsProtos( $sql ) {
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    $sqls= "select c.ABSTRACT_PROTO_ID from CS_HAS_PR e, CONCRETE_PROTO c ".
	   	    " where e.CONCRETE_SUBST_ID in (select x.CONCRETE_SUBST_ID from ".$this->sqlAfterNoOrd.")".
	   	    " AND e.CONCRETE_PROTO_ID=c.CONCRETE_PROTO_ID ORDER by STEP_NO";
	    $a_proto_arr=array();
	    $sql->query($sqls);
	    while ( $sql->ReadRow() ) {
	        $a_proto_arr[] = $sql->RowData[0];
	    }
	    
	    debugOut('a_proto_arr: '.print_r($a_proto_arr,1), $FUNCNAME, 2);
	    
	    $this->tmp_abstractProtos = $a_proto_arr;
	    return $a_proto_arr;
	}
	
	function get_apids_of_abs( &$sqlo, $sua_id ) {
	    
	    $sua_pra_lib = new oAS_HAS_PR_subs($sua_id);
	    $protoLog = $sua_pra_lib->getProtoLog($sqlo);
	    
	    $this->tmp_abstractProtos = array();
	    
	    // get abstract protocol IDs
	    if (!is_array($protoLog)) {
	        return;
	    }
	   
	    $a_proto_arr = array();
	    foreach( $protoLog as $key=>$val ) {
	        $a_proto_arr[]=$val['ap'];
	    }
	    
	    $this->tmp_abstractProtos = $a_proto_arr;
	    
	    return $a_proto_arr;
	}

	function doLoop( &$sqlo, &$sqlo2) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		/*
		$root_struct  = $this->get_sub_struct('0:0');
		$this->x_apid = $root_struct['apid'];
		*/
		
		/*
		$sqlsel = 'count(1) from '.$this->sqlAfterNoOrd;
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$cnt_obj = $sqlo->RowData[0];
		if ($cnt_obj > $this->MAX_OBJ_ANA) {
			$error->set( $FUNCNAME, 1, 'Too many objects selected ('.$cnt_obj.'). Max '. $this->MAX_OBJ_ANA.' allowed.' );
			return;
		}
		
		*/
		
		

		$showAll = 1;
		$sqlsel = "CONCRETE_SUBST_ID from ". $this->sqlAfter;
		$sqlo2->Quesel($sqlsel);
		while ( $sqlo2->ReadRow() ) {

		    /*
			$loop_substid = $sqlo2->RowData[0];
			$loop_charge  = $sqlo2->RowData[1];
			$extraCols=array();
			$extraCols[] = $loop_substid;
			$extraCols[] = $loop_charge;

			$this->show_one_row($sqlo, $extraCols, $loop_substid, $showAll);
			*/
			
			$suc_id = $sqlo2->RowData[0];
			
			$this->protoquant_lib->show_one_row($sqlo, $suc_id, $showAll);
		}
	}

}


