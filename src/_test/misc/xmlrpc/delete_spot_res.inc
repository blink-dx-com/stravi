<?php
// unittest for XMLRPC-method delete_spot_res

class delete_spot_res_TST extends BASE_TSTxrpc {
	function getXmlrpc_params() {
		$exp_id	 = $this->_getDefObjID( "EXP", "exp_update_script" );
		$params = array( $exp_id );
		return $params;
	}
}
