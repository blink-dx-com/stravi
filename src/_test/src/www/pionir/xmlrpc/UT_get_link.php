<?php 
require_once(dirname(__FILE__).'/../../../_test_lib/http_client.inc');
require_once('o.LINK.subs.inc');
require_once('insertx.inc');

class UT_get_link_php extends gUnitTestSub {
	
function __construct() {
	
	$this->module_noPreLoad=1;
}

/**
 * create document file
 * @param unknown $sqlo
 * @param unknown $objid
 */
private function _create_doc_file(&$sqlo, $objid) {
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	$link_name = linkpath_get( $objid );
	
	$this->_infoNow('Create Test-File on Server; PATH:', $link_name);
	$fp = fopen($link_name, 'w');
	$retVal = fputs( $fp, 'UNITEST-Testdata: '.$FUNCNAME.":".date('H:m:s'). "\n" ); /* write data */
	fclose( $fp );
}

function _get_doc($sqlo) {
	
	$docname = "UT_".__CLASS__;
	$docid   = glob_elementDataGet( $sqlo, 'LINK', 'NAME', $docname, 'LINK_ID');
	if (!$docid) {
		// create document
		$insertlib = new insertC();
		$args = array(
				"vals"=> array(
					'NAME'=>$docname
				),
			);
			
		$docid   = $insertlib->new_meta($sqlo, 'LINK', $args);
	}
	return $docid;
}

// return: 0 : not passed, 1: passed
function dotest( &$sql, $options ) {
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	
	$params = array(
		"srv_url"  => $_SESSION['s_sessVars']['DocRootURL']. '/pionir/xmlrpc/get_link.php',
		"hostname" => $_SERVER['SERVER_NAME'],
		// "protocol"=>'ssl'
	);
	$http_lib = new http_client($params);
	
	$objid  = $this->_get_doc($sql);
	$this->_infoNow('LINK-ID', $objid);
	
	$this->_create_doc_file($sql, $objid);
	

	$input_vars = array(
		'sess_id' => session_id(),
		'link_id' => "0" //
	);
	
	$this->_infoNow('INPUT:', print_r($input_vars,1));
	$answer   = $http_lib->send($input_vars);
	$this->_infoNow('Output', $answer);
	$key_text = 'icono-err-text';
	$test_ok  = 0;
	if (substr($answer,0,strlen($key_text))==$key_text)  {
		$test_ok=1;
	}
	$this->_saveTestResult( 'INPUT-PARAM-MISSING', $test_ok);
	
	
	// -------------------------------
	
	
	$input_vars = array(
			'sess_id' => session_id(),
			'link_id'  => $objid
	);
	$this->_infoNow('INPUT:', print_r($input_vars,1));
	$answer = $http_lib->send($input_vars);
	$test_ok  = 0;
	if (substr($answer,0,strlen($key_text))!=$key_text)  {
		$test_ok=1;
	}
	$this->_saveTestResult( 'NORMAL ANSWER', $test_ok);
	

	$retval = 1;
	return ($retval);
	
}

}
