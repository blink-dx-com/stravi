<?php
/**
 * REST API to the system
 
 * @package rest.php
 * @global <pre>
 * - $globals["xml_rpc.debug_level"] : debug level
 *    1: log call-names
 *    2: log all parameters
 * </pre>
 
 * @unittest exists, see jsonrpc_client.py in the TEST-dir
 * @throws errors, analyses PHP-exceptions
 *   1 : no session-id given
 *   2 : session not active
 *   3 : other errors ...
 */

require_once ('reqnormal.inc');
//require_once("../../../config/config.local.inc");
//require_once("../../../config/config.product.inc");
require_once('lib/metacall.inc');
require_once 'lib/api_debug.inc';
require_once("f.modulLog.inc");





/**
 * Result structure definition
 *  'data'  => array()
 *   'error' => array('num'=>, 'text'=>) 
 * @typedef array jsonrpc_result_STRUCT
 */


class RestEval {

    
    private function api_error( $error_num, $error_txt )
    {
        $modLogLib = new fModulLogC();
        $sqldummy  = NULL;
        $modLogLib->logModul($sqldummy, 'REST: method:'.$this->_method. ' ERR-NO:'.$error_num. ' ERR-TXT:'.$error_txt, 1); // do NOT logging on file
        
        return array( 'error'=>array('num'=>$error_num, 'text'=>$error_txt) );
    }
    

    /**
     * 
     * @param string $method
     * @param array $arguments
     * @return array jsonrpc_result_STRUCT
     */
    public function evaluate($method, $args_use)
    {
        global $error, $globals;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $this->_method = $method;

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
 
           
            if ($method=='gSessionAlive') {
                // check, if session is alive
                session_start();
                $tmp_result=0;
                if ( $_SESSION['sec']['appuser']!=NULL ) $tmp_result=1;
                return array('data'=>array('exists'=>$tmp_result) );
            }
                
            # START session ...
            session_start();
            if( $_SESSION['sec']['appuser']==NULL ) {
                return $this->api_error( 2, 'session not active.' );
               
            }

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
            

            if ($output_debug>0) {
                $logging_lib->log_output($answer);
            }
            
            return $answer;
            
            
        } else {
          return $this->api_error( 3, 'LOGIN not implemented' );
            
        }
  
    }
    
    function reply($answer) {
        
        // required headers
        header("Access-Control-Allow-Origin: *");
        header("Content-Type: application/json; charset=UTF-8");
        header("Access-Control-Allow-Methods: POST");
        header("Access-Control-Max-Age: 3600");
        header("Access-Control-Allow-Headers: Content-Type, Access-Control-Allow-Headers, Authorization, X-Requested-With");
        
        http_response_code(200);
        echo json_encode($answer);
    }
    
    
}

global $error;
$error  = & ErrorHandler::get();

$server  = new RestEval();
$content = file_get_contents('php://input');
if (!strlen($content)) {
    $server->api_error( 1, 'No input.' );
}
$arr_in = json_decode($content, TRUE);

$answer = $server->evaluate($arr_in['mod'], $arr_in);
$server->reply($answer);
