<?php
// example for testing: ErrorHandler.inc

class UT_ErrorHandler_inc extends gUnitTestSub {
	

function _sub_method() {
	
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	$error = & ErrorHandler::get();
	
	$error->set( 'FUNC2', 100, 'Error2' ); // set an error
}

// return: 0 : not passed, 1: passed
function dotest( &$sqlo, $options ) {

	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	$error = & ErrorHandler::get();
	
	$error->reset(); // reset error stack
	
	$error->set( 'FUNC1', 101, 'Error1' );
	
	$this->_sub_method();
	
	$error->set( 'FUNC3', 103, 'Error3' );
	
	$allErArr  = $error->getAllArray(); // error as array
	$num_errors = sizeof($allErArr);
	$error->reset();
	
	$this->_info( 'ErrorStack', print_r($allErArr,1) ); // just info for the test
	
	$ERR_EXPECT=3;
	$result = $num_errors==$ERR_EXPECT ? 1 : 0;
	$infotext="Number of errors: ".$num_errors;
	$this->_saveTestResult('ErrorStack_number', $result, $infotext);
	
	$retval = 1;
	
	return ($retval);
}

}
