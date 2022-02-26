<?php
require_once(dirname(__FILE__).'/../../../_test_lib/http_client.inc');
require_once('date_funcs.inc');
require_once($_SESSION['s_sessVars']['loginPATH'].'/api/lib/metacall.inc');

class http_client_UT {
    
    function __construct($params) {    
        $this->url = 'http://'.$params["hostname"].$params["srv_url"];  
    }
    
    
    function send($input_vars) {
        
        // close session, because, the URL calls the same server with the same session ...
        session_write_close();
        
        $postdata = http_build_query($input_vars);
        $full_url = $this->url .'?' . $postdata;
        
        $my_file = fopen( '/tmp/xml_rpc_log.log', 'a' );
        if ($my_file) {
            fwrite($my_file,"<XX: date=\"". date("Y-m-d H:i:s"). ": ".$full_url . "\n");
            fclose($my_file);
        }
        
        $response = file_get_contents($full_url);
        
        // start session again
        session_start();
        
        return $response;
    }
}

class UT_get_doc_php extends gUnitTestSub {
	
function __construct() {
	$this->module_noPreLoad=1;
	//$this->GUI_test_flag = 1;
}



function _download_one(&$http_lib, $params) {
    

    $params['session_id'] = session_id();
    $answer   = $http_lib->send($params);
    $this->_infoNow( 'http-answer:', htmlspecialchars($answer).' LEN:'.strlen($answer) );
    

    return $answer;
}
	

// return: 0 : not passed, 1: passed
function dotest( &$sql, $options ) {
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	
	
	$exp_id = $this->_getDefObjID( "EXP", "toolbox" );
	
	if ($error->Got(READONLY))  {
	    $error->set( $FUNCNAME, 1, 'UT-Exp creation failed.' );
	    return;
	}
	$this->_infoNow( 'Exp-ID', $exp_id);
	
	
	$upload_url = $_SESSION['s_sessVars']['DocRootURL']. '/pionir/api/get_doc'; // .php
	$this->_infoNow( 'Upload-UR:', $upload_url);
	
	$testfilename = 'sub1/file1.txt';
	$this->_infoNow( 'TestFileName:', $testfilename);
	
	$params = array(
			"srv_url"  => $upload_url,
			"hostname" => $_SERVER['SERVER_NAME'],
			// "protocol"=>'ssl'
	);
	$http_lib = new http_client_UT($params);
	
	$this->_infoNow( 'call URL-params', print_r($params,1) );	
	
	$answer = $this->_download_one( $http_lib, array('exp_id'=>$exp_id, 'filename'=>$testfilename) );
	
	$tmpres = 1;
	$this->_infoNow( 'Output', htmlspecialchars($answer) );	
	$this->_saveTestResult('TEST01', $tmpres, 'EXP:'.$exp_id);
	
	
	$exp_id = 'BAD';
	$answer = $this->_download_one( $http_lib, array('exp_id'=>$exp_id, 'filename'=>$testfilename) );
	$tmpres = 1;
	$this->_infoNow( 'Output', htmlspecialchars($answer) );
	$this->_saveTestResult('TEST02', $tmpres, 'EXP:'.$exp_id);
	
	

	$retval = 1;
	return ($retval);
	
}

}