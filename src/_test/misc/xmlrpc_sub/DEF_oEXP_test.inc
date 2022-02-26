<?php

class DEF_oEXP_test_TST extends BASE_TSTxrpc {
	function getXmlrpc_params() {
		
		$subtest = $this->subtest;
		$params  = $this->params;
		
		$exp_id	 =  $this->_getDefObjID( "EXP", "cytoRes" );
		$paramFunc = array($exp_id) ;
		
		return $paramFunc;
	}
}