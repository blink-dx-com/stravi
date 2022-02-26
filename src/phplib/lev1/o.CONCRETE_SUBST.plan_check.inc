<?php
/**
 * check planned PROTO parameters
 * @package o.CONCRETE_SUBST.plan_check.inc
 * @swreq REQ-0028 o.MORDER > define EXPECTED protocols STEP DETAILS
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @created 2021-10-14
 */
require_once 'o.PROTO.steps.inc';
require_once 'o.MORDER.proto_def.inc';
require_once 'o.CONCRETE_SUBST.proto.inc';
require_once 'o.MORDER.subs.inc';

class oCONCRETE_SUBST_plan_check {
    
    var $has_def_params=0;
    
    function __construct($sqlo, $id) {
        $this->id = $id;
        $this->has_def_params=0;
        
        $mord_id = oMORDER_subs::SUC_has_order($sqlo, $id);
        if (!$mord_id) return; // no order found.

        $order_proto_lib  = new oMORDER_proto_def($mord_id);
        $this->def_params_all = $order_proto_lib->proto_details_LOAD($sqlo);
        if (empty($this->def_params_all)) return;
            
        // $obj_features = glob_ObjDataGet( $sqlo, 'CONCRETE_SUBST', $this->objid, array('ABSTRACT_SUBST_ID'));
        $this->has_def_params=1;
        $suc_proto_lib = new oCONCRETE_SUBST_proto($id);
        $this->suc_log = $suc_proto_lib->get_c_protos($sqlo);   
    }
    
    /**
     * check protocol ...
     * @param object $sqlo
     * @param int $cpid
     * @return array 
     *    'err':
     *      10 : other warnings
     *       1 : parameter warning 
     *       0 : no error
            -1 - no params
            -2 - CPID not found in SUC-log
            -3 - no defined parameters configured
            
            -10 - defined parameters are empty
           'txt' : string with <br>
     */
    function check_proto($sqlo, int $cpid) {
        
        $out = array('err'=>-20, 'txt'=>'unknown');
        
        if (empty($this->def_params_all)) return array('err'=>-1, 'txt'=>'no params');
        
        $obj_features = glob_ObjDataGet( $sqlo, 'CONCRETE_PROTO', $cpid, array('ABSTRACT_PROTO_ID'));
        $cp_pra_id = $obj_features['ABSTRACT_PROTO_ID'];
        
        
        // get proto-log row
        $found=0;
        foreach($this->suc_log  as $row) {
            if ($row['cp']==$cpid) {
                $found=1;
                break;
            }
        }
        if (!$found)  return array('err'=>-2, 'txt'=>'CPID not found in SUC-log');
        
        $STEP=$row['or'];
        if (empty($this->def_params_all[$STEP])) return array('err'=>-3, 'txt'=>'no defined parameters configured');
        
        $def_pra = $this->def_params_all[$STEP]['pra'];
        if ($def_pra!=$cp_pra_id) {
            $out = array('err'=>10, 'txt'=>'Planned PRA:'.$def_pra.' is not equal to this PRA:'.$cp_pra_id);
            return $out;
        }
        $def_steps = &$this->def_params_all[$STEP]['steps'];

        if (empty($def_steps)) return array('err'=>-10, 'txt'=>'defined parameters are empty');
        
        // check if any protocol param is given, if not => no check ...
        $step_cnt_now = gProtoOrg::has_any_step($sqlo, $cpid);
        if (!$step_cnt_now) {
            return array('err'=>-12, 'txt'=>'protocol has still no data.');
        }
        
        //
        // compare DEFINED parameters with REAL parameters
        $step_cnt=0;
        $err_arr=array();
        
        foreach($def_steps as $step_nr=>$def_values) {
            
            $curr_values = gProtoOrg::conc_proto_params_RAW($sqlo, $cpid, $step_nr);
            foreach($def_values as $key=>$def_val) {
                $now_val = $curr_values[$key];
                //echo "DDDDX step_nr:$step_nr $key=>$def_val now_val:$now_val <br>";
                if ( $def_val!=$now_val ) {
                    if ($now_val===NULL or $now_val==='') {
                        $now_val_html='empty';
                    } else $now_val_html=$now_val;
                    
                    $pra_step_fea = gProtoOrg::get_pra_one_steps($sqlo, $cp_pra_id, $step_nr);
                    
                    $err_arr[]='Step '.$step_nr.': "'.$pra_step_fea['NAME'].'": '.
                        gProtoOrg::GUI_KEYs_NICE[$key].': Expected value: "'.$def_val. 
                        '" &lt;=&gt; given value: "'.$now_val_html.'"';
                }
            }
            $step_cnt++;
        }
        
        if (empty($err_arr)) {
            return array('err'=>0, 'txt'=>$step_cnt.' steps analysed.');
        }
        
        return array('err'=>1, 'txt'=>'Conflicts detected due to definitions in the '.tablename_nice2('MORDER').'<br>'. implode('<br>',$err_arr) );
    }
}