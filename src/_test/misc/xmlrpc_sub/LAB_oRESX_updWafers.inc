<?php

class LAB_oRESX_updWafers_TST extends BASE_TSTxrpc {
	function getXmlrpc_params() {
		
		$subtest = $this->subtest;
		$params  = $this->params;
		
		$obj_id	 =  $this->_getDefObjID( "RESX","update");
		$waf1	 =  $this->_getDefObjID( "W_WAFER","default");
		$waf2	 =  $this->_getDefObjID( "W_WAFER","default2");
		$wafarr=array(array(1,$waf1),array(2,$waf2));
		$paramFunc = array($obj_id, $wafarr);
			
		
		return $paramFunc;
	}
}
