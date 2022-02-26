<?php


class UT_rconnect_php extends gUnitTestSub {
	
function __construct() {
	
	$this->module_noPreLoad=1;
	$this->GUI_test_flag = 1;
}

// return: 0 : not passed, 1: passed
function dotest( &$sql, $options ) {
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	$upload_url= "https://".$_SERVER['SERVER_NAME']. $_SESSION['s_sessVars']['DocRootURL']. '/pionir/xmlrpc/rconnect.php';
	
	echo "URL: ".$upload_url."<br>";
	echo '<form ENCTYPE="multipart/form-data" ACTION="'.$upload_url.'" METHOD=POST>'."\n";
	echo 'User:<input type=text name=user value=""><br> ';
	echo 'PW:<input type=password name=pwBase64 value=""> (Base64-encoded)<br> ';
	echo 'DBID:<input type=text name=dbid value=""><br> ';
	echo '<input type=submit >';
	
	echo "</form>\n";


	$retval = 1;
	return ($retval);
	
	
	
	$retval = 1;
	
	return ($retval);
}

}