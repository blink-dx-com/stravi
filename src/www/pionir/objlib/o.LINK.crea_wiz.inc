<?php
require_once 'lev1/glob.obj.crea_wiz_objabs.inc';

/**
 * extend for Crea_WIZ
 * @package o.LINK.crea_wiz.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class oLINK_objCreaWiz extends objCreaWiz_OBJ {
    
    function preCheck(&$sqlo, $obj_name, &$newparams, $upload) {
        
        if ($upload["userfile_name"] != '' && ($obj_name == '')) $obj_name = $upload["userfile_name"];
        if ($obj_name == '') $obj_name = 'new document'; // because, it is mandatory
        
        return array('obj_name'=>$obj_name, 'create_now'=>1 );
    }
    
}