<?php
// unittest for XMLRPC-method exp_get_scrdev

class exp_get_scrdev_TST extends BASE_TSTxrpc {
	function getXmlrpc_params() {
		$subtest  = $this->subtest;
		$params   = $this->params;
		
		$funcopt = 0;
		
		$exp_id	 = $this->_getDefObjID( "EXP", "exp_update_script" ); // or $exp_id	 = 1888700;
		if ($params!=NULL) $exp_id	 = $params;
		
		switch ($subtest) {
			case 'OPT=1':
				$funcopt = 1;
				break;
			case 'OPT=2':
				$funcopt = 2;
				break;
		}
		$params = array( $exp_id, $funcopt );

		return $params;
	}
}
