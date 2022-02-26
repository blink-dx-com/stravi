<?php

class LAB_p098_printLabel_TST extends BASE_TSTxrpc {
	
	function getSubTestKeys() {
		return array('user', 'prod', 'wrap');
	}
	
	
	function getXmlrpc_params() {
		$subtest = $this->subtest;
		
		$inputstr=$this->params;
		$paramFunc = array("", $inputstr) ;
		switch  ( $subtest ) {
			case 'user':
				$paramFunc = array('user',$inputstr);
				break;
				
			case 'prod':
				
				$paramFunc = array('prod',$inputstr);
				break;
			
			case 'wrap':
			$paramFunc = array('wrap',$inputstr);
				break;
		}
		return $paramFunc;
	}
}