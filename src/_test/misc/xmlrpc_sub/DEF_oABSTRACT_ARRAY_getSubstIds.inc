<?php

class DEF_oABSTRACT_ARRAY_getSubstIds_TST extends BASE_TSTxrpc {
	function getXmlrpc_params() {
		
		$subtest = $this->subtest;
		$params  = $this->params;
		
		$array_id  = $this->_getDefObjID( "ABSTRACT_ARRAY", 'default' );
		$array_id  = 102;
		$paramFunc = array($array_id) ;
		
		return $paramFunc;
	}
}
