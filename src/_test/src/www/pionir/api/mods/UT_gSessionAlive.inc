<?php 
require __DIR__ . '/../../../../../../vendor/autoload.php';
use Datto\JsonRpc\Http\Client;
use Datto\JsonRpc\Http\HttpException;
use Datto\JsonRpc\Http\HttpResponse;
use Datto\JsonRpc\Response;
// use HttpException;

require_once dirname(__FILE__).'/../../../../../misc/jsonrpc_help.inc';


class UT_gSessionAlive_inc extends \gUnitTestSub {
    
    function __construct() {
        $this->module_noPreLoad =1; 
       
    }

    // return: 0 : not passed, 1: passed
    function dotest( &$sqlo, $options ) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        
        $jsonrpc_config = $this->_get_test_configval('jsonrpc');
        if (!is_array($jsonrpc_config)) {
            $error->set( $FUNCNAME, 1, 'config-data missing for jsonrpc.' );
            return;
        }
        $this->_infoNow( 'Connect-Info',  'URL: '.$jsonrpc_config['url'].' DBID:'.$jsonrpc_config['dbid'] .' USER:'.$jsonrpc_config['user'] );

        
        // Send your requests to the remote server:
        try {
  
            $jsonrph_help_lib = new \jsonrpc_UT_help();
            $rpc_session = $jsonrph_help_lib->login($jsonrpc_config);
            if ($rpc_session==NULL) {
                throw new Exception('(41) LOGIN failed.');
            }

            $clientSession = new \UT_SessionClient($jsonrpc_config['url'], $rpc_session);
            $argu=array();
            $dataout=NULL;
            $clientSession->query('gSessionAlive', $argu, $dataout );
            $clientSession->send();
            
            $test_result_flag=0;
            $tmp_result = $dataout['data']['exists'];
            if ( $tmp_result>0) {
                $test_result_flag=1;
            }
            $this->_saveTestResult('TEST01', $test_result_flag, 'Expect: session valid: '.$tmp_result );
            
            // invalid session ...
            $rpc_session='UTFANTASY001';
            $clientSession = new \UT_SessionClient($jsonrpc_config['url'], $rpc_session);
            $argu=array();
            $clientSession->query('gSessionAlive', $argu, $dataout );
            $clientSession->send();
            $test_result_flag=0;
            $tmp_result = $dataout['data']['exists'];
            if ( !$tmp_result) {
                $test_result_flag=1;
            }
            $this->_saveTestResult('TEST02', $test_result_flag, 'Expect: session invalid: '.$tmp_result );
            
            
            
        } catch (Exception $exception) {
            $message = $exception->getMessage();
            $error->set( $FUNCNAME, 3, 'RPCQuery: '.$message );
            return;
        }
        
        return 1;
    }
    
}