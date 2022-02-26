<?php
// unittest for XMLRPC-method get_proto_list

class get_proto_list_TST extends BASE_TSTxrpc {
	function getXmlrpc_params() {
		$cp_id	 =  $this->_getDefObjID( "CONCRETE_PROTO", "default" );
		$params = array( $cp_id );
		return $params;
	}
}
