<?php
// unittest for XMLRPC-method cart_batch_qaset

class query_TST extends BASE_TSTxrpc {
	function getXmlrpc_params() {
		
		$params = array("select DB_USER_ID from DB_USER where nick like 'steffen'");
		return $params;
	}
}
