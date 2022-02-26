<?php
// unittest for XMLRPC-method cart_batch_cartInf1

class cart_batch_cartInf1_TST extends BASE_TSTxrpc {
	
	function getXmlrpc_params() {
		
		$subtest  = $this->subtest;
		$batch_id = $this->_getDefObjID( "CART_BATCH", "cart_batch_cartInf1" );
			
		$params = array( $batch_id );
		
		if ( $subtest=='expiryTest') {
			$batch_id = $this->_getDefObjID( "CART_BATCH", "expiryTest" );
			$params   = array( $batch_id );
		}
		return $params;
	}
}
