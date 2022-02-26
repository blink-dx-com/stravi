<?php

class DEF_oGLOBALS_update_TST extends BASE_TSTxrpc {
	function getXmlrpc_params() {
		
		$paramFunc = array('DbUserMessage','2009-09-16 Wartung bis 22:00 Uhr') ;
			//$paramFunc = array('DbUserMessage',NULL) );
		$paramFunc = array('DbLoginDeny','');
			
		
		return $paramFunc;
	}
}
