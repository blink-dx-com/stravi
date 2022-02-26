<?php

class LAB_p129_createSocket_TST extends BASE_TSTxrpc {
	function getXmlrpc_params() {
		
		$subtest = $this->subtest;
		$params  = $this->params;
		
		$subtest = $this->subtest;
		
		switch ($subtest) {
			case "001": //Batch existiert nicht
				$paramFunc = array('DMC'=>'1127300005', 'ILN'=>'674');
				break;
			case "002": // error; no batch_id
				$paramFunc = array('DMC'=>'1127300456', 'ILN'=>'');
				break;
			case "002.1": // error; no dmc
				$paramFunc = array('DMC'=>'',  'ILN'=>'315');
				break;
			case "002.2": // error; wirrer dmc
				$paramFunc = array('DMC'=>'test',  'ILN'=>'315');
				break;
			case "003": // error: batch not exists
				$paramFunc = array('DMC'=>'1127300456',  'ILN'=>'9999999999');
				break;
			case "004": // o.k. cart exists
				$paramFunc = array('DMC'=>'1128000020',  'ILN'=>'315');
				break;
			case "005": // o.k. cart CREATED
				$paramFunc = array('DMC'=>'1128000047',  'ILN'=>'315');
				break;
			case "006": // o.k. cart CREATED with takeout = '1'
				$paramFunc = array('DMC'=>'1128000046',  'ILN'=>'315', 'takeout'=>'1');
				break;
			case "007": // o.k. cart CREATED with takeout = '0'
				$paramFunc = array('DMC'=>'1128000043',  'ILN'=>'315', 'takeout'=>'0');
				break;
			case "008": // error; takeout not numeric
				$paramFunc = array('DMC'=>'1128000044',  'ILN'=>'315', 'takeout'=>'test');
				break;
			case "009": // o.k. takeout > 1
				$paramFunc = array('DMC'=>'1128000049',  'ILN'=>'315', 'takeout'=>'2');
				break;
			case "010": // barcode delivered
				$paramFunc = array('DMC'=>'1128000045',  'ILN'=>'315', 'takeout'=>'2', 'barcode'=>'12345');
				break;
			case "011": // o.k. cart exists, update values
				$paramFunc = array('DMC'=>'1128000045',  'ILN'=>'315', 'takeout'=>'1', 'barcode'=>'67890');
				break;
		}
		return $paramFunc;
	}
}
