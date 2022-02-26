<?php 
require_once  ( $_SESSION['s_sessVars']['loginPATH'].'/api/jsonrpc_client.inc');

class UT_gLogout_inc extends gUnitTestSub {
	
    function __construct() {
    	$this->module_noPreLoad=1;
    }
    
    // return: 0 : not passed, 1: passed
    function dotest( &$sql, $options ) {
        global $error;
    	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
    	

    	$jsonrpc_config = $this->_get_test_configval('jsonrpc');
    	if (!is_array($jsonrpc_config)) {
    	    $error->set( $FUNCNAME, 1, 'config-data missing for jsonrpc.' );
    	    return;
    	}
    	
    	$rpclib = new goz_rcp_client($jsonrpc_config);
    	
    	try {
    	    
    	    $this->_infoNow( 'Login_INPUT',  'URL: '.$jsonrpc_config['url'].' DBID:'.$jsonrpc_config['dbid'] .' USER:'.$jsonrpc_config['user'] );
    	    
    	    $rpclib->connect();
    	    $test_result_flag=1;
    	    $this->_saveTestResult('LOGIN', $test_result_flag, '' );
    	    
    	    $argu=array();
    	    $this->_infoNow( 'START:gSessionAlive', '');
    	    $dataout = $rpclib->call( 'gSessionAlive', $argu);
    	    $this->_infoNow( 'OUT:gSessionAlive',  print_r($dataout,1));
    	    
    	    $argu = array();
    	    $dataout = $rpclib->call( 'DEF/gLogout', $argu);
    	    $this->_infoNow( 'CALL:OUTPUT',  print_r($dataout,1));
    	    $test_result_flag=0;
    	    if ( is_array($dataout) ) {
    	        $test_result_flag=1;
    	    }
    	    $this->_saveTestResult('TEST01', $test_result_flag, print_r($dataout,1) );
    	    
    	   
    	    
    	} catch (Exception $exception) {
    	    $mess = $exception->getMessage();
    	    $error->set( $FUNCNAME, 2, 'JSONRPC-CALL failed: '. $mess);
    	    return; // must stop here 
    	}
    	
    	$err_mess='';
    	try {
    	    
    	    // make 
    	    $argu=array(
    	        't'=>'CONCRETE_SUBST',
    	        'cols'=>array( 'x.CONCRETE_SUBST_ID','x.NAME', 'x.NOTES', 'a.MOD_DATE'),
    	        'filter'=>array(
    	            'cols'=> array(
    	                array('col'=>'x.NAME', 'con'=>'LIKE', 'val'=>'%XXXXXX%', 'bool'=>'AND')
    	            ),
    	        ),
    	        'sort'=>'x.NAME',
    	        'pagesize'=>30
    	    );
    	    
    	    $dataout = $rpclib->call( 'DEF/gObj_list', $argu);
    	    $this->_infoNow( 'CALL:OUTPUT',  print_r($dataout,1));
    	    $test_result_flag=0;
    	    
    	} catch (Exception $exception) {
    	    $err_mess = $exception->getMessage();
    	    $test_result_flag=1;
    	    // expected error ...
    	}
   
    	$this->_saveTestResult('TEST02', $test_result_flag, $err_mess );
    	    
    	    
    	
    	
    	return 1;
    }

}