<?php
// unittest for XMLRPC-method wafer_seclab

class wafer_step_updoc_TST extends BASE_TSTxrpc {
	function getXmlrpc_params() {
		$w_wafer_id	 = $this->_getDefObjID( "W_WAFER", "default" );
		$params = array( $w_wafer_id, "xxxxx", array("name"=>"unittest"), "xxxx" );
		return $params;
	}
}
