<?php

class LAB_p129_finish_order_TST extends BASE_TSTxrpc {
	
	function getXmlrpc_params() {
		
		$subtest = $this->subtest;
		
		switch ($subtest) {
			case "001":	//Job kann beendet werden		
				$paramFunc = array(
			  		'jobId'   => 4, 
			  		'runStart'=>'2011-04-05T16:23', 
			 		'runEnd'  =>'2011-04-06T08:14', 
			  		'ioCount'  => 10000, 
			  		'nioCount'=>  1200,
			  		'qcCount' =>    15
		  		);
		  		break;
		  		
		  	case "002": //Job ID existiert nicht
				$paramFunc = array(
			  		'jobId'   => 9999929999, 
			  		'runStart'=>'2011-04-05T16:28', 
			 		'runEnd'  =>'2011-04-06T08:19', 
			  		'ioCount'  => 10000, 
			  		'nioCount'=>  1200,
			  		'qcCount' =>    15
		  		);
		  		break;
		  		
		  	case "003": //Job besitzt keine Batch
				$paramFunc = array(
			  		'jobId'   => 26, 
			  		'runStart'=>'2011-04-05T16:23', 
			 		'runEnd'  =>'2011-04-06T08:14', 
			  		'ioCount'  => 10000, 
			  		'nioCount'=>  1200,
			  		'qcCount' =>    15
		  		);
		  		break;
		  	
		  	case "004": //keine Job ID
				$paramFunc = array(
		  		);
		  		break;
		}
		
		return $paramFunc;
	}
}
