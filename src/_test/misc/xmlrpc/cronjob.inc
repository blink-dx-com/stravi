<?php
/** unittest for XMLRPC-method cronjob
 * - give PARAMS: OEE.UPDATE_CART_NUM,OEE.CHECK_SBREAK_END
 * @author steffen
 *
 */
class cronjob_TST extends BASE_TSTxrpc {
	
	function getXmlrpc_params() {
		
		$params=array();
		$params = array( $this->subtest );
		return $params;
	}
	
}
