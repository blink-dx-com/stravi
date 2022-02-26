<?php
// example for testing: 

class UT_f_zip_instr_upload_inc extends gUnitTestSub {
	
function __construct() {
	$this->module_noPreLoad =0;  // 0,1 - do NOT preload ? can be set in constructor of class
	$this->GUI_test_flag    =0;
}

/**
 * 
 * @param unknown $testname
 * @param unknown $filename
 * @param unknown $expectResult
 *   'error' => 0
 *      >0 : error-code
 *   'windows_path'
 */
function _checkFileName($testname, $filename, $expectResult) {
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	$error_id  = 0;
	$error_txt = NULL;
	/**
	$result = array('server_path', 'windows_path')
	 */
	$result = $this->_testLib->check_filename($filename);
	if ($error->Got(READONLY))  {
		$errLast   = $error->getLast();
		$error_txt = $errLast->text;
		$error_id  = $errLast->id;
		
		$error->reset();
	}
	
	if ($expectResult['error']!=$error_id) {
		$this->_saveTestResult($testname, 0, 'unexpected error; result-error:'.$error_id. ' ('.$error_txt.') expect:'.$expectResult['error']); 
		return;
	}
	
	$resval=0;
	if ($expectResult['windows_path']==$result['windows_path']) {
		$resval=1;
	}
	
	$this->_saveTestResult($testname, $resval, 'expect_error: '.$expectResult['error'].';result[windows_path]:'.$result['windows_path']);
	
}

// return: 0 : not passed, 1: passed
function dotest( &$sqlo, $options ) {
    
    $this->_saveTest_def_later('TEST01');
    
    return 1;


	$this->_testLib = new f_zip_instr_upload('NGAi');
	
	$this->_checkFileName('1_normal', '2018-02-09_132709_1-1_01-00_0640300050.zip', 
			array(
				'error'=>0, 
				'windows_path'=>'Z:\Forschung\NGAI\2018\generated\2018-02-09\2018-02-09_132709_1-1_01-00_0640300050'
					)
		);
	$this->_checkFileName('2_normal', '20180209_132709_1-1_01-00_0640300050.zip',
			array(
					'error'=>0,
					'windows_path'=>'Z:\Forschung\NGAI\2018\generated\2018-02-09\20180209_132709_1-1_01-00_0640300050'
			)
	);
	// bad dateformat
	$this->_checkFileName('3_errorDateFormat', '2018-02-31_132709_1-1_01-00_0640300050.zip',
			array(
					'error'=>4,
					'windows_path'=>''
			)
	);
	// bad dateformat
	$this->_checkFileName('4_errorDateFormat', '20180231_132709_1-1_01-00_0640300050.zip',
			array(
					'error'=>4,
					'windows_path'=>''
			)
	);
	// bad dateformat
	$this->_checkFileName('5_errorDateFormat', '2018-02-X5_132709_1-1_01-00_0640300050.zip',
			array(
					'error'=>1,
					'windows_path'=>''
			)
	);
	// bad dateformat
	$this->_checkFileName('6_errorDateFormat', '201802X1_132709_1-1_01-00_0640300050.zip',
			array(
					'error'=>1,
					'windows_path'=>''
			)
	);
	
	
	
	$retval = 1;
	return ($retval);
}

}
