<?php


class UT_o_proj_profile_inc extends gUnitTestSub {
	
function __construct() {
	$this->module_noPreLoad =0;  // 0,1 - do NOT preload ? can be set in constructor of class
	$this->GUI_test_flag    =0;
}

// return: 0 : not passed, 1: passed
function dotest( &$sqlo, $options ) {

	$user_id = $this->_getDefObjID( "DB_USER", "default" );
	$this->_infoNow( 'Test-USER:', $user_id ); 

	$testLib = new profile_funcs();
	$proj_id = $testLib->getProj($sqlo, 'other','READONLY', $user_id);
	$log = $testLib->get_log();
	$this->_infoNow( '01:', $proj_id . 'INFO:'.print_r($log,1)); 
	
	$testLib = new profile_funcs();
	$proj_id = $testLib->getProj($sqlo, 'bookmarks','READONLY', $user_id);
	$log = $testLib->get_log();
	$this->_infoNow( '02:', $proj_id . 'INFO:'.print_r($log,1)); 
	
	$testLib = new profile_funcs();
	$proj_id = $testLib->getProj($sqlo, 'bookmarks','CREATE', $user_id);
	$log = $testLib->get_log();
	$this->_infoNow( '03:', $proj_id . 'INFO:'.print_r($log,1)); 
	
	$testLib = new profile_funcs();
	$proj_id = $testLib->getProj($sqlo, 'bookmarks','CREATE', $user_id);
	$log = $testLib->get_log();
	$this->_infoNow( '04:', $proj_id . 'INFO:'.print_r($log,1)); 
	
	

	
	return (1);
}

}
