<?php

class DEF_oEXP_TMPL_getScript_TST extends BASE_TSTxrpc {
	
	function getParamInfo() {
		return array(
			'info'=>'subtest: "allScripts", "error1"; optional param: {EXP_TMPL_ID},{Protokind}'
		);
	}
	
	function getXmlrpc_params() {
		
		$subtest = $this->subtest;
		$params  = $this->params;
		
		switch  ( $subtest ) {
			case "allScripts": //get all Scripts + kind
				$paramFunc = array ('4203');
				break;
			case "error1": //get all Scripts + kind
				$paramFunc = array ('4203','wrongType');
				break;
			default:
				break;
		}
		
		if ( trim($params)!=NULL ) {
			$paramarr = explode(',',$params);
			// set EXP_TMPL_ID
			$paramFunc = array ( trim($paramarr[0]), trim($paramarr[1]) );
		}
		
		return $paramFunc;
	}
}
