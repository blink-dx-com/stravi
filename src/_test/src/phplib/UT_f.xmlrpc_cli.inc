<?php

class UT_f_xmlrpc_cli_inc extends gUnitTestSub {
    // return: 0 : not passed, 1: passed
    function dotest( &$sqlo, $options ) {
    	global $error;
    	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
    	
    	$conf_file=$_SESSION['s_sessVars']['AppRootDir'] . "/config/xml_rpc_tests.inc";
    	if (!file_exists($conf_file)) {
    	    $error->set( $FUNCNAME, 10, 'Conf-File not found. "'.$conf_file.'"' );
    	    return;
    	}
    	require_once ($conf_file);
    	
    	$connect_sys_array = xml_rpc_testLogins();
    	if (!is_array($connect_sys_array)) {
    		$error->set( $FUNCNAME, 1, 'no xmlrpc connect configured in xml_rpc_tests.inc.' );
    		return;
    	}
    	$target = $this->_get_test_configval('xmlrpc.target');
    	if ($target==NULL) {
    		$error->set( $FUNCNAME, 2, 'no target configured in UNITTEST-config.' );
    		return;
    	}
    	
    	$connect_params = $connect_sys_array[$target];
    	if (!is_array($connect_params)) {
    		$error->set( $FUNCNAME, 3, 'target "'.$target.'" not configured.' );
    		return;
    	}
    	
    	$this->_infoNow( 'XMLPRC-Server:', 'Target: '.$target. ' hostname:'.$connect_params['hostname']  );
    
    	$obj1 = new fXmlrpcCli($connect_params);
    	$obj1->connect();   // can set $error
    	$answer = $obj1->funcCall("get_version", $params); // can set $error
    	$this->_infoNow( 'answer get_version:', print_r($answer,1) ); // just info for the test
    	
    	$exp_tmpl_id = $this->_getExistObjID('EXP_TMPL', 'default');
    	$param=array($exp_tmpl_id);
    	$answer = $obj1->funcCall("get_exp_tmpl_notes", $param ); // can set $error
    	$this->_infoNow( 'answer get_exp_tmpl_notes:', print_r($answer,1)  ); // just info for the test
    	
    	if ($answer!=NULL) $retval = 1;
    	else  $retval = 0;
    	
    	return ($retval);
    }

}
