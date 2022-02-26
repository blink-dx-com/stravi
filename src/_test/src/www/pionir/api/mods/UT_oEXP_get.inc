<?php 
// UT_gObj_list.inc
require_once( $_SESSION['s_sessVars']['loginPATH'].'/api/lib/metacall.inc' );

class UT_oEXP_get_inc extends gUnitTestSub {
	
function __construct() {
	
	$this->module_noPreLoad=1;
}

// return: 0 : not passed, 1: passed
function dotest( &$sql, $options ) {
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	$exp_id = $this->_getDefObjID('EXP', 'default');
	
	$args_use=array(
	    'id'   => $exp_id, 
	    'types'=> array('proto.all'=>1)
	);
	$metalib = new metacall('DEF/oEXP_get', $args_use);
	$answer  = $metalib->run();
	$this->_infoNow('data', print_r($answer,1));
	
	$result=0;
	if (is_array($answer['data'])) $result=1;
	
	$this->_saveTestResult('TEST01', $result, 'experiment has data');
	
	
	
	return 1;
}

}