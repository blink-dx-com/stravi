<?php

class DEF_gObj_addAttach_TST extends BASE_TSTxrpc {
	function getXmlrpc_params() {
		
		$subtest = $this->subtest;
		
		$obj_id	   	= $this->_getDefObjID( "EXP", "default" );
	  	switch  ( $subtest ) {
	  		case 'ADD':
	  			$data = "Dies ist ein Test.";
				$data      	= base64_encode($data);
				$key		= "results";
				$name		= "result.txt";
				$notes 		= "test results";
				$paramFunc = array('EXP', $obj_id, $data, $key, $name, $notes) ;
				break;
			case 'UPDATE':
				$data = "Dies ist ein neuer Test.";
				$data      	= base64_encode($data);
				$key		= "new results";
				$name		= "result.txt";
				$notes 		= "neue test results";
				$paramFunc = array('EXP', $obj_id, $data, $key, $name, $notes) ;
				break;
			case "BIGFILE":
				// /////clondiag.jena/dfs/Projekte/89.Partisan_system/Dev/03_Test_data/02_misc/gObj_addAttach.bigfile.dat
				$path = "/terra/expdata/Mario/iroq3_0260674264_log_PIMA-D-000105.txt";
				$fp = fopen($path, 'r');
			  	$data = fread($fp, filesize($path));
			  	fclose($fp);
			  	$data      	= base64_encode($data);
				$key		= "results";
				$name		= "result.txt";
				$notes 		= "test results";
				$paramFunc = array('EXP', $obj_id, $data, $key, $name, $notes) ;
				break;
	  	}	

	  	return $paramFunc;
		 
	}
}
