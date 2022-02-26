<?php

class LAB_p131_printLabel_TST extends BASE_TSTxrpc {
	function getXmlrpc_params() {
		
		$subtest = $this->subtest;
		
		$cabnatid = 40;
		switch  ( $subtest ) {
			case 'prod':
				$paramFunc = array('prod', $cabnatid);
				break;
			case 'user':
				$paramFunc = array('user', $cabnatid);
				break;
			case 'wrap':
				$paramFunc = array('wrap', $cabnatid);
				break;
		}
		
		return $paramFunc;
	}
}
