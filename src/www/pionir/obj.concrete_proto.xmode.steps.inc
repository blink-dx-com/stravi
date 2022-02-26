<?php
/**
 * - edit proto steps
 * - it contains code, which will be included in edit.tmpl.php, when the rider is set to "MODE"
 * $Header: trunk/src/www/pionir/obj.concrete_proto.xmode.steps.inc 59 2018-11-21 09:04:09Z $
 * @package    edit.tmpl.php
 * @subpackage obj.concrete_proto.xmode.steps.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
require_once ("gui/o.CONCRETE_PROTO.edit.inc");
require_once ("glob.obj.usagegui.inc");
require_once ("f.objview.inc");	
require_once ('o.S_VARIO.subs.inc');

class oCONCRETE_PROTO_xmode_steps {
	
function __construct( &$editLib ) {
	$this->editLib = &$editLib;
}


function xmode_start( &$sql, &$sql2, $id, &$arguByKey, &$x_rights, &$extraobj_o, &$tabArgu, $editAllow) {
	global $error;
	
	$tablename = 'CONCRETE_PROTO';
	$CCT_ACCESS_ID = $arguByKey['CCT_ACCESS_ID'];
	$abstract_id   = $arguByKey['ABSTRACT_PROTO_ID'];
	$abstract_name = obj_nice_name ( $sql, 'ABSTRACT_PROTO', $abstract_id );
	echo '<ul>'."\n";
	$moObj = $this->editLib->hasMother( $sql );
	if ($moObj!=NULL) {
		$moTable2 = mothertable_get2($moObj['t']);
		if ($moTable2!=NULL) {
			$moTable = $moTable2;
		} else {
			$moTable = $moObj['t'];
		}
		if ($moObj['id']) {
		    $objLinkLib = new fObjViewC();
    		$html = $objLinkLib->bo_display( $sql, $moTable, $moObj['id'] );
		}
		if ($moObj['found']>1) {
		    $objLinkLib = new fObjViewC();
		    $html = '<img src="'.$objLinkLib->_getIcon($moTable).'"> more than one object found.';
		}
		echo '<span style="color:gray;">mother object:</span> <b>'.$html.'</b>';
		echo '&nbsp;&nbsp;&nbsp;<span style="color:gray;">('.tablename_nice2($moTable) .')</span> <br />'."\n";
	}
	echo '<font color=gray>protocol abstract:</font> <b>'.$abstract_name.'</b><br />'."\n";
	if ( !$x_rights['insert'] ) $editAllow = 0;
	
	if ($editAllow) {
		$objShowUseLib = new gObjUsageGui();
		$useopt = array('show'=>-1);
		$objShowUseLib->show_usage($sql, $sql2, $tablename, $id, $CCT_ACCESS_ID, $useopt);
		$numUse = $objShowUseLib->getObj_num_all();
	}
	echo '</ul>'."\n";
	
	$variolib = new oS_VARIO_sub('ABSTRACT_PROTO');
	$pluginAddress = $variolib->getValByKey($sql, $abstract_id, 'AlterEditor');
	if ($pluginAddress=='html_attachment') {
		// alternative Editor
		
		require_once('gui/o.PRC.altereditor1.inc');
		$edFormLib = new oPRC_altereditor1($sql);
		$edFormLib->set_PRC($sql, $id);
		if ($error->printAll())  {
			return;
		}
		
		echo '<hr>'."\n";
		
		$edFormLib->showform( $sql, $sql2, $editAllow );
		$error->printAll();
		
		return;
	}
	
	
	$edFormLib = new oConcProtoEdform($sqlo, $id);
	$edFormLib->showform( $sql, $editAllow );
	
}

}
