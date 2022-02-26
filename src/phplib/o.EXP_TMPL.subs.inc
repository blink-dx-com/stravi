<?php
require_once 'o.S_OBJLINK.subs.inc';

/**
 * EXP_TMPL : subs
 * @package o.EXP_TMPL.subs.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class oEXP_TMPL_subs {

    function __construct( $exp_tmpl_id ) {
        $this->exp_tmpl_id = $exp_tmpl_id;
    }
    
    /**
     * get ASR_lib ID
     * @param object $sqlo
     * @return number|mixed
     */
    function get_ASR_lib($sqlo) {
        
        $obj_link_lib = new oS_OBJLINK_subs();
        $asr_ids = $obj_link_lib->getChiObjByChiTab($sqlo, 'EXP_TMPL', $this->exp_tmpl_id, 'AS_ASR');
        if (empty($asr_ids)) return 0;
        $asr_id = current($asr_ids);
        return $asr_id;
        
    }
    
    /**
     * get KEY of CARTRIDGE:KEY
     * @param object $sqlo
     * @return NULL|string
     */
    function get_cart_SOCK_TYPE_KEY($sqlo) {
        
        $ABS_SOCKET_ID = glob_elementDataGet( $sqlo, 'EXP_TMPL', 'EXP_TMPL_ID', $this->exp_tmpl_id, 'ABS_SOCKET_ID');
        if (!$ABS_SOCKET_ID) return NULL;
        
        $sqlsel = 'h.KEY from ABS_SOCKET x join H_SOCK_TYPE h on x.H_SOCK_TYPE_ID=h.H_SOCK_TYPE_ID'.
            ' where x.ABS_SOCKET_ID='.$ABS_SOCKET_ID;
        $sqlo->Quesel($sqlsel);
        $sqlo->ReadRow();
        $key = $sqlo->RowData[0];
        
        return $key;
    }
    
    /**
     * is a multireactor ?
     * @param object $sqlo
     * @return number
     */
    function is_multireactor($sqlo) {
        
        $is_multi=0;
        $type_key = $this->get_cart_SOCK_TYPE_KEY($sqlo);
        
        if ($type_key=='07') {
            $is_multi=1;
        }
        
        return $is_multi;
    }

}