<?php
// unittest for XMLRPC-method delete_ref_points

class delete_ref_points_TST extends BASE_TSTxrpc {
	function getXmlrpc_params() {
		
		$exp_id	 =  $this->_getDefObjID( "EXP", "cytoRes" );
		$img_id	 =  $this->_getDefObjID( "IMG", "default" );
		$spotlist= array( array(1,2,3.3), array(2,5,4.3) );
		$tmpparam  = array( $exp_id, $img_id, $spotlist );
		
		$this->XmlCliObj->funcCall( "insert_ref_points2", $tmpparam );
		
		$params = array( $exp_id, $img_id );
		return $params;
	}
}
