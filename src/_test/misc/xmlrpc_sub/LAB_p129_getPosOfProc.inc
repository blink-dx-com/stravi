<?php

class LAB_p129_getPosOfProc_TST extends BASE_TSTxrpc {
	function getXmlrpc_params() {
		
		$subtest = $this->subtest;
		$params  = $this->params;
		
		
		$dev_id	 =  $this->_getDefObjID( "CART_BATCH","default");
		$paramFunc = array($dev_id, 'LAI');

		
		return $paramFunc;
	}
}
