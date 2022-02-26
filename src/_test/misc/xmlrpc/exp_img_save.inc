<?php
// unittest for XMLRPC-method exp_get_script
/*
 *  [1] $exp_name       chip name, barcode id
    [2] $img_params     img object parameters; array(field => value, ...)
    [3] $class_name     extra class name
    [4] $class_params   extra attribs; array(field => value, ...) 
*/
class exp_img_save_TST extends BASE_TSTxrpc {
	function getXmlrpc_params() {
		$exp_id	 = $this->_getDefObjID( "EXP", "default" );
		$params = array( $exp_id, array('EXPOSURE_TIME'=>8.34), 'series', array('number'=>15) );
		return $params;
	}
}
