<?php
// unittest for XMLRPC-method array_socket_connect

class get_refp_pos_TST extends BASE_TSTxrpc {
	
	function getXmlrpc_params() {
		
		$subtest  	= $this->subtest;
		
		$imgid	 = 187059; // on test database; contains spots with NEGATIVE spot-ids
		$expid   = 0; // not needed
	
		$params = array( $expid, $imgid );
		
		return $params;
		
	}
	
}
