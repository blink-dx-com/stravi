<?php 
// UT_gObj_list.inc
require_once( $_SESSION['s_sessVars']['loginPATH'].'/api/lib/metacall.inc' );

class UT_gObj_list_inc extends gUnitTestSub {
	
    function __construct() {
    	$this->module_noPreLoad=1;
    }
    
    // return: 0 : not passed, 1: passed
    function dotest( &$sql, $options ) {
    	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
    	
    	$args_use=array(
    	      't'=>'CONCRETE_SUBST',
    	      'cols'=>array( 'x.CONCRETE_SUBST_ID','x.NAME', 'x.NOTES', 'a.MOD_DATE'),
    	      'filter'=>array( 
    	          'cols'=> array(
    	              array('col'=>'x.NAME', 'con'=>'LIKE', 'val'=>'%A%', 'bool'=>'AND') 
    	           ),
    	     ),
    	     'sort'=>'x.NAME',
    	     'pagesize'=>30
    	);
    	$metalib = new metacall('DEF/gObj_list', $args_use);
    	$answer = $metalib->run();
    	$this->_infoNow('data', print_r($answer,1));
    	
    	$args_use=array(
    	    't'=>'CONCRETE_SUBST',
    	    'cols'=>array( 'x.CONCRETE_SUBST_ID','x.NAME', 'x.NOTES', 'a.MOD_DATE'),
    	    'filter'=>array(
    	        'projid'=>11
    	    ),
    	    'sort'=>'x.NAME',
    	    'pagesize'=>40
    	);
    	$metalib = new metacall('DEF/gObj_list', $args_use);
    	$answer = $metalib->run();
    	$this->_infoNow('data_PROJ', print_r($answer,1));
    	
    	return 1;
    }

}