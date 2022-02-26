<?php
require_once 'o.CONCRETE_PROTO.mod.inc';
require_once('insert.inc');
require_once('date_funcs.inc');
require_once ("f.assocUpdate.inc");
require_once 'g_delete.inc';

/**
 * experiment protocol mod functions
 * @package o.EXP.proto_mod.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class oEXP_proto_mod {
    
    function __construct() {
        $this->sqlDateNow = date_unix2sql( time() );
    }
    
    function set_exp(&$sqlo, $exp_id) {
        global $error;
        // $FUNCNAME= __CLASS__.':'.__FUNCTION__;
  
        $this->exp_id = $exp_id;
        
        $this->assoclib = new  fAssocUpdate();
        $this->assoclib->setObj( $sqlo, "EXP_HAS_PROTO", $exp_id );
        if ($error->Got(READONLY))  {
            return;
        }
    }
    
    // check if step exists
    function step_exists(object $sqlo, int $step_no) {
        $exists=0;
        $sqlsel = "STEP_NO from EXP_HAS_PROTO where EXP_ID=".$this->exp_id." AND STEP_NO=".$step_no;
        $sqlo->Quesel($sqlsel);
        if ( $sqlo->ReadRow() ) {
            $exists=1;
        }
        return $exists;
    }
    
    // get CPID of STEP_NO
    function get_cpid_of_step(object $sqlo, int $step_no) {
        $cpid = 0;
        $sqlsel = "CONCRETE_PROTO_ID from EXP_HAS_PROTO where EXP_ID=".$this->exp_id." AND STEP_NO=".$step_no;
        $sqlo->Quesel($sqlsel);
        if ( $sqlo->ReadRow() ) {
            $cpid=$sqlo->RowData[0];
        }
        return $cpid;
    }

    function add_existing($sqlo, $cpid, $step_no) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        // check exists ?
        if ( $this->step_exists($sqlo, $step_no) ) {
            $error->set( $FUNCNAME, 3, "Protocol on STEP_NO $step_no already exists.");
            return;
        }
        
        $argu=array();
        $argu['STEP_NO']	       = $step_no;
        $argu['CONCRETE_PROTO_ID'] = $cpid;
        $this->assoclib->insert( $sqlo, $argu );
    }
    
    /**
     * create protocol, add a protocol to experiment
     * @param object $sql
     * @param object $sql2 --- only needed if $blue_conc_prot is set
     * @param int $abstract_proto_id
     * @param int $step_no
     * @param array $argu CONCRETE_PROTO argus
     * @param int $blue_conc_prot
     * @param array $opt
     *   "checkStepNo" 0,1 check, if step-no exists => error
     * @return int $c_proto
     */
    function create( &$sql, &$sql2, $abstract_proto_id, $step_no, $cargu=array(), $blue_conc_prot = 0,  $opt=NULL  ) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $id = $this->exp_id;

        if ($opt["checkStepNo"]>0 ) {
            if ( $this->step_exists($sql, $step_no) ) {
                $error->set( $FUNCNAME, 3, "Protocol on STEP_NO $step_no already exists.");
                return;
            }
        }
        if ($blue_conc_prot and !is_object($sql2) ) {
            throw new Exception('sql2 handle missing.');
        }
        
        
        $cargu['ABSTRACT_PROTO_ID'] = $abstract_proto_id;
        $c_proto = insert_row( $sql, "CONCRETE_PROTO", $cargu );
        if ($c_proto<=0) {
            $error->set( $FUNCNAME, 1, "Problems at creation of protocol.");
            return;
        }
        $argu=array();
        $argu['STEP_NO']	       = $step_no;
        $argu['CONCRETE_PROTO_ID'] = $c_proto;
        $ret_id = $this->assoclib->insert( $sql, $argu );
        if ($ret_id<=0) {
            $error->set( $FUNCNAME, 2, "Problems at connection of protocol to experiment.".
                " Params: exp_id: $id step_no:$step_no c_proto_id:$c_proto");
            return;
        }
        
        if ( $blue_conc_prot ) {
            $cropt=array( 'exec_date'=>$this->sqlDateNow);
            oCONCRETE_PROTO_mod::assoc_copy( $sql, $sql2, $blue_conc_prot, $c_proto, $cropt );
        }
        return $c_proto;
    } 
    
    /**
     * unlink and delete protocol from experiment
     */
    function del_prot(&$sql, $concrete_proto_id) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $idarr = array( "CONCRETE_PROTO_ID"=>$concrete_proto_id) ;
        $this->assoclib->delOneRow($sql, $idarr);
        if ($error->Got(READONLY))  {
            $error->set( $FUNCNAME, 1, "Unlink of protocol and experiment failed." );
            return;
        }
        
        $o_rights = access_check( $sql, "CONCRETE_PROTO", $concrete_proto_id );
        if (!$o_rights["delete"]) {
            $error->set( $FUNCNAME, 1, "not o_right to delete the protocol." );
            return;
        } else {
            $dellib = new fObjDelC();
            $retval = $dellib->obj_delete ( $sql, "CONCRETE_PROTO", $concrete_proto_id, NULL);
            if ( $retval<0 ) {
                $error->set( $FUNCNAME, 1, "Problem at delete of the protocol" );
                return;
            }
        }
    }
    
    /**
     * unlink protocol from experiment
     */
    function unlink_prot(&$sqlo, int $step_no) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $concrete_proto_id = glob_elementDataGet2($sqlo, "EXP_HAS_PROTO", array('EXP_ID'=>$this->exp_id, 'STEP_NO'=>$step_no), 'CONCRETE_PROTO_ID');
        if (!$concrete_proto_id) {
            // no protocol found, this is not an error ...
            return;
        }
        
        $idarr = array( "CONCRETE_PROTO_ID"=>$concrete_proto_id) ;
        $this->assoclib->delOneRow($sqlo, $idarr);
        if ($error->Got(READONLY))  {
            $error->set( $FUNCNAME, 2, "Unlink of protocol and experiment failed." );
            return;
        }
        
        
    }

}