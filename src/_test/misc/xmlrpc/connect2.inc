<?php
// unittest for XMLRPC-method CONNECT

class connect2_TST extends BASE_TSTxrpc {
	
	function getXmlrpc_params() {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		$subtest  	= $this->subtest;
	
		// $pwB64 = 'xxx';
		
		$params = $this->XmlCliObj->getConnectionParams();
		//$params["cct_user"]   = 'steffen';
		//$params["cct_password"]   = $pwB64;
		//$params["dbid"]   = db01test;
		
		
		$this->XmlCliObj->setConnectionParams($params);
		$this->XmlCliObj->connect(0,1);
		$tmpsess = $this->XmlCliObj->getSessionID();
		// echo $FUNCNAME.":INFO: sent_params: ".print_r($params,1)."<br>\n";
		echo $FUNCNAME.":INFO: returned Session-ID: ".$tmpsess."<br>\n";
		
		if ($error->Got(READONLY))  {
			$error->printAll();
			return;
		}
		
		
		
		$params = array();
		$this->metacallFunc = 'get_version'; // CHANGE the XMLRPC method  !!!!!
		
		return $params;
		
	}
	
}
