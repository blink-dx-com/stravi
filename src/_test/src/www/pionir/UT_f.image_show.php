<?php
// module: f.image_show.php

class UT_f_image_show_php  extends gUnitTestSub {
	
function __construct() {
	$this->module_noPreLoad=1;
	$this->GUI_test_flag = 1;
}
	

// return: 0 : not passed, 1: passed
function dotest( &$sql, $options ) {
	
	$imgid  = $this->_getDefObjID( "IMG", "default" );
	$relurl = 'pionir/f.image_show.php';
	$params = 
		'?filename='. urlencode('\\\\clondiag.jena\dfs\Produktion\prod\QC_funktional_OP\2011\2011_08_22_85136\AT-06.24196044010305.chip.bmp') .
		'&dim[0]=300&dim[1]=250&img_params='. urlencode('-rotate "45"');
	
	$this->_info( 'call URL-params', $relurl . htmlspecialchars($params) );
	
	$this->_openWindow( $relurl, $params );

	$retval = 1;
	
	return ($retval);
}

}