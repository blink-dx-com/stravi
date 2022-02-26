<?php
/**
 * sub functions for AlterEditor
 * @package 
 * @swreq UREQ:6576 o.CONCRETE_PROTO > Protokoll-Editor > Alternative
 * @package o.PRC.altereditor_sub.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/phplib/gui/o.PRC.altereditor_sub.inc 59 2018-11-21 09:04:09Z $
 */
require_once('o.SATTACH.subs.inc');
require_once('o.S_VARIO.mod.inc');

class oPRC_altereditor_sub {
	
	function getFormData(&$sqlo, $prcid) {
		$vals = NULL;
		$variolib = new oS_VARIO_sub('CONCRETE_PROTO');
		$valstr = $variolib->getValByKey($sqlo, $prcid, 'AlterEditor');
		if ($valstr!=NULL) {
			$vals = unserialize($valstr);
		}
		
		return $vals;
	}
}
