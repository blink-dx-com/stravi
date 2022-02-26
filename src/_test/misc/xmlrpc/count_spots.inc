<?php
// unittest for XMLRPC-method array_socket_connect

class count_spots_TST extends BASE_TSTxrpc {
	
	function getXmlrpc_params() {
		
		$subtest  	= $this->subtest;
		
		$absarray	 = $this->_getDefObjID( "ABSTRACT_ARRAY", "default" );
	
		$params = array( $absarray, 'ABSTRACT_ARRAY' );
		
		return $params;
		
	}
	
}
