<?php 
// UT_gObj_list.inc
require_once( $_SESSION['s_sessVars']['loginPATH'].'/api/lib/metacall.inc' );

class UT_oPROJ_getObjects_inc extends gUnitTestSub {
	
function __construct() {
	
	$this->module_noPreLoad=1;
}

function _call_one($testid, $args_use) {
    
    $this->_infoNow( $testid.':Input', print_r($args_use,1));
    $metalib = new metacall('DEF/oPROJ_getObjects', $args_use);
    $answer = $metalib->run();
    
    $this->_infoNow( $testid.':Output', print_r($answer,1));
    
    return $answer;
}

// return: 0 : not passed, 1: passed
function dotest( &$sql, $options ) {
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	$objid = $this->_getDefObjID( "PROJ", "default" );
	
	$this->_infoNow('UT_Proj_ID:', $objid);
	
	$testid='TEST01_root_project';
	$args_use = array(
	    'id'   => 0, 
	    'tables'=> array(),
	    'getpath' => 1
	);
	$answer = $this->_call_one($testid, $args_use);
	
	$result=0;
	if (is_array($answer['data'])) $result=1;
	$this->_saveTestResult($testid.':1', $result);
	
	
	$data = &$answer['data'];
	if ( !is_array( $data['objects']['PROJ_ORI']) ) {
	    $this->_saveTestResult($testid.':2', 0, 'No sub-projects');
	}
	
	$testid='TEST02_project';
	$args_use=array(
	    'id'    => $objid,
	    'tables'=> array('PROJ_ORI'),
	    'getpath' => 1 
	);
	$answer = $this->_call_one($testid, $args_use);
	
	$result=0;
	if (is_array($answer['data'])) $result=1;
	$this->_saveTestResult($testid.':1', $result);
	$data = &$answer['data'];
	if ( !is_array( $data['path']) ) {
	    $this->_saveTestResult($testid.':2', 0, 'No path data');
	}
	
	$testid='TEST03_project';
	$args_use=array(
	    'id'    => $objid,
	    'tables'=> array('ABSTRACT_SUBST'),
	    'getpath' => 1
	);
	$answer = $this->_call_one($testid, $args_use);
	
	$result=0;
	if (is_array($answer['data'])) $result=1;
	$this->_saveTestResult($testid.':1', $result);
	$data = &$answer['data'];
	if ( !is_array( $data['path']) ) {
	    $this->_saveTestResult($testid.':2', 0, 'No path data');
	}
	if ( !is_array( $data['objects']['ABSTRACT_SUBST']) ) {
	    $this->_saveTestResult($testid.':3', 0, 'No ABSTRACT_SUBST data');
	}
	
	$testid='TEST04_error';
	$args_use=array(
	    'id'   => NULL,
	);
	$answer = $this->_call_one($testid, $args_use);
	
	$result=0;
	if ($answer['error']['num']>0) $result=1;
	$this->_saveTestResult($testid, $result );
	
	$testid='TEST05_error';
	$args_use=array(
	    'id'   => 'jsjsj',
	);
	$answer = $this->_call_one($testid, $args_use);
	
	$result=0;
	if ($answer['error']['num']>0) $result=1;
	$this->_saveTestResult( $testid, $result );
	
	$testid='TEST06_error';
	$args_use=array(
	    'id'   => 99999999999,
	);
	$answer = $this->_call_one($testid, $args_use);
	
	$result=0;
	if ($answer['error']['num']>0) $result=1;
	$this->_saveTestResult( $testid, $result );
	
	$testid='TEST07_err_table';
	$args_use=array(
	    'id'   => $objid,
	    'tables'=> array('BAD_Table'),
	    'getpath' => 1
	);
	$answer = $this->_call_one($testid, $args_use);
	
	$result=0;
	if ($answer['error']['num']>0) $result=1;
	$this->_saveTestResult( $testid, $result );
	
	
	// get PROJs
	$testid='TEST08_project';
	$args_use=array(
	    'id'   => 0,
	    'tables'=> array('PROJ_ORI'),
	    'getpath' => 1
	);
	$answer = $this->_call_one($testid, $args_use);
	$result=0;
	if (is_array($answer['data']['objects']['PROJ_ORI'])) $result=1;
	$this->_saveTestResult($testid.':1', $result);
	
	
	return 1;
}

}