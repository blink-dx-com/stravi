<?php
// unittest for XMLRPC-method exp_update_script
// @version $Header: trunk/src/www/_tests/www/test/xmlrpc/exp_update_script.inc 59 2018-11-21 09:04:09Z $

class exp_update_script_TST extends BASE_TSTxrpc {
	function getXmlrpc_params() {
		
		$optInputParams = $this->params;
		$subtest  = $this->subtest;

		// exp_id, hash, $my_link_dict, $ic_version
		$exp_id	 = $this->_getDefObjID( "EXP", "exp_update_script" );
		$docid   = $this->_getDefObjID( "LINK", "exp_update_script" );
		
		$my_link_dict = array("name"=>"TEST ".$this->CLASSNAME, "segDir"=>"/segtest/hallo/" );
		$ic_version   = "2.3r1pre050222";
		
		if ($optInputParams!=NULL) {
			$exp_id	 = $optInputParams;
		}
		
		switch ($subtest) {
			
			case  "NewDocument":
				$randnum = rand(1,1000000);
				$newhash = md5($randnum);
				$params = array( $exp_id, $newhash, $my_link_dict, $ic_version );
				break;
			case  "NewDocument2":
				// test EMPTY NAME
				$randnum = rand(1,1000000);
				$newhash = md5($randnum);
				$my_link_dict = array("name"=>NULL, "segDir"=>"/segtest/hallo/" );
				$params = array( $exp_id, $newhash, $my_link_dict, $ic_version );
				break;
			default:
				$params = array( $exp_id, "0a69b9e4ed1eb3d6f56de438ac737875", $my_link_dict, $ic_version );
		}
		return $params;
	}
}
