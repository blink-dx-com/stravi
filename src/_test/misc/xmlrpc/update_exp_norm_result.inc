<?php
// unittest for XMLRPC-method update_exp_norm_result

class update_exp_norm_result_TST extends BASE_TSTxrpc {
	function getXmlrpc_params() {
		
		$subtest = $this->subtest;
		$exp_id	 =  $this->_getDefObjID( "EXP", "cytoRes" );
			
		$tmpparam = array($exp_id);
		$this->XmlCliObj->funcCall( "delete_spot_res", $tmpparam );
	
		$spot_result =  array( 
					1=>array(2,2.2,0),
					2=>array(3,3.2,0),
					3=>array(4,4.2,1),	 		
				);
		$coldefs = array("spot_id", "value", "flags");
		
		switch  ( $subtest ) {
			case "err1":
				// bad value
				$spot_result[2][2] = NULL; 
				break;
			case "err2":
				// bad column definition
				$coldefs = array("spot_id", "value", "flags", "flags"); 
				break;
		}
		$params = array( $exp_id, $coldefs, $spot_result);
		return $params;
	}
}
