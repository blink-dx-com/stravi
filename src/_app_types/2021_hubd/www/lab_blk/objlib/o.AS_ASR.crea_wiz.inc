<?php
require_once 'lev1/glob.obj.crea_wiz_objabs.inc';

/**
 * extend for Crea_WIZ
 * @package o.AS_ASR.crea_wiz.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class oAS_ASR_objCreaWiz_G extends objCreaWiz_OBJ_gui {
    function __construct() {
        parent::__construct();
        $this->alternate_wiz=array('script'=>'p.php', 'params'=> 'mod=LAB/o.AS_ASR.insert_c');
    }
    
}