<?php
require_once 'f.xmlrpc_cli.inc';

/**
 * test connect_xml
 * @author steffen
 *
 */
class UT_connect2_xml_inc extends gUnitTestSub {
	
function __construct() {

}

// return: 0 : not passed, 1: passed
function dotest( &$sqlo, $options ) {
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	$this->_saveTest_def_later('TEST01','Activate on demand ...');
	return 1;
	
	
	$xmlrpc_connect_id = 'db01test.roburdev';
	require_once( $_SESSION['s_sessVars']['AppRootDir'].'/config/xml_rpc_tests.inc');
	
	$login_arr = xml_rpc_testLogins();
	
	$connect_params = $login_arr[$xmlrpc_connect_id];
	if (!is_array($connect_params)) {
		$this->_saveTestResult('data_prep: config_settings', 0, 'profile '.$xmlrpc_connect_id. ' in file xml_rpc_tests.inc not found');
		return 0;
	}
	
	// TEST1
	$obj1 = new fXmlrpcCli( $connect_params );
	$connectmethod = 1; // NEW method
	$obj1->connect(1,$connectmethod); 
	$sessionid_got = $obj1->sess_id;
	$tmpresult = $sessionid_got !=NULL ? 1 : 0;
	$this->_saveTestResult('01:connect: session_got', $tmpresult, $sessionid_got);
	// $this->_infoNow( '01:sessionid_got', $sessionid_got ); // just info for the test
	
	// TEST2
	$connect_params = $login_arr[$xmlrpc_connect_id];
	$connect_params['dbid'] = '???';
	$obj1 = new fXmlrpcCli( $connect_params );
	$obj1->connect(1);
	if ($error->Got(READONLY))  {
		$errLast   = $error->getLast();
		$error_txt = $errLast->text;
		$error_id  = $errLast->id;
		$error->reset();
	}
	
	$sessionid_got = $obj1->sess_id;
	$tmpresult = !($sessionid_got) ? 1 : 0;
	$this->_saveTestResult('02:connect: SESSION DENIED?', $tmpresult, $sessionid_got.' ERR:'.$error_txt.':'.$error_id );
	
	
	// TEST3
	$xmlrpc_connect_id ='clon02test2';
	$connect_params = $login_arr[$xmlrpc_connect_id];
	if (!is_array($connect_params)) {
		$this->_saveTestResult('data_prep: config_settings', 0, 'profile '.$xmlrpc_connect_id. ' in file xml_rpc_tests.inc not found');
		return 0;
	}
	$obj1 = new fXmlrpcCli( $connect_params );
	$obj1->connect(1);
	$sessionid_got = $obj1->sess_id;
	$tmpresult = $sessionid_got !=NULL ? 1 : 0;
	$this->_saveTestResult('03:connect: session_got', $tmpresult, $sessionid_got);
	
	
	return (1);
}

}
