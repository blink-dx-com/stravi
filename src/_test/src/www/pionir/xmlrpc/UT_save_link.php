<?php
require_once(dirname(__FILE__).'/../../../_test_lib/http_client.inc');

/**
 * module: www/pionir/xmlrpc/save_link.php
 * @author steffen
 *
 */
class UT_save_link_php extends gUnitTestSub {
	
function __construct() {
	
	$this->module_noPreLoad=1;
	
}

// return: 0 : not passed, 1: passed
function dotest( &$sql, $options ) {
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	$linkid  = $this->_getDefObjID( "LINK", "default" );
	$relurl = $_SESSION['s_sessVars']['DocRootURL'].'/pionir/xmlrpc/save_link.php';
	
	$params = array(
		"srv_url"  => $relurl,
		"hostname" => $_SERVER['SERVER_NAME'],
		// "protocol"=>'ssl'
	);
	$http_lib = new http_client($params);
	
	$this->_infoNow( 'call URL-params', print_r($params,1) );
	$this->_infoNow( 'Document-ID',  $linkid);
	
	$input_vars = array(
			'sess_id' => session_id(),
			'link_id' => $linkid
	);
	
	$upload_files = array();
	$filename = tempnam ( '/tmp' , 'UnitTest' );
	$fp = fopen($filename, 'w');
	$retVal = fputs( $fp, 'UnitTest:'.$FUNCNAME."  BINARY-DATA:\x30\x55\x39\xfa" ); /* write binary data */
	fclose( $fp );
	
	$upload_files[] = array('field'=>'s_file', 'path'=>$filename);
	
	$answer   = $http_lib->send($input_vars, $upload_files);
	$this->_infoNow( 'http-answer:', $answer );
	
	$this->_compareTestResult('NORMAL', 'OK', $answer, 'normal upload');
	
	
	//
	// provoke error: NO link-id
	//
	$input_vars = array(
			'sess_id' => session_id(),
			'link_id' => 0
	);
	$answer   = $http_lib->send($input_vars, $upload_files);
	$this->_infoNow( 'http-answer:', $answer );
	$startstr='icono-err-text:';
	
	if (substr($answer,0,strlen($startstr))==$startstr ) $result=1;
	else $result=0;
	$this->_saveTestResult('Provoke-Error', $result, 'Link-ID missing');

	$retval = 1;
	return ($retval);
	
	/*
	 echo "UPLOAD-form: for LINK-ID: $linkid <br>";
	echo '<form ENCTYPE="multipart/form-data" ACTION="'.$relurl.'" METHOD=POST>'."\n";
	echo 'Hash: <input  name=hash >';
	echo '<input type=hidden name=link_id value='.$linkid.'><br>';
	echo '<input type=hidden name=mime value="application/iconoclust-run"><br>';
	echo 'File: <INPUT NAME="s_file" TYPE="file" ><br>';
	echo '<input type=submit >';
	echo "</form>\n";
	*/
	
}

}