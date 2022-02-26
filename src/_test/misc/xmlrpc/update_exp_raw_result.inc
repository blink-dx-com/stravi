<?php
// unittest for XMLRPC-method update_exp_raw_result

class update_exp_raw_result_TST extends BASE_TSTxrpc {
	function getXmlrpc_params() {
		
		/* $subtest : "2channels"
					  "noData"
		*/		
		$exp_id	 =  $this->_getDefObjID( "EXP", "cytoRes" );
		
		$tmpparam = array($exp_id, 0); // delete all channels
		$this->XmlCliObj->funcCall( "delete_raw_res", $tmpparam ); 
		
		$OneChanParams = 
			array( $exp_id, 1, array("spot_id", "Mean_1", "COG_x"), 
			 	array( 
				1=>array(2,2.2983292338, 0.234),
				2=>array(3,3.23, 	  234.3),
				3=>array(4,4.2322323, 300),	 	   
				  ) 
		    );
		$SecChanParams = 
			array( $exp_id, 2, array("spot_id", "Mean_1", "COG_x"), 
			 	array( 
				1=>array(2,2.3, 0.0345  ),
				2=>array(3,3.3, 234.5),
				3=>array(4,4.3, 400  ),	 	   
				  ) 
		    );			
		
		switch  ( $subtest ) {
			// no data
			case  "noData":
				$params = array( $exp_id, 1, array("spot_id", "Mean_1"), NULL );
				break;
			case  "missData":
				// mising data of "COG_x"
				$params = array( $exp_id, 1, array("spot_id", "Mean_1", "COG_x"), 
						 	array( 
							1=>array(2,2.2983292338, ),
							2=>array(3,3.23, 	  ),
							3=>array(4,4.2322323, ),	 	   
							  ) 
					  );
				break;
			case  "2channels":
				$params = $OneChanParams;
				$this->XmlCliObj->funcCall( "update_exp_raw_result", $params ); 
				$params = $SecChanParams;
				break;
			case "normal01":
				// no rounding of values !!!
				
				$expid=135851;
				$tmpparam = array($expid, 0); // delete all channels
				$this->XmlCliObj->funcCall( "delete_raw_res", $tmpparam ); 
				
				$params = 
					array( $expid, 1, array("spot_id", "Mean", "Flags"), 
					 	array( 
						1=>array(2,2.2983292338, 1.45),
						2=>array(3,3.23, 	  0.34),
						3=>array(4,4.2322323, 1.393),	 	   
						  ) 
				    );
				break;
			case  "wrongColType":
				$params = $OneChanParams;
				$params[0] = 135851;
				break;	
			default:
				$params = $OneChanParams;
		}
			
		return $params;
	}
}
