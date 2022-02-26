<?php

/**
 * helper of PROTOQUANT execute modules, NOT for the config tool !
 * @package o.PROTO.quant_gui.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class oProtoQuantGuiC {
    
    function __construct($q_table) {
        $this->q_table = $q_table; 
    }
    
    function get_config_url() {
        $url='obj.protocol.quant.php?action=sel_config&q_table='.$this->q_table;
        return $url;
    }
}