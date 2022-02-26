<?php
// unittest for XMLRPC-method gObj_getIdByName

class gObj_getIdByName_TST extends BASE_TSTxrpc {
	
	function getXmlrpc_params() {
		
		$subtest = $this->subtest;
		switch ($subtest) {
			case 'like':
				$object_name =  'UNITTEST%';
				$sea_opt =array('LIKE'=>1);
				$params = array( 'ABSTRACT_PROTO', $object_name, $sea_opt );
				break;
			default:
				$object_name =  'UNITTEST gObj_getIdByName';
				$params = array( 'ABSTRACT_PROTO', $object_name );
		}
		return $params;
		
	}
}
