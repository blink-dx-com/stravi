<?php

class DEF_oPROJ_getElems_TST extends BASE_TSTxrpc {
	function getXmlrpc_params() {
		
		$subtest = $this->subtest;
		$params  = $this->params;
		
		$projID  = 88631;
		switch  ( $subtest ) {
			case "exp_only":
				$paramFunc = array($projID, array("EXP"));
				break;
			default:
				$paramFunc = array($projID);
				break;
		}
		
		return $paramFunc;
	}
}
