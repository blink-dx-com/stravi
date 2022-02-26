<?php
// example for testing: csv.substance.lib.xarray.inc

class UT_f_upload_inc extends gUnitTestSub {
	
function __construct() {

}

// return: 0 : not passed, 1: passed
function dotest( &$sqlo, $options ) {
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	$test_data_path = $this->_getTestDir_path();
	
	$testLib = new uploadC();
	
	// EMPTY ZIP-archive
	$testfilename = $test_data_path. '/phplib/UT_f.upload.inc.1.zip';
	$answer = $testLib->testZipFile( $testfilename );
	
	if ($error->Got(READONLY))  {
		$errLast = $error->getLast();
		$error_txt = $errLast->text;
		$error->reset();
		$result = 0;
		$this->_saveTestResult('TEST1', 0, 'ERROR: '.$error_txt);
	} else {
		$tmpres = 0;
		if ($answer==0) $tmpres = 1;
		$this->_saveTestResult('TEST1', $tmpres, 'ZIP contains valid files? '.$answer );
	}
	
	
	// one file
	$testfilename = $test_data_path. '/phplib/UT_f.upload.inc.2.zip';
	$answer = $testLib->getZipfileInfo( $testfilename );
	
	if ($error->Got(READONLY))  {
		$errLast = $error->getLast();
		$error_txt = $errLast->text;
		$error->reset();
		$result = 0;
		$this->_saveTestResult('TEST2', $result, 'ERROR: '.$error_txt);
	} else {
		
		if ($answer['numfiles']!=1) {
			$this->_saveTestResult('TEST2_1', 0, 'Expected one file.');
		} else {
			$this->_saveTestResult('TEST2_1', 1, 'NUM:'.$answer['numfiles']);
		}
		
		$tmp_expect = 17;
		if  ( $answer['sizeall']!=$tmp_expect ) {
			$this->_saveTestResult('TEST2_1', 0, 'Expected size files:'. $tmp_expect);
		} else {
			$this->_saveTestResult('TEST2_1', 1, 'SIZE:'.$answer['sizeall']);
		}
		
	}
	
	// three file
	$testfilename = $test_data_path. '/phplib/UT_f.upload.inc.3.zip';
	$answer = $testLib->getZipfileInfo( $testfilename );
	
	if ($error->Got(READONLY))  {
		$errLast = $error->getLast();
		$error_txt = $errLast->text;
		$error->reset();
		$result = 0;
		$this->_saveTestResult('TEST3', $result, 'ERROR: '.$error_txt);
	} else {
	
		if ($answer['numfiles']!=3) {
			$this->_saveTestResult('TEST3_1', 0, 'Expected three files.');
		} else {
			$this->_saveTestResult('TEST3_1', 1, 'NUM:'.$answer['numfiles']);
		}
	
		$tmp_expect = 66;
		if  ( $answer['sizeall']!=$tmp_expect ) {
			$this->_saveTestResult('TEST3_1', 0, 'Expected size files:'. $tmp_expect);
		} else {
			$this->_saveTestResult('TEST3_1', 1, 'SIZE:'.$answer['sizeall']);
		}
	
	}
	
	
	
	$retval = 1;
	
	return ($retval);
}

}
