<?php
// unittest for XMLRPC-method cart_batch_qaset

class socket_getarr_TST extends BASE_TSTxrpc {
	function getXmlrpc_params() {
		
		$params=array(array(3,'828288'));
		return $params;
	}
}
