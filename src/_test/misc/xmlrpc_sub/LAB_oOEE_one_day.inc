<?php
require_once 'date_funcs.inc';

class LAB_oOEE_one_day_TST extends BASE_TSTxrpc {
	function getXmlrpc_params() {
		
		$subtest = $this->subtest;
		
		$devid = 17056;
		$params  = $this->params;
		switch  ( $subtest ) {
			case 'now':
				$devid = $params;
				$date_param = date_unix2datestr(time(),1);
				$paramFunc  = array('devid'=>$devid, 'date'=>$date_param, 'viewNow'=>1);
				break;
			default:
				$paramFunc = array('devid'=>$devid, 'date'=>$params);
		}
		
		return $paramFunc;
	}
}
