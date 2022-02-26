<?php
require_once 'o.ABSTRACT_PROTO.step_KI.inc';

/**
 * ABSTRACT_SUBST  KI:  all ( ABSTRACT_PROTO by STEP_NR + NAME )
 * @package o.ABSTRACT_SUBST.proto_step_KI.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 *
 */
class o_ABSTRACT_SUBST_proto_step_KI_1 {
    
    function __construct() {
        $this->all_step_arr = array();
    }
    
    // add one step array of a complete protocol
    function add_step_arr($step_arr) {
        $this->all_step_arr[]=$step_arr;
    }
    
    /**
     * find SUAs by protocol steps, multiple protocols possible
     * INPUT: $this->all_step_arr
     * @param object $sqlo
     * @param array $options
     * @return array
     *   'SUA'  => array(
     *      'ids'   => ids of ABSTRACT_SUBST, 
            'status'=> text
     *   )
     *   'PRAs' => array() -- one entry per input $step_arr
             array( -- one protocol
                'ids'   => ids of ABSTRACT_PROTO, 
                'status'=> text
             )
     */
    function analyse($sqlo, $options=array() ) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $MAX_SUAS = 100;
        
        $output=array(
            'SUA'  => array(),
            'PRAs' => array(),
            'status'=>''
        );
        
        if(empty($this->all_step_arr)) {
            throw new Exception('Input all_step_arr missing.');
        }

        
        $pra_KI_lib = new o_ABSTRACT_PROTO_step_KI_1();

        $pra_result=array();
        foreach($this->all_step_arr as $step_arr) {
            $pra_KI_lib->set_steps($step_arr);
            $KI_result = $pra_KI_lib->analyse($sqlo); // array('ids'=>$pra_final_arr, 'status'=>$status_txt);
            $pra_result[]  = $KI_result;
        }
        $output['PRAs'] = $pra_result;
        
        if ( $_SESSION["userGlob"]["g.debugLevel"]>1 ) {
            glob_printr( $pra_result, $FUNCNAME." pra_result:", 1);
        }
        
        $all_found = 1;
        $found_pras = array();
        foreach($pra_result as $i=>$row) {
            
            $pra_id =0;
            $pra_found = 0;
            if (empty($row['ids'])) {
                // not foundcontinue;
            } else {
                if(sizeof($row['ids'])==1) {
                    $pra_found = 1;
                    $pra_ids = array_keys($row['ids']); 
                }
                if (sizeof($row['ids'])>1) {
                    $pra_found = 2;
                    $pra_ids = array_keys($row['ids']); 
                }
            }
            
            //echo "DDD: $i: pra_found:$pra_found pra_id:$pra_id size:".sizeof($row['ids'])."<br>";
            
            if (!$pra_found) {
                $all_found = 0;
            }
            if ($pra_found>0) {
                $found_pras = array_merge($found_pras, $pra_ids); // can be more than ONE PRA per input-steps !!!
            } 
            if ($pra_found>1) {
                $all_found = 2;
            }
            
        }
        
        if (empty($found_pras)) {
            $output['status']='not any PRA found.';
            return $output;
        }
        
        if (!$all_found) {
            $output['status']='not all PRAs found.';
            return $output;
        }
        
        
        
        // find SUA by PR arrays
        $suas = array();
        $pra_str = implode(',',$found_pras);
        $pra_cnt_expect = sizeof($this->all_step_arr);
        $sqlsel= "max(ABSTRACT_SUBST_ID) from AS_HAS_PR where ABSTRACT_PROTO_ID in (".$pra_str.") ".
            "group by ABSTRACT_SUBST_ID having COUNT(ABSTRACT_PROTO_ID)>=".$pra_cnt_expect;
        
        $sqlo->Quesel($sqlsel);
        
        $cnt=0;
        while ($sqlo->ReadRow() ) {
            if ($cnt>$MAX_SUAS) {
                break;
            }
            $sua_id = $sqlo->RowData[0];
            $suas[]   = $sua_id;
            $cnt++;
        }
        
        if ( $_SESSION["userGlob"]["g.debugLevel"]>2 ) {
            glob_printr( $suas, $FUNCNAME." suas:", 1);
        }
        
        $output['SUA']['ids'] = $suas;
        
        if ( $_SESSION["userGlob"]["g.debugLevel"]>1 ) {
            glob_printr( $output, $FUNCNAME." output:", 1);
        }
        
        return $output;
    }
    
    /**
     * analyse PRAs, find best matches of SUAs
     * - the tool finds even SUAs with parts of the protocol log
     * @param object $sqlo
     * @param array  $output[] : 
     *     array('sua'=>, 'pra_cnt'=>);
     */
    function analyse_pra_log_parts($sqlo, $pra_arr ) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $MAX_SUAS = 50;
        $pra_str = implode(',',$pra_arr);
       
        $sqlsel= "max(ABSTRACT_SUBST_ID), COUNT(ABSTRACT_PROTO_ID) from AS_HAS_PR where ABSTRACT_PROTO_ID in (".$pra_str.") ".
            "group by ABSTRACT_SUBST_ID order by COUNT(ABSTRACT_PROTO_ID) DESC";
        
        $sqlo->Quesel($sqlsel);
        
        $output = array();
        $cnt=0;
        while ($sqlo->ReadRow() ) {
            if ($cnt>$MAX_SUAS) {
                break;
            }
            $sua_id  = $sqlo->RowData[0];
            $pra_cnt = $sqlo->RowData[1];
            $output[]  = array('sua'=>$sua_id, 'pra_cnt'=>$pra_cnt);
            $cnt++;
        }
        
        if ( $_SESSION["userGlob"]["g.debugLevel"]>2 ) {
            glob_printr( $output, $FUNCNAME." output:", 1);
        }
        
        
        return $output;
    }
        
}
