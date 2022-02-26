<?php 
require_once ('f.msgboxes.inc');
require_once 'o.CONCRETE_SUBST.proto.inc';
require_once 'o.SOCKET.subs.inc';
require_once 'gui/o.CONCRETE_PROTO.li_show1.inc';

/**
 * example of module
 * @package gui/o.SOCKET.mr_prot_show.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class o_SOCKET_mr_prot_show {
    
    function __construct($socket_id) {
        $this->socket_id = $socket_id;
    }
    
    function show($sqlo, $tool_opts=array()) {
        //global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $tablename_s='SOCKET';
        $socket_id=$this->socket_id;
        
        if(!is_array($tool_opts)) $tool_opts=array();
        
        $sock_lib = new oSOCKET_subs();
        $sock_lib->init($sqlo, $socket_id );
        $sock_sucs = $sock_lib->get_cart_content_arr_simple($sqlo);
        
        if(empty($sock_sucs)) {
            cMsgbox::showBox("info", 'Has no components.');
            return;
        }
        
        $first_row = current($sock_sucs);
        $suc_id    = $first_row['suc'];
        if (!$suc_id) {
            cMsgbox::showBox("warning", 'First '.tablename_nice2($tablename_s).'-component has no MAC info!');  
            return;
        }
        $suc_prc_lib = new oCONCRETE_SUBST_proto($suc_id);
        $prc_log = $suc_prc_lib->get_c_protos($sqlo);
        if (empty($prc_log)) {
            cMsgbox::showBox("warning", 'First '.tablename_nice2($tablename_s).'-component (MAC:'.$suc_id.') has no protocol!');
            return;
        }
        
        
        /**
         * 
         * @var array $all_proto_arr
         *  [i] = proto_array (for each object
         *      where i is the INDEX of the PROTOCOL-log 
         */
        
        $step_or_arr=array();
        
        $all_proto_arr   = array();
        $suc_inp_arr = array();
        foreach($sock_sucs as $obj_col_index => $row) {
            
            $suc_id = $row['suc'];
            $suc_inp_arr[] = $suc_id;
            
            if ($suc_id) {
                $suc_prc_lib = new oCONCRETE_SUBST_proto($suc_id);
                $prc_log      = $suc_prc_lib->get_c_protos($sqlo);
                foreach($prc_log as $row) {
                    $step_id = $row['or'];
                    $cp = $row['cp'];
                    if ( !is_array($step_or_arr[$step_id]) ) {
                        $step_or_arr[$step_id] = array();
                    }
                    $step_or_arr[$step_id][$obj_col_index] = $cp;
                }
                
            } 
        }
        
        $step_or_nums = array_keys($step_or_arr);
        $obj_col_index_arr = array_keys($sock_sucs);
        
        
        foreach($step_or_nums as $step_id) {
            $all_cpids=array();
            foreach($obj_col_index_arr as $obj_col_index) {
                $all_cpids[] = $step_or_arr[$step_id][$obj_col_index]; 
            }
            $all_proto_arr[] = $all_cpids;
            
            // glob_printr( $all_cpids, $FUNCNAME.":array all_cpids for ONE PRA", 1 );
        }
        
        if (sizeof($all_proto_arr)>1) {
            $tool_opts['many_PRA_flag'] = 1;
        }
        
        $show_objects = array('mo_arr'=>&$suc_inp_arr,  'mo_table'=>'CONCRETE_SUBST');
        
        $prot_edit_lib = new o_CONCRETE_PROTO_li_show1($sqlo, $show_objects, $tool_opts);
        $prot_edit_lib->open_table($sqlo);
        foreach($all_proto_arr as $one_cp_arr) {
            $prot_edit_lib->show_one_pra($sqlo, $one_cp_arr);
        }
        $prot_edit_lib->table_close();
    }
}
