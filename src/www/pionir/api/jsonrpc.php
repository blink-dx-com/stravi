<?php
/**
 * JSONRPC API to the system
 * - external interface code for Python etc see GIT:/ext/Python ...
 * 
 * - important methods:
 *   - gSessionAlive
 *   - login
 *   - gGetVersion
 * @package jsonrpc.php
 * @global <pre>
 * - $globals["xml_rpc.debug_level"] : debug level
 *    1: log call-names
 *    2: log all parameters
 * </pre>
 * @install need from COMPOSER:  https://github.com/datto/php-json-rpc-http
 
 * @unittest exists, see jsonrpc_client.py in the TEST-dir
 * @throws errors, analyses PHP-exceptions
 *   1 : no session-id given
 *   2 : session not active
 *   3 : other errors ...
 */

require_once ('reqnormal.inc');
require_once("../../../config/config.local.inc");
require_once("../../../config/config.product.inc");
require_once 'lib/Authenticate.inc';
require_once('lib/metacall.inc');
require_once 'lib/api_debug.inc';
require_once("f.modulLog.inc");

require __DIR__ . '/../../../vendor/autoload.php';

global $error, $varcol;


/**
 * Result structure definition
 *  'data'  => array()
 *   'error' => array('num'=>, 'text'=>) 
 * @typedef array jsonrpc_result_STRUCT
 */
$jsonrpc_result_STRUCT=NULL; 


use Datto\JsonRpc\Http\Server;

use Datto\JsonRpc;
use Datto\JsonRpc\Exceptions\ArgumentException;
use Datto\JsonRpc\Exceptions\MethodException;

class Evaluator implements JsonRpc\Evaluator
{
    
    private function api_error( $error_num, $error_txt )
    {
        $modLogLib = new fModulLogC();
        $sqldummy  = NULL;
        $modLogLib->logModul($sqldummy, 'JSONRPC: method:'.$this->_method. ' ERR-NO:'.$error_num. ' ERR-TXT:'.$error_txt, 1); // do NOT logging on file
        
        return array( 'error'=>array('num'=>$error_num, 'text'=>$error_txt) );
    }
    

    /**
     * 
     * @param unknown $method
     * @param unknown $arguments
     * @throws MethodException
     * @return jsonrpc_result_STRUCT
     */
    public function evaluate($method, $arguments)
    {
        global $error, $globals;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $this->_method = $method;
        
        $jsonrpc_lib = new Authenticate();
        $args_use = $arguments[0];
        
        $output_debug = $globals["xml_rpc.debug_level"]; 
        $logging_lib = new api_debug($output_debug);
        
        if ($method!='login') {
          
           
            
            if ($method=='gGetVersion') {
                return array('data'=>array('version'=>'1.0') );
            }

            // gGetVersion does not need a session !
            if ($output_debug>0) {
                $logging_lib->log_input($method, $args_use);
            }
            
            
            $sessionid = $_SERVER['HTTP_X_RPC_AUTH_SESSION'];
            if ($sessionid==NULL) {
                return $this->api_error( 1, "No sessionid given." );
            }
            
            if ($method=='gSessionAlive') {
                // check, if session is alive
                session_id($sessionid);
                session_start();
                $tmp_result=0;
                if ( $_SESSION['sec']['appuser']!=NULL ) $tmp_result=1;
                return array('data'=>array('exists'=>$tmp_result) );
            }
                
            $session_result = $jsonrpc_lib->startSession($sessionid, $method);
            if ($session_result[0]<1) { 
                return $this->api_error( 2, $session_result[1] );
            }
            
            //             $prio =3;
            //             $MODULE = $_SERVER ['PHP_SELF'];
            //             $logerrTxt = "INDATA: METHOD:".$MODULE.' DATAIN:'.print_r($args_use,1);
            //             $error->logxMeta( "INFO", $prio , $FUNCNAME , $MODULE , $logerrTxt );
            
            // META call
            $err_flag = 0;
            $err_mess = '';
            try {
   
                $metalib = new metacall($method, $args_use);
                $answer = $metalib->run();
 
            } catch (Exception $e) {
                $err_mess = $e->getMessage();
                $err_flag = 1;
            }

            if ($err_flag) {
                return $this->api_error( 3, $err_mess);
            }
            
            //             $prio =3;
            //             $MODULE = $_SERVER ['PHP_SELF'];
            //             $logerrTxt = "OUTDATA: METHOD:".$MODULE.' DATAOUT:'.print_r($answer,1);
            //             $error->logxMeta( "INFO", $prio , $FUNCNAME , $MODULE , $logerrTxt );
            
            if ($output_debug>0) {
                $logging_lib->log_output($answer);
            }
            
            return $answer;
            
            
        } else {
            
            //return $this->api_error( 999, 'DDDEBUG:'.print_r($arguments,1) );
            
            // login ...
            $answer = $jsonrpc_lib->checklogin($args_use);
            
            $modLogLib = new fModulLogC();
            $sqldummy  = NULL;
            $modLogLib->logModul($sqldummy, 'JSONRPC:'.$method, 0);
            
            if ($error->Got(READONLY))  {
                
                $errLast   = $error->getLast();
                $error_txt = $errLast->text;
                $error_id  = $errLast->id;
                $origin_id = $errLast->origin;
                
                return $this->api_error( 3, $error_txt. ' ('.$error_id.':'.$origin_id.')' );
            }
            
            return array('data'=>$answer);
            
        }
  
    }
    
    
}

$error  = & ErrorHandler::get();
// $varcol = & Varcols::get();


$evaluator = new Evaluator();
$server    = new Server($evaluator);


$server->reply();
