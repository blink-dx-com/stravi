<?php
/**
 * test the JSONRPC interface ...
 */

require __DIR__ . '/../../../../../vendor/autoload.php';
use Datto\JsonRpc\Http\Client;
use Datto\JsonRpc\Http\HttpException;
use Datto\JsonRpc\Http\HttpResponse;
use Datto\JsonRpc\Response;

require_once dirname(__FILE__).'/../../../../misc/jsonrpc_help.inc';



class UT_jsonrpc_php extends \gUnitTestSub {
	
function __construct() {
	$this->module_noPreLoad =1;  // 0,1 - do NOT preload ? can be set in constructor of class
	$this->GUI_test_flag    =0;
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
 
    $client = new Client($jsonrpc_config['url']);
    
    $dataout=NULL;
    $client->query('gGetVersion', array(1, 2), $dataout);
    
    // Send your requests to the remote server:
    try {
        
        $client->send();
        //$dataout = $this->onSuccess($responses);
        $test_result_flag=0;
        $tmp_vers = $dataout['data']['version'];
        if ( $tmp_vers!=NULL) {
            $test_result_flag=1;
        }
        $this->_saveTestResult('VERSION', $test_result_flag, $tmp_vers );
        
        
        $clientAuth = new \UT_AuthenticatedClient($jsonrpc_config['url'], $jsonrpc_config['user'], $jsonrpc_config['pw']);
        $argu   = array("dbid"=> $jsonrpc_config['dbid'] );
        $clientAuth->query('login', array($argu), $dataout );
        $clientAuth->send();
        //$dataout = $this->onSuccess($responses);
        $rpc_session = $dataout['data']['sessionid'];
        $test_result_flag=0;
        if ($rpc_session!=NULL) {
            $test_result_flag=1;
        }
        $this->_saveTestResult('TEST01_LOGIN', $test_result_flag, 'Session:'.$rpc_session );
        
        // $this->_infoNow( 'Login-Result', print_r($dataout,1) );
        
        $clientSession = new \UT_SessionClient($jsonrpc_config['url'], $rpc_session);
        $clientSession->query('DEF/gObj_getParams', array(array('t'=>'EXP', 'id'=>10,'cols'=>array('NAME'))), $dataout); 
        $clientSession->send();
        //$dataout = $this->onSuccess($responses);
        
        $test_result_flag=0;
        if ( is_array($dataout['data']) ) {
            $test_result_flag=1;
        }
        $this->_saveTestResult('TEST02', $test_result_flag, print_r($dataout,1) );
        
        // second call
        $clientSession->query('DEF/gObj_getParams', array(array('t'=>'EXP', 'id'=>15,'cols'=>array('NAME'))), $dataout);
        $clientSession->send();
        //$dataout = $this->onSuccess($responses);
        $test_result_flag=0;
        if ( is_array($dataout['data']) ) {
            $test_result_flag=1;
        }
        $this->_saveTestResult('TEST02_02', $test_result_flag, print_r($dataout,1) );
        
        // provoke error: no session
       

        $invalid_session='xxx-session-invalid';
        $clientSession->set_session_id($invalid_session);
        $clientSession->query('DEF/gObj_getParams', array(array('t'=>'EXP', 'id'=>10,'cols'=>array('NAME'))), $dataout);
        $clientSession->send();
        //$dataout   = $this->onSuccess($responses);
        $test_result_flag=0;
        if ( is_array($dataout['error']) and  $dataout['error']['num']>0) {
            $test_result_flag=1;
        }
        $this->_saveTestResult('TEST03_error', $test_result_flag, print_r($dataout,1) );
        
        // provoke error: wrong input
        
       
        $this->_infoNow( 'Session_ID again',  $rpc_session);
        $clientSession->set_session_id( $rpc_session );  // set the right one
        $clientSession->query('DEF/gObj_getParams', array(array('t'=>'XXX', 'id'=>10,'cols'=>array('NAME'))), $dataout);
        $clientSession->send();
        //$dataout = $this->onSuccess($responses);
        
        $test_result_flag=0;
        if ( is_array($dataout['error']) and  $dataout['error']['num']>0) {
            $test_result_flag=1;
        }
        $this->_saveTestResult('TEST04_error', $test_result_flag, print_r($dataout,1) );
        
        // TEST05_error
        
        $this->_infoNow( 'Session_ID again',  $rpc_session);
        $clientSession->set_session_id( $rpc_session );  // set the right one
        $clientSession->query( 'DEF/gUT_test', array(array('method'=>'Exception')), $dataout );
        $clientSession->send();
        //$dataout = $this->onSuccess($responses);
        
        $test_result_flag=0;
        if ( is_array($dataout['error']) and  $dataout['error']['num']>0) {
            $test_result_flag=1;
        }
        $this->_saveTestResult('TEST05_error', $test_result_flag, print_r($dataout,1) );
        
        
        $clientSession->query( 'DEF/gAdmin_getSessVars', array(), $dataout );
        $clientSession->send();
        //$dataout = $this->onSuccess($responses);
        
        $this->_infoNow( 'Session vars',  print_r($dataout,1));
        
        $test_result_flag=1;
        
        $this->_saveTestResult('TEST05_error', $test_result_flag, '' );
        
        
    } catch (HttpException $exception) {
        $httpResponse = $exception->getHttpResponse();
        
        $errormsg = "HttpException\n".
            " * statusCode: ". $httpResponse->getStatusCode(). "\n".
            " * reason: ". $httpResponse->getReason(). "\n".
            " * headers: ". json_encode($httpResponse->getHeaders()). "\n".
            " * version: ". $httpResponse->getVersion(). "\n";
        
        $this->_infoNow( 'ERROR',  str_replace("\n","<br>", $errormsg));
        
        $error->set( $FUNCNAME, 2, 'HttpException: '.$errormsg );
        return;
   
    } catch (ErrorException $exception) {
        $message = $exception->getMessage();
        $error->set( $FUNCNAME, 3, 'RPCQuery: '.$message );
        return;
    }
    
	
    
	
	$retval = 1;
	
	return ($retval);
}

function onSuccess(array $responses) {
    global $error;
    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
    
    /**
     * @var Response[] $responses
     */
    foreach ($responses as $response) {
        $id = $response->getId();
        echo "id: {$id}\n";
        if ($response->isError()) {
            $error = $response->getError();
            $code = $error->getCode();
            $message = $error->getMessage();
            $data = $error->getData();
            $errormsg = 
                "    code: ". json_encode($code). "\n".
                "    message: ". json_encode($message). "\n".
                "    data (if any): ". json_encode($data). "\n";
            $error->set( $FUNCNAME, 1, $errormsg );
            return;
        } else {
            $result = $response->getResult();
            return $result;
        }
        
    }
}




}