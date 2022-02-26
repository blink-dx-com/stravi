<?php

class DEF_gObj_getParams_TST extends BASE_TSTxrpc {
	function getXmlrpc_params() {
		
		$subtest = $this->subtest;
		$params  = $this->params;
		
		$exp_id	   = $this->_getDefObjID( "EXP", "default" );
		$cols      = array('NAME', 'NOTES');
		$paramFunc = array('EXP', $exp_id, $cols) ;
		#$paramFunc = (array('RESX', 1446, array('NAME')));
		switch  ( $subtest ) {
			case 'PARAM':
				$this->outinfo('params-input: EXP_ID');
				$paramFunc = array('EXP', $params, $cols );
				break;
			case 'lc_tab':	#error (lower-case tablename)
				$paramFunc = array('exp_tmpl', 4215, array('NAME'));
				break;
			case 'lc_att':	#error (lowercase attribute)
				$paramFunc = array('EXP_TMPL', 4215, array('name'));
				break;
			case 'up_all':	#fine (all uppercase)
				$paramFunc = array('EXP_TMPL', 4215, array('NAME'));
				break;
				
			case 'objfull1':	#fine (all uppercase)
				$paramFunc = array( 'ABSTRACT_SUBST', 146025, array('NAME', 'NOTES', 'EXTRA_OBJ_ID', 'CCT_ACCESS_ID'), array('vario'=>1) );
				break;
				
			case 'ass1':	#fine (associates array)
				$paramFunc = array('EXP_HAS_PROTO', $exp_id , array('STEP_NO', 'CONCRETE_PROTO_ID'));
				break;
				
			case 'err02':	# error test; object not found
				$paramFunc = array('EXP_TMPL', 999999, array('NAME'));
				break;
			case 'err03':	# error test; bad column
				$paramFunc = array('EXP', $exp_id, array('NAME', 'BADCOL'));
				break;
		}

		
		
		return $paramFunc;
	}
}