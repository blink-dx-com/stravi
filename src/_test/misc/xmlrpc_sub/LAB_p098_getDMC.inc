<?php

class LAB_p098_getDMC_TST extends BASE_TSTxrpc {
	
	function getSubTestKeys() {
		return array('default');
	}
	
	
	function getXmlrpc_params() {
		$subtest = $this->subtest;
		
		$inputstr  = $this->params;
		$paramFunc = array("", $inputstr) ;
		switch  ( $subtest ) {
			
			case 'default':
			default:
				$paramFunc = array($inputstr);
				break;
				
		}
		return $paramFunc;
	}
}