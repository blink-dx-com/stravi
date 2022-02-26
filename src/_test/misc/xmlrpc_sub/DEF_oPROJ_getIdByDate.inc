<?php

class DEF_oPROJ_getIdByDate_TST extends BASE_TSTxrpc {
	function getXmlrpc_params() {
		
		$subtest = $this->subtest;
		$params  = $this->params;
		
		$paramFunc = array('2010-06-28', '85003', 'Fetzel');
		
		return $paramFunc;
	}
}
