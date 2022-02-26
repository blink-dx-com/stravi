<?php

/**
 * generated ERROR-tests for the API
 * - called in _tests/www/test/xmlrpc_sub/ERR_test.inc
 * @swreq SREQ:0001179:001 xml-rpc > metacall > error-handling
 * @param 'method'
 * @author Steffen Kube
 */
class gUT_test_XRPC  extends XRPC_BASE {
	


function &run() {
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	$retval=1;
	
	$param0 = $this->_params['method'];
	
	if ($param0==NULL) {
	    $error->set( $FUNCNAME, 1, 'No method parameter given.' );
	}
	
	if ($param0=='smaller10') {
		$error->set( 'subfunction1', 6, 'sub error1' );
		$error->set( $FUNCNAME, 9, 'error "smaller10" occurred' );
		return;
	}
	
	if ($param0=='bigger10') {
		$error->set( 'subfunction1', 1, 'sub error1' );
		$error->set( $FUNCNAME, 12, 'error "bigger10" occurred' );
		return;
	}
	
	if ($param0=='Exception') {
	    throw new Exception('UnitTest Exception.');
	}

	return array('ok'=>$retval );
}

}

