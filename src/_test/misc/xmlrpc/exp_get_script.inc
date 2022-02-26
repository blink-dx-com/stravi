<?php
// unittest for XMLRPC-method exp_get_script

class exp_get_script_TST extends BASE_TSTxrpc {
	function getXmlrpc_params() {
		$exp_id	 = $this->_getDefObjID( "EXP", "exp_update_script" );
		$params = array( $exp_id );
		return $params;
	}
}
