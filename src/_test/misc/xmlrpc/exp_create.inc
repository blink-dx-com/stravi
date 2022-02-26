<?php
// unittest for XMLRPC-method exp_create

class exp_create_TST extends BASE_TSTxrpc {
	function getXmlrpc_params() {
		
		$subtest  = $this->subtest;
		
		switch  ( $subtest ) {
			case "no-project":
				$projid=0;
				break;
			default:
				$projid=$this->_getDefObjID( "PROJ", "default" );
		}
		
		$tmplid = $this->_getDefObjID( "EXP_TMPL", "default" );
		$params = array ( 
			'UT_example01',
			$tmplid,
			'expname',
			$projid,
			 );
			
		return $params;
	}
}
