<?php
// unittest for XMLRPC-method array_socket_connect

class array_socket_connect_TST extends BASE_TSTxrpc {
	
	function getXmlrpc_params() {
		
		$w_wafer_id	 = $this->_getDefObjID( "W_WAFER", "default" );
		$socket_id	 = $this->_getDefObjID( "SOCKET", "default" );
		$params = array( $w_wafer_id, 1, $socket_id );
		return $params;
		
	}
	
}
