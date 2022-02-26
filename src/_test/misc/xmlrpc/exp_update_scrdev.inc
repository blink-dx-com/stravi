<?php
// unittest for XMLRPC-method exp_update_scrdev

class exp_update_scrdev_TST extends BASE_TSTxrpc {
	function getXmlrpc_params() {
		$optInputParams = $this->params;
		$subtest  = $this->subtest;
		
		$exp_id	 = $this->_getDefObjID( "EXP", "exp_update_script" );
		$hash    = "2d8e07939738d98a5a7bfc705e02ffe0";
		$my_link_dict= array( "name" => "dev-package test");
		
		if ( $subtest == "unlink") {
			$my_link_dict = NULL;		
			$params = array( $exp_id, NULL, $my_link_dict);
		} elseif ( $subtest == "newhash") {
			$hash   = md5(time());
			$my_link_dict= array( "name" => "dev-package testNewHash");
			$params = array( $exp_id, $hash, $my_link_dict);
		} else {
			$params = array( $exp_id, $hash, $my_link_dict);
		}
		return $params;
	}
}
