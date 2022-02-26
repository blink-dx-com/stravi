<?php
// unittest for XMLRPC-method delete_raw_res

class delete_raw_res_TST extends BASE_TSTxrpc {
	function getXmlrpc_params() {
		$exp_id	 = $this->_getDefObjID( "EXP", "cytoRes" );
		$params = array( $exp_id, 0 );
		return $params;
	}
}
