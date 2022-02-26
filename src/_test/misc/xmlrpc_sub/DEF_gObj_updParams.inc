<?php

class DEF_gObj_updParams_TST extends BASE_TSTxrpc {
	function getSubTestKeys() {
		return array('badColumn');
	}
	
	function getXmlrpc_params() {
		
		$subtest = $this->subtest;
		$params  = $this->params;
		
		$exp_id	= $this->_getDefObjID( "EXP", "update" );
		$cols   = array(
			'NAME'=>'UnitTest update', 
			'EXP_DATE'=>'2009-10-03 14:23:53'
			);
		
		switch  ( $subtest ) {
			
			case "objUpdate":
				// EXTRA_OBJ
				$cols   = array(
				'NAME'=>'UnitTest update2'

				);
				// 'STAU_character'
				$paramFunc = array('EXP', 296106 , array('vals'=>$cols, 'xobj'=>
							   array('class_name'=>'Typing', 'values'=>array('Spa_short'=>'spa_test'))
						       ) ) ;
				break;
			case "badColumn":
				// CCT_ACCESS_ID is not allowed !
				$cols   = array(
					'NAME'=>'UnitTest update', 
					'CCT_ACCESS_ID'=>100
				);
				$paramFunc = array('EXP', $exp_id, array('vals'=>$cols) ) ;
				break;
			case "err01":
				// objid missing
				$cols   = array(
				'NAME'=>'UnitTest update',
				'CCT_ACCESS_ID'=>100
				);
				$paramFunc = array('EXP', 0, array('vals'=>$cols) ) ;
				break;
			case "err02":
				// objid not numeric
				$cols   = array(
				'NAME'=>'UnitTest update',
				'CCT_ACCESS_ID'=>100
				);
				$paramFunc = array('EXP', 'nonNumeric', array('vals'=>$cols) ) ;
				break;
				
					
			default:
				$paramFunc = array('EXP', $exp_id, array('vals'=>$cols) ) ;
				break;
		}
		
		return $paramFunc;
	}
}
