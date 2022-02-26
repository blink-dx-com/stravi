<?php


class UT_o_CONCRETE_SUBST_autofini_inc extends gUnitTestSub {
	
function __construct() {

}


/* return: 0 : not passed, 1: passed
 
 */
function dotest( &$sqlo, $options ) {
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	$sqlo2 = &$this->_getSqlo2();
	
	$testlib = new o_CONCRETE_SUBST_autofini();
	$answer = $testlib->autoFinish($sqlo, $sqlo2);
	
	$this->_infoNow('Result', print_r($answer,1));
	
	
	return (1);
}

}
