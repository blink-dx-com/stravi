<?php
// unittest for XMLRPC-method get_href_pos_sys2

class get_href_pos_sys2_TST extends BASE_TSTxrpc {
	function getXmlrpc_params() {
		$subtest  = $this->subtest;
		
		$abs_arr_id	 =  $this->_getDefObjID( "ABSTRACT_ARRAY", "default" );
		if ($subtest=='oneSpot') {
			$abs_arr_id	 =  1480 ;
		}
		
		$params = array( $abs_arr_id, 'ABSTRACT_ARRAY' );
		return $params;
	}
}
