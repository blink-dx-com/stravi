<?php
// unittest for XMLRPC-method update_exp_params

class update_exp_params_TST extends BASE_TSTxrpc {
	function getXmlrpc_params() {
		$subtest  = $this->subtest;
		$exp_id   = $this->_getDefObjID( 'EXP', 'default' );
		
		$params   = array( $exp_id, array("NAME"=>"test", "NOTES"=>"yesle", "SHORTRES"=>'hallo') );
		
		switch  ( $subtest ) {
			case "noWriteAccess":
				$exp_id	 =  $this->_getDefObjID( "EXP", "noWriteAccess" );
				$params = array( $exp_id, array("NAME"=>"test", "NOTES"=>"yesle") );
				break;
			case "valueError":
				// SHORTRES contains bad value
				$params = array( $exp_id, array("NAME"=>"test", "NOTES"=>"yesle", "QUALITY"=>'badvalue') );
				break;
		}
		return $params;
	}
}
