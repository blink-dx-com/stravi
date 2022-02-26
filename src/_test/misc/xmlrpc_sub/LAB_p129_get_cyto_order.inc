<?php

class LAB_p129_get_cyto_order_TST extends BASE_TSTxrpc {
	function getXmlrpc_params() {
		global $error;
		
		$subtest = $this->subtest;
		$params  = $this->params;
		
		switch ($subtest){
			case "001": //Job mit vollständigen Daten
				$cyto_order_id	 =  "4";
				$paramFunc = array($cyto_order_id);
				break;
			case "002": //Job mit vollständigen Daten, simulatePreAssembly
				$cyto_order_id	 =  "4";
				$paramFunc = array($cyto_order_id);
				break;
			case "003": //Job existiert nicht
				$cyto_order_id	 =  "9999";
				$paramFunc = array($cyto_order_id);
				break;
			case "004": //Job ohne Batch
				$cyto_order_id	 =  "26";
				$paramFunc = array($cyto_order_id);
				break;
			case "005": //Job mit Batch ohne Kartuschen
				$cyto_order_id	 =  "24";
				$paramFunc = array($cyto_order_id);
				break;
			case "006": //Job ohne Material
				$cyto_order_id	 =  "39";
				$paramFunc = array($cyto_order_id);
				break;
			case "007": //keine Job ID
				$paramFunc = array($cyto_order_id);
				break;
			case "008": //Job mit Material ohne SAP ILNs
				$cyto_order_id	 =  "35";
				$paramFunc = array($cyto_order_id);
				break;
			case "009": //Job mit Material ohne SAP Materialnummer
				$cyto_order_id	 =  "33";
				$paramFunc = array($cyto_order_id);
				break;
			case "010": //keine Parameterübergabe
				$cyto_order_id	 =  "33";
				$paramFunc = array();
				break;
			case "011": //Job mit vollständigen Daten, nicht simulatePreAssembly
				$cyto_order_id	 =  "31";
				$paramFunc = array($cyto_order_id);
				break;
			case "012": //Job mit vollständigen Daten, gescannten Kartuschen
				$cyto_order_id	 =  "5";
				$paramFunc = array($cyto_order_id);
				break;

			case "paramCYTO": //Job mit vollständigen Daten, gescannten Kartuschen
				$cyto_order_id	 =  $params;
				$paramFunc = array($cyto_order_id);
				break;

			case "013": //Job mit updateFlag
				$cyto_order_id	 =  "4";
				$paramFunc = array($cyto_order_id);
				break;
			case "014": //Job ohne updateFlag
				$cyto_order_id	 =  "47";
				$paramFunc = array($cyto_order_id);
				break;

		}
		if (!$cyto_order_id) {
			$error->set($FUNCNAME, 1, 'benoetige cyto_order_id als "Parameter"');
			return;
		}
		return $paramFunc;
	}
}
