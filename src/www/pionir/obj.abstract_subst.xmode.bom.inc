<?php
require_once ("visufuncs.inc");

require_once ("f.objview.inc");	
require_once ('f.help.inc');
require_once 'o.ABSTRACT_SUBST.bom.inc';
require_once 'subs/o.ABSTRACT_SUBST_bom_form.inc';

/**
 * BOM visu
 * it contains code, which will be included in edit.tmpl.php, when the rider is set to "MODE"
 * $Header: trunk/src/www/pionir/obj.abstract_subst.xmode.vers.inc 59 2018-11-21 09:04:09Z $
 * @package    edit.tmpl.php
 * @subpackage obj._default.xmode.MODE.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class oABSTRACT_SUBST_xmode_bom {
	
    

    function xmode_start( &$sqlo, &$sqlo2, $id, &$arguByKey, &$x_rights, &$extraobj_o, &$tabArgu, $editAllow) {
    	global $error, $varcol;
    	
    	$this->objid   = $id;
    	
    	$sua_bom_lib = new oABSTRACT_SUBST_bom($id);
    	$sua_compo_pos  = $sua_bom_lib->get_compo_for_form($sqlo);
    	
    	echo '<ul>'."\n";
    	
    	$form_sua_lib = new oABSTRACT_SUBST_bom_form($this->objid);
    	$form_sua_lib->form1($sqlo, $sua_compo_pos, $editAllow);
    	
 
    	echo "\n".'</ul>';
    }

}

