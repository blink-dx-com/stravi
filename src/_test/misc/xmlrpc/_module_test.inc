<?php
// unittest for XMLRPC-method wafer_seclab

class _module_test_TST extends BASE_TSTxrpc {
	function getXmlrpc_params() {
		$subtest  = $this->subtest;
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;

		if ($subtest==NULL) {
			$error->set( $FUNCNAME, 2, "Need a subtest for this test." );
			break;
		}
		$params = array($subtest);
		return $params;
	}
}
