<?php
// example for testing: csv.substance.lib.xarray.inc

class UT_f_math_inc extends gUnitTestSub {
	
	function __construct() {

	}
	
	
	// return: 0 : not passed, 1: passed
	function dotest( &$sqlo, $options ) {
		$retval=1;
		
		
		$data = array(1,2,2,3);
		$stdDev = fMathC::standard_deviation ( $data );
		if ($stdDev<0.816 or $stdDev>0.817) $retval=0;
		$this->_infoNow( 'stdDev: '.$stdDev,$retval);
		
		
		return ($retval);
	}

}
