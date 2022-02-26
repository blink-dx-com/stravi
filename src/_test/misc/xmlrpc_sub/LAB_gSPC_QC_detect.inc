<?php

class LAB_gSPC_QC_detect_TST extends BASE_TSTxrpc {
	
	function getXmlrpc_params() {
		
		$subtest = $this->subtest;
		$params  = $this->params;
		
		$paramFunc=array();
		$paramFunc['LFC']  = 'Production';
		$paramFunc['Date'] = '2018-04-19';
		$paramFunc['Batch_num'] = 2;
		$paramFunc['product'] = 'Detect';
		
		switch ($subtest) {
			case "error":
				break;
		}
		
		/*
		$paramFunc = array();
		if ($params!=NULL) {
			$paramFunc['Date']=trim($params);
		}
		*/

		
		return $paramFunc;
	}
}
