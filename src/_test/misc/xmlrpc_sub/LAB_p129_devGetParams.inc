<?php

class LAB_p129_devGetParams_TST extends BASE_TSTxrpc {
	function getXmlrpc_params() {
		
		$subtest = $this->subtest;
		$params  = $this->params;
		
		$paramFunc = array('MSK');	
		
		return $paramFunc;
	}
}
