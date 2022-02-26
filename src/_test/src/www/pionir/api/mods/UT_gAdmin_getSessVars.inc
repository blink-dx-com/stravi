<?php 
// UT_gObj_list.inc
require_once( $_SESSION['s_sessVars']['loginPATH'].'/api/lib/metacall.inc' );

class UT_gAdmin_getSessVars_inc extends gUnitTestSub {
	
function __construct() {
	
	$this->module_noPreLoad=1;
}

// return: 0 : not passed, 1: passed
function dotest( &$sql, $options ) {
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	$args_use=array();
	$metalib = new metacall('DEF/gAdmin_getSessVars', $args_use);
	$answer = $metalib->run();
	$this->_infoNow('data', print_r($answer,1));
	
	
	return 1;
}

}