<?php
/**
 *  @package jsonrpc_help.inc
 */
require __DIR__ . '/../../vendor/autoload.php';
use Datto\JsonRpc\Http\Client;
use Datto\JsonRpc\Http\HttpException;
use Datto\JsonRpc\Http\HttpResponse;
use Datto\JsonRpc\Response;


/**
 * This is an example of an authenticated API: GITHUB: datto/php-json-rpc-http
 * @see https://en.wikipedia.org/wiki/Basic_access_authentication Basic access authentication
 *
 * Of course, you can use any type of authentication that you prefer!
 */
class UT_AuthenticatedClient extends Client
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

class UT_SessionClient extends Client
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
    
    function set_session_id($session_id) {
        parent::setHeader('X-RPC-Auth-Session', $session_id);
    }
}


class jsonrpc_UT_help {
    
    function login($jsonrpc_config) {
        
        $clientAuth = new \UT_AuthenticatedClient($jsonrpc_config['url'], $jsonrpc_config['user'], $jsonrpc_config['pw']);
        $argu  = array("dbid"=> $jsonrpc_config['dbid'] );
        $argu2 = array($argu);
        $dataout=NULL;
        $clientAuth->query('login', $argu2, $dataout );
        $clientAuth->send();
        
        $rpc_session = $dataout['data']['sessionid'];
        return $rpc_session;
    }
}
