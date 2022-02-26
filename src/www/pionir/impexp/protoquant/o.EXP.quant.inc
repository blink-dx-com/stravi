<?php
require_once ('o.PROTO.quant.inc');
$tmp_file = $_SESSION['s_sessVars']['AppLabLibDir'] . '/lablib/bEXP/analyseM.inc';
if (file_exists($tmp_file)) require_once ($tmp_file);

/**
 * [protoQuant] specials for experiment
   simple protocol compare (show only quantities)
 * @package o.EXP.quant.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class oEXP_Quant extends fQuant_obj_ABS  {
    
    function __construct() {
        $this->set_table('EXP');
    }
    
    function get_columns() {
        
        $cols = array(
            
            array(
                'name'   => 'Assay_proto',
                'col_nice'=> 'Assay Protocol',
                'ty'     => 'objtrc2',
                't'      => 'CONCRETE_PROTO',            
                'fkt'    => 'EXP_HAS_PROTO',          // the table for the object tracking
                'notes'  => 'Assay Protocol'
            ),
           
            array(
                'name'    => 'Assay.End_Time',
                'col_nice'=> 'Assay.End_Time',
                'ty'      => 'objtrc2',
                'notes'   => 'json: Assay End Time'
            ),
            
            
        );
        
        return $cols;
        
    }
    
    
    
    /**
     * trace back, get object-ID
     * @param unknown $sqlo
     * @param unknown $dest_table
     * @param unknown $options
     *   'depend_id'
     */
    function get_objtrc2_data($sqlo, $name, $obj_id, $options) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;

        if ($name=='x.Assay_proto') {
            // 'objtrc_x' => array( 'pk'=>'CONCRETE_PROTO_ID', 't'=>'CONCRETE_PROTO_STEP', 'where'=>array('CONCRETE_SUBST_ID'=>'obj_id', 'ABSTRACT_PROTO_ID'=>'opt_abs_id') ), // ADVANCED object tarcking

            $cp_id   = glob_elementDataGet2( $sqlo, 'EXP_HAS_PROTO', array('EXP_ID'=>$obj_id, 'STEP_NO'=>10), 'CONCRETE_PROTO_ID' );
           
            if (!$cp_id) {
                return;
            }
            
            $pra_id    = glob_elementDataGet2( $sqlo, 'CONCRETE_PROTO', array('CONCRETE_PROTO_ID'=>$cp_id), 'ABSTRACT_PROTO_ID' );
            $pra_name  = glob_elementDataGet2( $sqlo, 'ABSTRACT_PROTO', array('ABSTRACT_PROTO_ID'=>$pra_id), 'NAME' );
            
            
            return $pra_name;
        }
        
        if ($name=='x.Assay.End_Time') {
            
            // read json file
            $exp_lib = new \bEXP\analyseM();
            try {
                $exp_lib->set_exp($sqlo, $obj_id);
                $exp_lib->check_data_path();
                $exp_dict = $exp_lib->get_json_dict($sqlo);
   
            } catch (Exception $e) {
                $error->set( $FUNCNAME, 1, $e->getMessage() );
            }
            
            if ($error->Got(READONLY))  {
                $errLast   = $error->getLast();
                $errdet    = $errLast->text;
               
                $output = 'ERR: No JSON-dict found. '.$errdet;
                $error->reset();
                return $output;
            }
            
            $output = $exp_dict['run']['finish'];
            return $output;
        }
    }
    
    /**
     * trace back, get object-ID
     * @param unknown $sqlo
     * @param unknown $dest_table
     * @param unknown $options
     *   'depend_id'
     */
    function get_objtrc2_obj($sqlo, $name, $dest_table, $mo_obj_id, $options) {
        

        if ($name=='EXP_ID') {
            // 'objtrc_x' => array( 'pk'=>'CONCRETE_PROTO_ID', 't'=>'CONCRETE_PROTO_STEP', 'where'=>array('CONCRETE_SUBST_ID'=>'obj_id', 'ABSTRACT_PROTO_ID'=>'opt_abs_id') ), // ADVANCED object tarcking

            
            $apid   = $options['depend_id'];
            if (!$apid) {
                return;
            }
            $cp_id  = glob_elementDataGet2( $sqlo, 'CONCRETE_PROTO_STEP', array('CONCRETE_SUBST_ID'=>$mo_obj_id, 'ABSTRACT_PROTO_ID'=>$apid), 'CONCRETE_PROTO_ID' );
            if (!$cp_id) {
                return;
            }
            $obj_id   = glob_elementDataGet2( $sqlo, 'EXP_HAS_PROTO', array('CONCRETE_PROTO_ID'=>$cp_id), 'EXP_ID' );

            return $obj_id;
        }
        
      
    }
    
    
    function getAbsProtos( $sql ) {
        
        $sqls= "select e.STEP_NO, c.ABSTRACT_PROTO_ID from EXP_HAS_PROTO e, CONCRETE_PROTO c ".
            " where e.EXP_ID in (select x.EXP_ID from ".$this->sqlAfterNoOrd.")".
            " AND e.CONCRETE_PROTO_ID=c.CONCRETE_PROTO_ID ORDER by STEP_NO";
        $a_proto_arr=array();
        $sql->query($sqls);
        while ( $sql->ReadRow() ) {
            $a_proto_arr[] = $sql->RowData[1];
        }
        $this->tmp_abstractProtos = $a_proto_arr;
        return $a_proto_arr;
    }

    
    function doLoop( &$sql, &$sql2 ) {
        
        $showAll = 1;

        $sqls = "SELECT x.EXP_ID, x.NAME FROM ".$this->sqlAfter;
        $sql2->query($sqls);
        while ( $sql2->ReadRow() ) {
            
            $expid = $sql2->RowData[0];

            $this->protoquant_lib->show_one_row($sql, $expid, $showAll);
           
            
        }
        
    }
    
}

