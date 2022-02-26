<?php


require_once ("f.clipboard.inc");
require_once ("insert.inc");
require_once ("f.assocUpdate.inc");
require_once ('f.update.inc');

/**
 * paste proto steps from CLIPBOARD
 * @package obj.abstract_proto.CLIPBOARD.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   string INPUT
 */
class oAbsProtoStepPaste {
	
    function __construct( $protoid ) {
    	$this->protoid = $protoid;
    }
    
    function _initUpdate( &$sqlo ) {
        
        $this->pra_up_lib = new oAbsProtoStepMod();
        $this->pra_up_lib->init($sqlo, $this->protoid);
    	
    }
    
    private function single_step_copy( &$sql, $proto_src, $step_nr,  $step_nr_dest, $main_step_new ) {
    	 global $error;
    	 $FUNCNAME= __CLASS__.':'.__FUNCTION__;
 
    	$sqls= "select * from ABSTRACT_PROTO_STEP where
    		ABSTRACT_PROTO_ID=" .$proto_src. " AND STEP_NR=" . $step_nr;
    	$sql->query($sqls);
    	
    	if ( $sql->ReadArray() ) {

    		$argu= $sql->RowData;
    		$argu["MAIN_STEP_NR"]= $main_step_new;
    		unset($argu['ABSTRACT_PROTO_ID']);
    		
    		$this->pra_up_lib->insert_step( $sql, $step_nr_dest, $argu );
	
    	} else {
    		
    		$error->set( $FUNCNAME, 2, 'No source step found.' );
    		return;
    		
    	}
    	
    }
    
    private function _absProtoCpy( &$sql, &$sql2, $clip_prim_id) {
    	
    	global $error;
    	$FUNCNAME= "_absProtoCpy";
    
    	$id = $this->protoid;
    	
    	$tablename = "ABSTRACT_PROTO";
    	
    	if ( $clip_prim_id <= 0 ) { 
    		$error->set( $FUNCNAME, 2, "ERROR: no protocol (abstract) in clipboard!" );
    		return;
    	} 
    	
    	$sqls= "select count(*) from ABSTRACT_PROTO_STEP where ABSTRACT_PROTO_ID=".$id;
    	$sql->query($sqls);
    	$sql->ReadRow();
    	$tmp_num_steps=$sql->RowData[0];
    	if ( $tmp_num_steps > 0 ) {
    		$error->set( $FUNCNAME, 3, "$tmp_num_steps steps allways exist, please delete them before paste.");
    		return;
    	}
    	
    	/* copy H_PROTO_KIND_ID */
    	$sqls= "select H_PROTO_KIND_ID from ABSTRACT_PROTO where ".
    			" ABSTRACT_PROTO_ID=".$clip_prim_id;
    	$sql->query($sqls);
    	$sql->ReadRow();
    	$argu=array();
    	$argu["ABSTRACT_PROTO_ID"]=$id;
    	$argu["H_PROTO_KIND_ID"]  =$sql->RowData[0];
    	$retVal = gObjUpdate::update_row( $sql, $tablename, $argu );
    	echo "copy H_PROTO_KIND_ID : return val: $retVal<br>";
    	
    	/* copy single steps */
    	
    	$sqls= "select STEP_NR, MAIN_STEP_NR from ABSTRACT_PROTO_STEP where
    		ABSTRACT_PROTO_ID=".$clip_prim_id. " ORDER by STEP_NR";
    	$sql->query($sqls);
    	while ( $sql->ReadRow() && ($retVal>=0) ) {
    	
    		$step_nr = $sql->RowData[0];
    		$main_step_nr = $sql->RowData[1];
    		$retVal = $this->single_step_copy ( $sql2, $clip_prim_id, $step_nr, $step_nr, $main_step_nr );
    		if ($error->Got(READONLY))  {
    		    $error->set( $FUNCNAME, 1, 'Error on step:'. $step_nr);
    		    return;
    		}
    	}
    }
    
    private function _absStepCpy( &$sql, &$sql2, $start_pos_behind ) {
    	/* get the highest MAIN_STEP_NR and STEP_NR */
    	
    	global $error;
    	$FUNCNAME= "_absStepCpy";
    
    	$id = $this->protoid;
    	
    	$sqls= "select max(step_nr), max(main_step_nr) from ABSTRACT_PROTO_STEP where ABSTRACT_PROTO_ID=".$id;
    	$sql->query("$sqls");
    	$sql->ReadRow();
    	$step_nr_ori_max 	= $sql->RowData[0];
    	$main_step_nr_ori_max 	= $sql->RowData[1];
    	
    	$step_nr_new 	  = $step_nr_ori_max +1;
    	$main_step_nr_new = $main_step_nr_ori_max +1;
    	
    	$clipobj = new clipboardC();
    	$clipobj->reset();
    	
    	$new_step_arr=array();
    	
    	while ( $elemArr = $clipobj->getcurrElem()  ) {
    
    		$tmp_tabname	= $elemArr[0];
    		$clip_prim_id 	= $elemArr[1];
    		$step_nr 		= $elemArr[2];
    		
    		echo "clip-elem: $tmp_tabname, $clip_prim_id, $step_nr  --" ;
    		
    		if ( ($tmp_tabname == "ABSTRACT_PROTO_STEP" ) && ($clip_prim_id>0) && ($step_nr>0) ) { 
    	
    			$this->single_step_copy ( $sql2, $clip_prim_id, $step_nr, $step_nr_new, $main_step_nr_new );
    			if ($error->Got(READONLY))  {
    			    $error->set( $FUNCNAME, 3, "step-copy-error: step_nr:$step_nr NEW:$step_nr_new");
    				break;
    			}
    			
    			$new_step_arr[] = $step_nr_new;
    			
    			$step_nr_new++;
    			$main_step_nr_new++;
    		}
    		echo "<br>\n";
    	}
    	
    	$this->pra_up_lib->move_steps($sql, $new_step_arr, $start_pos_behind);
    	
    	$clipobj->reset();
    }
    
    function doPaste( &$sql, &$sql2, $start_pos_behind=-1 ) {
    	// paste steps
    	
    	global $error;
    	$FUNCNAME= "doPaste";
    
    	// $id = $this->protoid;
    	
    	echo "Paste protocol-steps from clipboard<br>";
    	if ( !sizeof($_SESSION['s_clipboard']) ) {
    		$error->set( $FUNCNAME, 1, "no element in clipboard." );
    		return;
    	}
    	
    	$clipobj = new clipboardC();
    	$clipobj->reset();
    	$elemArr = $clipobj->getcurrElem();
    	$tmp_tabname = $elemArr[0];
    	
    	if ( $tmp_tabname != "ABSTRACT_PROTO" AND  $tmp_tabname != "ABSTRACT_PROTO_STEP") {
    		$error->set( $FUNCNAME, 2, "no ABSTRACT_PROTO and ABSTRACT_PROTO_STEP in clipboard." );
    		return;
    	}
    	
    	$this->_initUpdate( $sql );
    	
    	if ( $tmp_tabname == "ABSTRACT_PROTO_STEP" ) {
    	    $this->_absStepCpy( $sql, $sql2, $start_pos_behind );
    	    return;
    	}
    		
    	if ( $tmp_tabname == "ABSTRACT_PROTO" ) {
    		$clip_prim_id = $elemArr[1];
    		$this->_absProtoCpy( $sql, $sql2, $clip_prim_id);
    		return;
    	}
    		
    	
    }

}