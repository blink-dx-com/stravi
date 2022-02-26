<?php

class LAB_gSAPWE_TST extends BASE_TSTxrpc {
	function getXmlrpc_params() {
		
		$subtest = $this->subtest;
		$params  = $this->params;
		
		switch ($subtest) {
			case "ECM.error":
				break;
		}
		
		$paramFunc = array();
		if ($params!=NULL) {
			$paramFunc['wedatum']=trim($params);
		}

		
		return $paramFunc;
	}
}
