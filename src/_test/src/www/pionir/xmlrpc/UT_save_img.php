<?php
// module: www\pionir\xmlrpc\save_img.php

class UT_save_img_php extends gUnitTestSub {
	
function __construct() {

	$this->module_noPreLoad=1;
	$this->GUI_test_flag = 1;
	
}

// return: 0 : not passed, 1: passed
function dotest( &$sql, $options ) {
	
	$imgid  = $this->_getDefObjID( "IMG", "default" );
	$relurl = $_SESSION['s_sessVars']['DocRootURL'].'/pionir/xmlrpc/save_img.php';
	
	// $_SESSION['globals']["img_path"]='/tmp/sadsadasdasd'; // /data/data/db.db01test/images
	
	
	$this->_info( 'call URL-params', htmlspecialchars($params) );
	
	echo "UPLOAD-form: for IMG-ID: $imgid <br>";
	echo '<form ENCTYPE="multipart/form-data" ACTION="'.$relurl.'" METHOD=POST>'."\n";
	echo '<input type=hidden name=img_id value='.$imgid.'>';
	echo '<input type=submit >';
	echo '<INPUT NAME="s_file" TYPE="file" > <br>';
	echo "</form>\n";
	
	
	// $this->_openWindow( $relurl, $params );

	$retval = 1;
	
	return ($retval);
}

}