<?php

class DEF_oEXP_get_absArray_TST extends BASE_TSTxrpc {
	
	function getSubTestKeys() {
		return array('noParam', 'noExp', 'CAR', 'SOC', 'SOCWO', 'DIFF', 'EQUAL');
	}
	
	function getXmlrpc_params() {
		
		$subtest = $this->subtest;
		$params  = $this->params;
		
		switch  ( $subtest ) {
			case 'PARAM':
				$paramFunc = array($params);
				break;
			case 'noParam':
				//function call w/o parameters
				$paramFunc = array();
				//expected return value: array(-1, "Parameter missing (EXP_ID).")
				break;
			case 'noExp':
				//invalid experiment
				$exp_id = '999999';
				$paramFunc = array($exp_id);
				//expected return value: array(-2, "Experiment(ID:999999) not found.")
				break;
			case 'CAR':
				//exp with a valid concrete array
				$exp_id = '340767';
				$paramFunc = array($exp_id);
				//expected return value: array(0, ((7941)))
				break;
			case 'SOC':
				//exp with a valid socket
				$exp_id = '340768';
				$paramFunc = array($exp_id);
				//expected return value: array(0, ((9099)))
				break;
			case 'SOCWO':
				//experiment with socket not connected to an abstract array
				$exp_id = '340769';
				$paramFunc = array($exp_id);
				//expected return value: array(0, (( )))
				break;
			case 'DIFF':
				//experiment with both, concrete array and socket connected with different abstract arrays,
				//which is not allowed but maybe possible and thus needs to be checked
				$exp_id = '340766';
				$paramFunc = array($exp_id);
				//expected return value: array(-3, "Found two different abstract arrays (ID1:9099, ID2:9104))
				break;
			case 'EQUAL':
				//experiment with both, concrete array and socket connected with same abstract array
				$exp_id = '340765';
				$paramFunc = array($exp_id);
				//expected return value: array(0, ((9104)))
				break;		
		}
		return $paramFunc;
			
	}
}