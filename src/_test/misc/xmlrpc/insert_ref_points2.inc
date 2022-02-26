<?php
// unittest for XMLRPC-method insert_ref_points2

class insert_ref_points2_TST extends BASE_TSTxrpc {
	function getXmlrpc_params() {
		$exp_id	 =  $this->_getDefObjID( "EXP", "cytoRes" );
		$img_id	 =  $this->_getDefObjID( "IMG", "default" );
		$spotlist = array( array(1,2,3.3), array(2,5,4.3) );
		$params = array( $exp_id, $img_id, $spotlist );
		return $params;
	}
}
