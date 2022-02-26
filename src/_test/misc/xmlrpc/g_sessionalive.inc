<?php
// unittest for XMLRPC-method g_sessionalive

class g_sessionalive_TST extends BASE_TSTxrpc {
	function getXmlrpc_params() {
		$subtest   = $this->subtest;
		$paramFunc = array();
		switch ($subtest) {
			case "err":
				// send a invalid session ID
				$this->XmlCliObj->setSessionID('badSessionId');
				break;
			default:
				break;
		}
		return $paramFunc;
	}
}
