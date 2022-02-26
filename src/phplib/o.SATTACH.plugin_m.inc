<?php
/**
 * o.SATTACH > plugin class
 * @package o.SATTACH.plugin_m.inc
 * @swreq UREQ:14151 o.SUC > EVAgreen raw data import
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com) 
 * @version $Header: trunk/src/phplib/o.SATTACH.plugin_m.inc 59 2018-11-21 09:04:09Z $
 */
class ATPL_abstract {
	
	function __set_obj($tablename, $objid) {
		$this->tablename=$tablename;
		$this->objid =$objid;
	}
	
	// abstract function
	/**
	 * 
	 * @param unknown $sqlo
	 * @param int $attach_pos - REL_ID of attachment
	 */
	function start($sqlo, $attach_pos) {}
}