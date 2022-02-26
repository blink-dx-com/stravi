<?php

class DEF_gObj_getAssParams_TST extends BASE_TSTxrpc {
	function getXmlrpc_params() {
		
		$subtest = $this->subtest;
		$params  = $this->params;
		
		$exp_id	   = $this->_getDefObjID( "EXP", "default" );
		$paramFunc = array('EXP_HAS_PROTO', $exp_id , array('STEP_NO', 'CONCRETE_PROTO_ID'));
		$cols      = array('STEP_NO', 'CONCRETE_PROTO_ID');
		
	
		switch  ( $subtest ) {
			case 'PARAM':
				$this->outinfo('params-input: EXP_ID');
				$paramFunc = array('EXP_HAS_PROTO', $params, $cols );
				break;
			
				
			case 'err1':	#fine (associates array)
				$paramFunc = array('EXP', $params, array('NAME') );
				break;
				
			
		}

		
		
		return $paramFunc;
	}
}