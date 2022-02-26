<?php

class LAB_p098_getNameParamsBySerial_TST extends BASE_TSTxrpc {
	
	function getSubTestKeys() {
		return array('noParam', 'invalidParam', 'noCarttype', 'noArrayBatch');
	}
	
	
	function getXmlrpc_params() {
		$subtest = $this->subtest;
		$serial	= '0014100001';
		$paramFunc = array($serial) ;
		switch  ( $subtest ) {
			case 'noParam':
				//function call w/o parameters
				$paramFunc = array();
				//expected return value: array(, "Parameter missing or invalid.")
				break;
			case 'invalidParam':
				$serial = '99999999999';
				$paramFunc = array($serial);
				break;
			case 'noCarttype':
				break;
			case 'noArrayBatch':
				break;
		}
		return $paramFunc;
	}
}