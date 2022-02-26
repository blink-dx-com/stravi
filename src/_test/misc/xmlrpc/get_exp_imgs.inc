<?php
// unittest for XMLRPC-method array_socket_connect

class get_exp_imgs_TST extends BASE_TSTxrpc {
	
	function getXmlrpc_params() {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$subtest  	= $this->subtest;
		
		$expid	 = $this->_getDefObjID( "EXP", "default" );
		if ($this->params!=NULL) {
			$expid = trim($this->params);
			if ( !is_numeric($expid) ) {
				$error->set( $FUNCNAME, 1, 'INPUT: if param given; must be NUMERIC' );
				return;
			}
		}
	
		$params = array( $expid );
		
		return $params;
		
	}
	
}
