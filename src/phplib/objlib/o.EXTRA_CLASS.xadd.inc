<?php
require_once('glob.obj.create.ABS.inc');
require_once 'o.EXTRA_CLASS.mod.inc';

/**
 * object specific modification class
 * @package oEXTRA_CLASS.xmod.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class oEXTRA_CLASS_xmodC extends gObjCreateC_ABS {
    
    function post_crea( &$sqlo, &$sqlo2, $objid,  $blueObjId, $option  = array()	) {
        if (!$blueObjId) return;
        
        // copy attributes
        $class_mod_lib = new oEXTRA_CLASS_mod($objid);
        $class_mod_lib->copy_all_atributes($sqlo, $blueObjId);
    }

}
