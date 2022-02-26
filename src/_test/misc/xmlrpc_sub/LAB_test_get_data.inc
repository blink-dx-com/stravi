<?php

class LAB_test_get_data_TST extends BASE_TSTxrpc {
	function getXmlrpc_params() {
		
		$subtest = $this->subtest;
		$params  = $this->params;

		$paramFunc = array('sessvars');

		
		return $paramFunc;
	}
}
