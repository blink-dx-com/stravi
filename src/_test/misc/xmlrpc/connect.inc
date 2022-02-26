<?php
// unittest for XMLRPC-method CONNECT

class connect_TST extends BASE_TSTxrpc {
	
	function getXmlrpc_params() {
		
		$subtest  	= $this->subtest;
	
		$params = array( );
		$this->metacallFunc = 'get_version'; // CHANGE the XMLRPC method  !!!!!
		
		return $params;
		
	}
	
}
