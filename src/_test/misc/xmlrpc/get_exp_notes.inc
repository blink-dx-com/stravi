<?php
// unittest for XMLRPC-method array_socket_connect

class get_exp_notes_TST extends BASE_TSTxrpc {
	
	function getXmlrpc_params() {
		
		$subtest  	= $this->subtest;
		
		$expid	 = $this->_getDefObjID( "EXP", "default" );
	
		$params = array( $expid );
		
		return $params;
		
	}
	
}
