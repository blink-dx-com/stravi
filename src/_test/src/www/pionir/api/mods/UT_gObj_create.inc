<?php 
// UT_gObj_list.inc
require_once( $_SESSION['s_sessVars']['loginPATH'].'/api/lib/metacall.inc' );

class UT_gObj_create_inc extends gUnitTestSub {
	
function __construct() {
	
	$this->module_noPreLoad=1;
}

function _subcall($args_use) {
    
    $this->_infoNow('Input', print_r($args_use,1));
    $metalib = new metacall('DEF/gObj_create', $args_use);
    $answer = $metalib->run();
    $this->_infoNow('Output', print_r($answer,1));
    
    return $answer;
}

// return: 0 : not passed, 1: passed
function dotest( &$sql, $options ) {
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	$doc_name = __CLASS__;
	$proj_id = $this->_getDefObjID( "PROJ", "default" );
	
	$args_use = array(
	    't'	=> 'LINK',
	    'args' => array(
	        'vals'  => array(
	            'NAME' => $doc_name
	        )
	      ),
	    'proj' => $proj_id
	);
	
	$this->_subcall($args_use);
	$result = 1;
	$this->_saveTestResult('TEST01', $result);
	
	$args_use = array(
	    't'	=> 'LINK',
	    'args' => array(
	        'vals'  => array(
	            'NAME' => $doc_name
	        )
	    ),
	    'proj' => 9999999999
	);
	
	$this->_subcall($args_use);
	$result = 1;
	$this->_saveTestResult('TEST02_err', $result);
	
	
	
	return 1;
}

}