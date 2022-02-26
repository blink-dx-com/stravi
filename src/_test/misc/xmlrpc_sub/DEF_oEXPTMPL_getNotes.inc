<?php

class DEF_oEXPTMPL_getNotes_TST extends BASE_TSTxrpc {
	function getXmlrpc_params() {
		
		$subtest = $this->subtest;
		$params  = $this->params;
		
		$exp_id	 =  $this->_getDefObjID( "EXP_TMPL", "default" );
		$paramFunc = array($exp_id);
		
		return $paramFunc;
	}
}
