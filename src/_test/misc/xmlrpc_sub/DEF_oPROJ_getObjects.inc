<?php

class DEF_oPROJ_getObjects_TST extends BASE_TSTxrpc {
	
	function getSubTestKeys() {
		return array('oneTable');
	}
	
	function getXmlrpc_params() {
		
		$subtest = $this->subtest;
		$params  = $this->params;
		
		$proj_id = $this->_getDefObjID( "PROJ", "default" );
		$paramFunc = array($proj_id);
		switch  ( $subtest ) {
			case "oneTable":
				$paramFunc = array($proj_id, 'CONCRETE_SUBST'); // only one type
				break;
			default:
				break;
		}
		
		return $paramFunc;
	}
}
