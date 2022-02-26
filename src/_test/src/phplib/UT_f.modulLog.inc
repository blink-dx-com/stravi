<?php
// example for testing: f.modulLog.inc

class UT_f_modulLog_inc extends gUnitTestSub {
	
function __construct() {

}

// return: 0 : not passed, 1: passed
function dotest( &$sqlo, $options ) {


	$testLib = new fModulLogC();
	$answer = $testLib->logModul($sqlo); // test 1
	$this->_info( 'param(): answer', $answer ); // just info for the test
	
	$answer = $testLib->logModul($sqlo, 'META:xxx'); // test 2
	$this->_info( 'param("META:xxx"): answer', $answer ); // just info for the test
	
	$answer = $testLib->logModul($sqlo, 'META:xxx2', 1); // test 2
	$this->_info( 'param("META:xxx2", 1): (NO LOGGING) answer', $answer ); // just info for the test
	
	if ($answer!=NULL) $retval = 1;
	else  $retval = 0;
	
	return ($retval);
}

}
