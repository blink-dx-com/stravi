<?php 
require_once 'lev1/glob.obj.crea_wiz_objabs.inc';

/**
 * extend for Crea_WIZ
 * @package o.IMG.crea_wiz.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class oIMG_objCreaWiz extends objCreaWiz_OBJ {
    
    function preCheck(&$sqlo, $obj_name, &$newparams, $upload) {
        if ($upload["userfile_name"] != '' && ($obj_name == '')) $obj_name = $upload["userfile_name"];
        
        return array('obj_name'=>$obj_name, 'create_now'=>1 );
    }
    
}