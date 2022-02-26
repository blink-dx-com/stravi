<?php

class LAB_p129_devSaveData_TST extends BASE_TSTxrpc {
	function getSubTestKeys() {
		return array('CAB', 'TT1');
	}
	
	function getXmlrpc_params() {
		
		$subtest = $this->subtest;
		$params  = $this->params;
		
		
		switch  ( $subtest ) {
			case "CAB":
				$dev_id	 =  $this->_getDefObjID( "CHIP_READER","CAB");
				$paramFunc = array(
					'step'=>array(
						 'ACTIONKEY'=>'PCP',
						 'DEVICE_ID'=>$dev_id,
						 'DATE1'    =>'2009-10-26 13:45',
						 'DATE2'    =>'2009-10-26 13:49',
						 'ERR'		=>'LEV1_error',
						 'NOTES'	=>'notes_yippi'
						), 
					'data'=>array(
						'0914700009'=>array('QUALITY'=>1),
						'0914700007'=>array('QUALITY'=>1),
						'0914700003'=>array('QUALITY'=>1),
						'0914700050'=>array('QUALITY'=>1),
						'0911400014'=>array('QUALITY'=>1) // other batch
					   ) 
					
					);
				break;
				
			case "TT1":
				$dev_id	 =  $this->_getDefObjID( "CHIP_READER","TT1");
				
				$paramFunc = array(
					'step'=>array(
						 'ACTIONKEY'=>'TT1',
						 'DEVICE_ID'=>$dev_id,
						 'DATE1'    =>'2009-10-26 13:45',
						 'DATE2'    =>'2009-10-26 13:49',
						 'ERR'		=>'LEV1_error',
						 'NOTES'	=>'notes_yippi'
						), 
					'data'=>array(
						'01107795101'=>array('QUALITY'=>1,'VOLFAC'=>34.9,    'BARCODE'=>234569),
						'01107795102'=>array('QUALITY'=>1,'VOLFAC'=>33.8,    'BARCODE'=>234568),
						'01107795103'=>array('QUALITY'=>1,'VOLFAC'=>33.7,    'BARCODE'=>234567)
						
					   ) 
					
					);
					/**
					    '0914700009' =>array('QUALITY'=>0, 'VOLFAC'=>39.9),
						'0914700050' =>array('QUALITY'=>1,'ERROR'=>'LIMITS', 'VOLFAC'=>40.9, 'BARCODE'=>234567),
						'01033200021'=>array('QUALITY'=>1,'VOLFAC'=>34.9,    'BARCODE'=>234569),
					 */
				
				break;
			default:
				$dev_id	 =  $this->_getDefObjID( "CHIP_READER","keyence");
				$paramFunc = array(
					'step'=>array(
						 'ACTIONKEY'=>'MSK',
						 'DEVICE_ID'=>$dev_id,
						 'DATE1'    =>'2009-10-26 13:45',
						 'DATE2'    =>'2009-10-26 13:49',
						 'ERR'		=>'LEV1_error',
						 'NOTES'	=>'notes_yippi'
						), 
					'data'=>array(
						'01033200023'=>array('QUALITY'=>1,'VOLFAC'=>39.9),
						'01033200024'=>array('QUALITY'=>0,'ERROR'=>'LIMITS', 'VOLFAC'=>43.9),
						'01033200025'=>array('QUALITY'=>1,'VOLFAC'=>25.4),
						'01108800002'=>array('QUALITY'=>1,'VOLFAC'=>34.9) // other batch
					   ) 
					
					);
				break;
		}	
				
			
		
		return $paramFunc;
	}
}
