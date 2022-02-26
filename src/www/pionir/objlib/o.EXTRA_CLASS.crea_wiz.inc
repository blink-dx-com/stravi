<?php 
/**
 * extend for Crea_WIZ
 * @package o.EXTRA_CLASS.crea_wiz.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
require_once 'lev1/glob.obj.crea_wiz_objabs.inc';
require_once('edit.edit.inc');



class oEXTRA_CLASS_objCreaWiz extends objCreaWiz_OBJ {
    
    function preCheck(&$sqlo, $obj_name, &$newparams, $upload) {
        $create_now=0; 
        if ($this->selecter=='obj') {
            $create_now=1;
            // on COPY of object: prevent error on UNIQUE contraint for "NAME" !
            $newparams['NAME']      = $obj_name;
            $newparams['NICE_NAME'] = $obj_name;
        } 
        return array('obj_name'=>$obj_name, 'create_now'=>$create_now );
    }
}


