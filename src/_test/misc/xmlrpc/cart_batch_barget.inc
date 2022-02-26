<?php
// unittest for XMLRPC-method cart_batch_barget

class cart_batch_barget_TST extends BASE_TSTxrpc {
	function getXmlrpc_params() {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$optInputParams = $this->params;
		if ($optInputParams==NULL) {
			$error->set( $FUNCNAME, 2, 'Need batch-ID as PARAMETERS.' );
			return;
		}
		$params = array( $optInputParams );
		return $params;
	}
}
