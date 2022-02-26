<?php 
require_once( $_SESSION['s_sessVars']['loginPATH'].'/api/lib/metacall.inc' );

class UT_oEXP_TMPL_get_pra_det_inc extends gUnitTestSub {
	
    function __construct() {
    	$this->module_noPreLoad=1;
    }
    
    // return: 0 : not passed, 1: passed
    function dotest( &$sql, $options ) {
    	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
    	
    	$exp_tmpl_id = $this->_getDefObjID('EXP_TMPL', 'toolbox');
    	$exp_tmpl_id=31;
    	
    	$this->_infoNow('EXP_TMPL_ID', $exp_tmpl_id);
    	
    	$args_use=array(
    	    'id'  => $exp_tmpl_id, 
    	    'kind'=> 'Experiment protocol' 
    	);
    	$metalib = new metacall('DEF/oEXP_TMPL_get_pra_det', $args_use);
    	$answer  = $metalib->run();
    	$this->_infoNow('data', print_r($answer,1));
    	
    	$result=0;
    	if (is_array($answer['data'])) $result=1;
    	
    	$this->_saveTestResult('TEST01', $result, 'exp_tmpl has data');
    	
    	
    	
    	return 1;
    }

}