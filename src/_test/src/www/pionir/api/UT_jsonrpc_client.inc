<?php
/**
 * test the JSONRPC interface ...
 */
class UT_jsonrpc_client_inc extends \gUnitTestSub {
	
function __construct() {
	$this->module_noPreLoad =1; 
	$this->GUI_test_flag    =0;
}

// return: 0 : not passed, 1: passed
function dotest( &$sqlo, $options ) {
    global $error;
    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
    
	
    require_once  ( $_SESSION['s_sessVars']['loginPATH'].'/api/jsonrpc_client.inc');
    
    $jsonrpc_config = $this->_get_test_configval('jsonrpc');
    if (!is_array($jsonrpc_config)) {
        $error->set( $FUNCNAME, 1, 'config-data missing for jsonrpc.' );
        return;
    }
 
    $rpclib = new goz_rcp_client($jsonrpc_config);

    // Send your requests to the remote server:
    try {
 
        $this->_infoNow( 'Login_INPUT',  'URL: '.$jsonrpc_config['url'].' DBID:'.$jsonrpc_config['dbid'] .' USER:'.$jsonrpc_config['user'] );
        
        $rpclib->connect();
        $test_result_flag=1;
        $this->_saveTestResult('LOGIN', $test_result_flag, '' );
        
        $argu = array('t'=>'EXP', 'id'=>10,'cols'=>array(   'NAME'   )  );
        $dataout = $rpclib->call( 'DEF/gObj_getParams', $argu);
        
        $this->_infoNow( 'CALL_EXP',  print_r($dataout,1));
        $test_result_flag=0;
        if ( is_array($dataout) ) {
            $test_result_flag=1;
        }
        $this->_saveTestResult('EXP_DATA', $test_result_flag, print_r($dataout,1) );
  
        
    } catch (Exception $exception) {
        $mess = $exception->getMessage();
        $error->set( $FUNCNAME, 2, 'JSONRPC-CALL: '. $mess);
        return;
    }
    
    // --- TEST BAD MODULE
    $testid='TEST02';
    $test_result_flag=0;
    $info_str='';
    try {
        $argu = array();
        $dataout  = $rpclib->call( 'DEF/XXX_BAD_MODULE', $argu);
        $info_str = print_r($dataout);
    } catch (Exception $exception) {
        $info_str = $exception->getMessage();
        $test_result_flag=1;
    }
    $this->_saveTestResult($testid, $test_result_flag, $info_str );
    
    // --- TEST BAD LOGIN
    $testid='TEST03';
    $jsonrpc_config['user'] = 'UT_baduser';
    $rpclib = new goz_rcp_client($jsonrpc_config);
    $test_result_flag=0;
    
    try {
        $this->_infoNow( 'Login_INPUT',  'URL: '.$jsonrpc_config['url'].' DBID:'.$jsonrpc_config['dbid'] .' USER:'.$jsonrpc_config['user'] );
        $rpclib->connect();
    } catch (Exception $exception) {
        $test_result_flag=1;  // expected ..
    }
    $this->_saveTestResult($testid, $test_result_flag );
    

	$retval = 1;
	return ($retval);
}

}