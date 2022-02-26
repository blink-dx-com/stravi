<?php
// example for testing: csv.substance.lib.xarray.inc

class UT_date_funcs_inc extends gUnitTestSub {
	
function __construct() {

}

// return: 0 : not passed, 1: passed
function dotest( &$sqlo, $options ) {
	global $error;
	
	
	$dateUse = '2010-04-15 13:56';
	$dateUnx = date_str2unix( $dateUse,5);

	$unix_stamp = mktime ( 13, 56, 0, 4, 15, 2010 );
	$ok=0;
	if ($unix_stamp==$dateUnx) $ok=1;
	
	$this->_info( 'date_str2unix(): dateUse', $dateUse. ', equal: '.$ok ); // just info for the test
	$this->_info( 'compare Unix-stamps', 'dateUseUnx: '. $dateUnx. ', reference: '.$unix_stamp);
	
	$inputarr = array('test Excel1 good', '12.03.2013', '2013-03-12');
	$newFormat = date_Excel2YMD($inputarr[1]);
	$testok = 0;
	if ($error->Got(READONLY))  {
		$testok = 0;
		$error->reset();
	} else {
		if ($newFormat==$inputarr[2]) $testok = 1;
	}
	$this->_saveTestResult($inputarr[0], $testok);
	
	$inputarr = array('test Excel1 bad', '5.A.2013', 0);
	$newFormat = date_Excel2YMD($inputarr[1]);
	$testok = 0;
	if ($error->Got(READONLY))  {
		$testok = 1;
		$error->reset();
	} else {
		$testok = 0;
	}
	$this->_saveTestResult($inputarr[0], $testok);
	
	if ($ok) $retval = 1;
	else  $retval = 0;
	
	return ($retval);
}

}
