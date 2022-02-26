<?php
/**
 * JSONRPC client for Gozilla ...
 * new version for PHP7.x (2020-02)
 * @package jsonrpc_client.inc
 */

require __DIR__ . '/../../../vendor/autoload.php';
use Datto\JsonRpc\Http\Client;
use Datto\JsonRpc\Http\HttpException;
use Datto\JsonRpc\Http\HttpResponse;
use Datto\JsonRpc\Response;
use ErrorException;



/**
 * This is an example of an authenticated API:
 * @see https://en.wikipedia.org/wiki/Basic_access_authentication Basic access authentication
 *
 * Of course, you can use any type of authentication that you prefer!
 */
class AuthenticatedClient extends Client
{
    /**
     * Client constructor.
     * @param string $uri
     * @param string $username
     * @param string $password
     */
    public function __construct($uri, $username, $password)
    {
        $authentication = base64_encode("{$username}:{$password}");
        $headers = array(
            'Authorization' => "Basic {$authentication}"
        );
        parent::__construct($uri, $headers);
    }
}

class SessionClient extends Client
{
    /**
     * Client constructor.
     * @param string $sessionid
     */
    public function __construct($uri, $sessionid)
    {
        $headers = array( 
            'X-RPC-Auth-Session' => $sessionid
        );
        parent::__construct($uri, $headers);
    }
}

/**
 * gozilla RPC client
 * @author Steffen
 *
 */
class goz_rcp_client {
    
    /**
     * 
     * @var array $connect_params
     *   'url',
     *   'user',
     *   'pw'
     *   'dbid'
     **/
    private $connect_params;
    
    
    function __construct( $connect_params ) {
        $this->connect_params = $connect_params;
        $check_keys = array('url', 'user', 'pw', 'dbid');
        foreach($check_keys as $key) {
            if ($connect_params[$key]==NULL) {
                throw new Exception('ConnectParam "'.$key.'": no value');
            }
        }
    }
    
    public function connect() {
        
        $this->clientSession = NULL;
        
        try {
            
            
            
            $clientAuth = new \AuthenticatedClient($this->connect_params['url'], $this->connect_params['user'], $this->connect_params['pw']);

            $argu   = array("dbid"=> $this->connect_params['dbid'] );
            $argu2  = array($argu);

            $dataout=NULL;
            $clientAuth->query('login', $argu2, $dataout );

            $clientAuth->send();
            
     
            
            // echo "DDD: after  \n";
            if (array_key_exists('error', $dataout)) {
                if (is_array($dataout['error'])) {
                    // error from JSONRPC handler
                    throw new Exception('ModuleError: '.print_r($dataout['error'],1) );
                }
            }
           
            
            $rpc_session = $dataout['data']['sessionid'];
            if ($rpc_session==NULL) {
                throw new Exception('NoSessionGot: ');
            }

            $this->clientSession = new \SessionClient($this->connect_params['url'], $rpc_session);
            
        } catch (HttpException $exception) {
            $this->onException($exception, 'connect');
            return;
            
        } catch (ErrorException $exception) {
            $message = $exception->getMessage();
            throw new Exception('ConnectRPCQuery: '.$message);
        }
        
    }
    
    /**
     * 
     * @param object $sqlo
     * @param array $params
     * @return array
     */
    function call( $method, $params ) {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $dataout=NULL;

        try {
       
            $result=NULL;
            $this->clientSession->query( $method, array($params), $result ); 
            $this->clientSession->send();

            if (array_key_exists('error', $result)) {
                if (is_array($result['error'])) {
                    // error from JSONRPC handler
                    throw new Exception('ModuleError: '.print_r($result['error'],1) );
                }
            }

            $dataout = &$result['data']; 
            
            // echo "DDD:call: ".print_r($result,1)."<br>\n";
            
            
            
        } catch (HttpException $exception) {           
            $this->onException($exception, 'call:'.$method);
            return;
       
        } catch (ErrorException $exception) {
            throw new Exception('RPCQuery: '.$message);
        }
    
        return ($dataout);
    }
    
    
    
    function onException($exception, $extratext) {
        
        $httpResponse = $exception->getHttpResponse();
        
        $errormsg = "HttpException (".$extratext.")\n".
            " * statusCode: ". $httpResponse->getStatusCode(). "\n".
            " * reason: ". $httpResponse->getReason(). "\n".
            " * headers: ". json_encode($httpResponse->getHeaders()). "\n".
            " * version: ". $httpResponse->getVersion(). "\n";
        throw new Exception($errormsg);
    }
        

}