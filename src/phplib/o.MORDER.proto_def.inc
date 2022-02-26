<?php 
require_once 'o.S_VARIO.subs.inc';
require_once 'o.S_VARIO.mod.inc';

/**
 * MORDER sub functions
 * @author Steffen
 * @package o.MORDER.proto_def.inc
 * 
 * @var array PROTO_DEF_STRUCT
 *    array [order] = pra_id -- where order=proto_arr_STRUCT: key: 'or'
 */
class oMORDER_proto_def {
    
    const VARIO_KEY='SUA.protos_use';
    const VARIO_SUA_PRC_AuditActions='PRC_AuditActions';  // name of Workflow-code-script which will be called after Audit-Status-Change of an protocol action ...
            // of  the derived SUC
    const VARIO_PRA_AuditPostAction='AuditPostActionSUC'; // 0 or 1: triggers Audit-Status Change action on the belonging MAC
    const S_OBJLINK_PRA_GrpEmailKey='oGROUP.info_email';
    
    const VARIO_MORDER_PROTO_DET = 'SUC.plan.proto.det'; // MORDER > save planned protocol details for the SUC
    
    function __construct(int $order_id) {
        $this->order_id = $order_id;
    }
    
    /**
     * save
     * @param object $sqlo
     * @param array $proto_def PROTO_DEF_STRUCT
     *    
     */
    function save_data($sqlo, array $proto_def) {
        $vario_mod_lib = new oS_VARIO_mod();
        $vario_mod_lib->setObject( $sqlo, 'MORDER', $this->order_id);
        $vario_mod_lib->updateKeyVal($sqlo, self::VARIO_KEY, json_encode($proto_def) );
    }
    
    
    
    // return array PROTO_DEF_STRUCT
    function get_data(object $sqlo) {
        $answer=NULL;
        $proto_def_str = oS_VARIO_sub::getValByTabKey($sqlo, 'MORDER', $this->order_id,  self::VARIO_KEY);
        if ($proto_def_str) {
            $answer = json_decode($proto_def_str,TRUE);
        } else {
            $answer = array();
        }
        return $answer;
    }
    
    // protocol step details
    // format : 
    /**
     * 
     * @param object $sqlo
     * @param array $proto_details
     *   [$STEP]= array(
     *      'pra'  => ABSTRACT_PROTO_ID, 
     *      'steps'=> o.PROTO.steps.inc::CP_STEP_gui_STRUCT 
     *   );
     */
    function proto_details_SAVE($sqlo, array $proto_details) {
        $vario_mod_lib = new oS_VARIO_mod();
        $vario_mod_lib->setObject( $sqlo, 'MORDER', $this->order_id);
        $vario_mod_lib->updateKeyVal($sqlo, self::VARIO_MORDER_PROTO_DET, json_encode($proto_details) );
    }
    function proto_details_LOAD($sqlo) {
        $answer=NULL;
        $proto_def_str = oS_VARIO_sub::getValByTabKey($sqlo, 'MORDER', $this->order_id,  self::VARIO_MORDER_PROTO_DET);
        if ($proto_def_str) {
            $answer = json_decode($proto_def_str,TRUE);
        } else {
            $answer = array();
        }
        return $answer;
    }
    
    
}