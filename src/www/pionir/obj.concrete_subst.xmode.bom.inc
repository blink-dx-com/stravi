<?php
/**
 * show CONCRETE_SUBST BOM
 * $Header:  59 2018-11-21 09:04:09Z $
 * @package    edit.tmpl.php
 * @subpackage 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
require_once('insert.inc');
require_once ('func_form.inc');
require_once 'gui/o.CONCRETE_SUBST.bom_gui.inc';

class oCONCRETE_SUBST_xmode_bom {

	
	/**
	 * 
	 * @return 
	 * @param object $sql
	 * @param object $sql2
	 * @param object $id
	 * @param object $arguByKey
	 * @param object $x_rights
	 * @param object $extraobj_o
	 * @param object $tabArgu
	 * @param object $editAllow
	 * @global array $_REQUEST['parx'] 
	 * @global int $_REQUEST['go'] 
	 */
    function xmode_start( &$sqlo, &$sqlo2, $id, &$arguByKey, &$x_rights, &$extraobj_o, &$tabArgu, $editAllow, $editmode) {
		global $error;
		
		echo "<ul>\n";
		
		$suc_help_lib = new oCONCRETE_SUBST_bom_gui($sqlo, $id);
		if ( $editAllow  > 0 ) {
		    echo '[<a href="glob.objtab.import.php?tablename=SUC_BOM&parx[action]=insert">Import BOM from file</a>]<br>';
		    $suc_help_lib->form_BOM($sqlo);
		} else {
		    $suc_help_lib->show_BOM($sqlo);
		}
		


		echo "</ul>\n";
	
	}

}

