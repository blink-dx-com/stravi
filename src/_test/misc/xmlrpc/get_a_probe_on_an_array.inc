<?php
// unittest for XMLRPC-method array_socket_connect

class get_a_probe_on_an_array_TST extends BASE_TSTxrpc {
	
	function getXmlrpc_params() {
		
		
		$optInputParams = $this->params;
		$subtest        = $this->subtest;
		
		$objid	 = $this->_getDefObjID( "ABSTRACT_ARRAY", "default" );
		
		if ($optInputParams>0) {
			$objid = $optInputParams;
		}
		
		switch  ( $subtest ) {
		
			case "mode:0":
				$params = array( $objid, 'ABSTRACT_ARRAY' );
				break;
			case "mode:1":
				$params = array( $objid, 'ABSTRACT_ARRAY', 1 );
				break;	
			case "EXP":
				$params = array( $objid, 'EXP' );
				break;
			default: 
				$params = array( $objid, 'ABSTRACT_ARRAY' );
				break;
		}
		return $params;
		
	}
	
}
