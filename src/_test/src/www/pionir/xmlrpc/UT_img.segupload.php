<?php
// module: img.segupload.php

class UT_img_segupload_php extends gUnitTestSub {
	
function __construct() {
	$this->module_noPreLoad=1;
	$this->GUI_test_flag = 1;
}
	

// return: 0 : not passed, 1: passed
function dotest( &$sql, $options ) {
	
	$imgid  = $this->_getDefObjID( "IMG", "default" );
	$relurl = 'pionir/xmlrpc/img.segupload.php';
	$params = 'img_id='.$imgid.'&s_file=hallo.png';
	
	$this->_info( 'call URL-params', htmlspecialchars($params) );
	
	$this->_openWindow( $relurl, $params );

	$retval = 1;
	
	return ($retval);
}

}