<?php
// example for testing: csv.substance.lib.xarray.inc

class UT_o_ABS_SOCKET_mod_inc extends gUnitTestSub {
	
function __construct() {
	$this->module_noPreLoad =0;  // 0,1 - do NOT preload ? can be set in constructor of class
	$this->GUI_test_flag    =0;
}

// return: 0 : not passed, 1: passed
function dotest( &$sqlo, $options ) {

    $abs_socket_id = $this->_getDefObjID( "ABS_SOCKET", "default" );
    $sua_id_ASR    = $this->_getDefObjID( "ABSTRACT_SUBST", "class_ASR" );

    $this->_infoNow( 'abs_socket_id', $abs_socket_id.' SUA:'.$sua_id_ASR);
    
    $mod_lib = new oABS_SOCKET_mod();
    $mod_lib->set_obj($sqlo, $abs_socket_id);
    $mod_lib->update_one_pos($sqlo, 1, $sua_id_ASR, 'REAG');
	
	
	
	$result=1;
	$this->_saveTestResult('ABS_SOCKET update', $result); 
	
	
	return 1;
}

}
