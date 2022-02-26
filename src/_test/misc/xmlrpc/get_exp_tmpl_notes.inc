<?php
// unittest for XMLRPC-method get_exp_tmpl_notes

class get_exp_tmpl_notes_TST extends BASE_TSTxrpc {
	function getXmlrpc_params() {
		$exp_id	 =  $this->_getDefObjID( "EXP_TMPL", "default" );
		$params = array( $exp_id );
		return $params;
	}
}
