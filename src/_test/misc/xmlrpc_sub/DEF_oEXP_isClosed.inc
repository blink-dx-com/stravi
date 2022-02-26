<?php

class DEF_oEXP_isClosed_TST extends BASE_TSTxrpc {
	
	function getSubTestKeys() {
		return array('PARAM', 'noParam', 'wrongID');
	}
	
	function getXmlrpc_params() {
		
		$subtest = $this->subtest;
		$params  = $this->params;
		
		switch  ( $subtest ) {
			case 'PARAM':
				$paramFunc = array($params);
				break;
			case 'noParam':
				//function call w/o parameters
				$paramFunc = array();
				//expected return value: array(1, "No object with ID:  found in table EXP.")
				break;
			case 'wrongID':
				//invalid experiment
				$exp_id = '99999999';
				$paramFunc = array($exp_id);
				//expected return value: array(1, "No object with ID: 99999999 found in table EXP.)
				break;
			default:
				$exp_id = '758552';
				$paramFunc = array($exp_id);
		}
		return $paramFunc;
			
	}
}