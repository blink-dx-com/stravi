<?php

class DEF_oEXP_projProto_connect_TST extends BASE_TSTxrpc {
	function getXmlrpc_params() {
		
		$subtest = $this->subtest;
		$params  = $this->params;
		
		$paramFunc = array ('88634', array( array('expName'=>'exp1', 'pos'=>'1'), 
				array('expName'=>'exp2', 'pos'=>'2'), 
				array('expName'=>'exp3', 'pos'=>'3'), 
				array('expName'=>'exp4', 'pos'=>'4'))
				);
		switch  ( $subtest ) {
			case "noExp": //Experiment not found
				$paramFunc = array ('88634', array( array('expName'=>'exp1000', 'pos'=>'4')));
				break;
			case "noProto": //No Protocol at given Position
				$paramFunc = array ('88634', array( array('expName'=>'exp3', 'pos'=>'1000')));
				break;
			case "noProj": //Project does not exist
				$paramFunc = array ('886343456789', array( array('expName'=>'exp1', 'pos'=>'1')));
				break;
			case "ProtoExp": //Protocol is already connected to Experiment
				$paramFunc = array ('88634', array( array('expName'=>'exp1', 'pos'=>'1')));
				break;
			default:
				break;
		}
		
		return $paramFunc;
	}
}
