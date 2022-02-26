<?php
// unittest for XMLRPC-method insert_spot_res_bulk

class insert_spot_res_bulk_TST extends BASE_TSTxrpc {
	function getXmlrpc_params() {
		$exp_id	 =  $this->_getDefObjID( "EXP", "cytoRes" );
		$spotids = array( 1,2,3);
		$my_value= array( 1.1,2.1,3.1);
		$my_sigma= array( 1.2,2.2,3.2);
		$my_backgrd	= array( 1,2,3);
		$my_valid	= array( 0,1,0);		
		$params = array( $exp_id, $spotids,  $my_value,$my_sigma,$my_backgrd,$my_valid  );
		return $params;
	}
}
