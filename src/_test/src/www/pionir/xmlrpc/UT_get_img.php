<?php 
require_once(dirname(__FILE__).'/../../../_test_lib/http_client.inc');

class UT_get_img_php extends gUnitTestSub {
	
function __construct() {
	
	$this->module_noPreLoad=1;
}

// return: 0 : not passed, 1: passed
function dotest( &$sql, $options ) {
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	
	$params = array(
		"srv_url"  => $_SESSION['s_sessVars']['DocRootURL']. '/pionir/xmlrpc/get_img.php',
		"hostname" => $_SERVER['SERVER_NAME'],
		// "protocol"=>'ssl' // TBD: test ssl later 
	);
	$http_lib = new http_client($params);
	
	$imgid  = $this->_getDefObjID( "IMG", "default" );
	$this->_infoNow('IMG-ID', $imgid);
	
	$input_vars = array(
		'sess_id' => session_id(),
		'img_id'  => 0
	);
	
	$answer   = $http_lib->send($input_vars);
	$key_text = 'icono-err-text';
	$test_ok  = 0;
	
	$this->_infoNow('Answer1', $answer);
	
	if (substr($answer,0,strlen($key_text))==$key_text)  {
		$test_ok=1;
	}
	$this->_saveTestResult( 'INPUT-PARAM-MISSING', $test_ok);
	
	$input_vars = array(
			'sess_id' => session_id(),
			'img_id'  => $imgid
	);
	
	$answer = $http_lib->send($input_vars);
	$test_ok  = 0;
	if (substr($answer,0,strlen($key_text))!=$key_text)  {
		$test_ok=1;
	}
	if (strlen($answer)==NULL)  {
		$test_ok=0;
	}
	$this->_saveTestResult( 'NORMAL ANSWER', $test_ok);
	

	$retval = 1;
	return ($retval);
	
}

}
