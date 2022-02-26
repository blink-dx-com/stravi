<?php
// unittest for XMLRPC-method update_exp_raw_result_bulk

class update_exp_raw_result_bulk_TST extends BASE_TSTxrpc {
	function getXmlrpc_params() {
		
		$exp_id	  =  $this->_getDefObjID( "EXP", "cytoRes" );
		$my_channel_ids = array(2,5,7);
		$colNames = array("spot_id", "Mean_1", "COG_x");
		
		$chanParams1 = 
			array( 
				array(2,2.2983292338, 0.234),
				array(3,3.23, 	  234.3),
				array(4,4.2322323, 300),	 	   
				  );
		$chanParams2 = $chanParams1;
		$chanParams2[0][1] =0.392929;
		$chanParams3 = $chanParams1;   
		$chanParams3[0][2] =123.392929;
		
		$params =  array($exp_id, $my_channel_ids, $colNames, array($chanParams1,$chanParams2,$chanParams3));
		return $params;
	}
}
