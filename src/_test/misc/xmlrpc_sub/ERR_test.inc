<?php
/**
 * generated ERROR-tests for XML-RPC
 * @swreq SREQ:0001179:001 xml-rpc > metacall > error-handling
 * @author Steffen Kube
 */
class ERR_test_TST extends BASE_TSTxrpc {
	function getSubTestKeys() {
		return array('wrongBase', 'ErrorBigger10', 'ErrorSmaller10');
	}
	
	function getXmlrpc_params() {
		
		$subtest = $this->subtest;
		$params  = $this->params;
		
		
		switch  ( $subtest ) {
			case 'wrongBase':
				// wrong base
				break;
			case 'ErrorBigger10':
				$this->metacallFunc = 'DEF/oEXP_test';
				$paramFunc = array('bigger10');
				break;
			case 'ErrorSmaller10':
				$this->metacallFunc = 'DEF/oEXP_test';
				$paramFunc = array('smaller10');
				break;
		}
		
		
		return $paramFunc;
	}
}
