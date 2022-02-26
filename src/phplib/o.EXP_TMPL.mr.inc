<?php
require_once 'o.ABS_SOCKET.subs.inc';

/**
 * experiment template multireactor functions
 * @namespace core::obj::EXP_TMPL
 * @package o.EXP_TMPL.mr.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class oEXP_TMPL_mr {
    
    
    private $exa_feats;
    const SOCK_TYPE_MULTIREACTOR='07';
    
    function __construct(int $exa_id) {
        $this->exa_id=$exa_id;
        $this->exa_feats=array();
       
    }

    /**
     * cache result ...
     * @param object $sqlo
     * @return array
     */
    function get_mr_details($sqlo) {
        
        $id = $this->exa_id;
        $this->exa_feats['ABS_SOCKET_ID'] = glob_elementDataGet( $sqlo, 'EXP_TMPL', 'EXP_TMPL_ID', $id, 'ABS_SOCKET_ID' );

        if(!$this->exa_feats['ABS_SOCKET_ID']) {
            //throw new Exception('Need a '.tablename_nice2('ABS_SOCKET'));
            return $this->exa_feats;
        }
        
        $sock_type_key = oABS_SOCKET_subs::get_socket_type_key_STAT($sqlo, $this->exa_feats['ABS_SOCKET_ID']);
        $this->exa_feats['SOCKET.type.key'] = $sock_type_key;
        return $this->exa_feats;
    }
    
    /**
     * is this a multireactor MOTHER experiment ?
     * - derived from SOCKET, ABS_SOCKET ...
     * @param object $sqlo
     * @throws Exception
     * @return number 0,1
     */
    function is_multireactor($sqlo) {
        $output = 0;
        
        $exa_feats = $this->get_mr_details($sqlo);
        $sock_type_key = $exa_feats['SOCKET.type.key'];
        if ($sock_type_key==self::SOCK_TYPE_MULTIREACTOR) {
            $output=1;
        }
        
        return $output;
    }
}

