<?php

class UT_o_CCT_ACC_UP_human_inc extends gUnitTestSub {
	
function __construct() {

}

// return: 0 : not passed, 1: passed
function dotest( &$sql, $options ) {
	$invalue = "<d:ass t:SPOT_RESULT pr:2,567><a:mod>";
	
	$tmplib = new oCCT_ACC_UP_hum() ;
	$result = $tmplib->extract($invalue);
	$this->_infoNow( 'Result', htmlspecialchars(print_r($result,1)) );
	
	
	
	$retval = 0;
	if (is_array($result)) $retval = 1;
	
	return ($retval);
}

}
 