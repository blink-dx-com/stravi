<?php
// example for testing: csv.substance.lib.xarray.inc

class UT_globals_inc extends gUnitTestSub {
	
	function __construct() {

	}
	
	function _sys_error_subMehtod() {
		sys_error_my( 'UnitTest test1' );
	}
	
	function _doSubTest(&$sqlo, $subTest) {
		
		$this->_infoNow( 'SubTest', $subTest); 
		
		switch ($subTest) {
			case 'update_row_sGet':
				$tablename='EXP';
				$idarr= array('EXP_ID'=>100);
				
				$argu = array('NAME'=>'test', 'EXP_TMPL_ID'=>NULL);
				$sqlString = update_row_sGet($sqlo, $tablename, $argu, $idarr, $option);
				$this->_infoNow( '1. SQL:', print_r($argu,1).' sql:'.htmlSpecialChars($sqlString) );
				
				$argu= array('NAME'=>'test', 'EXP_TMPL_ID'=>1);
				$sqlString = update_row_sGet($sqlo, $tablename, $argu, $idarr, $option);
				$this->_infoNow( '2. SQL:', print_r($argu,1).' sql:'.htmlSpecialChars($sqlString) );
				
				$argu= array('NAME'=>'test', 'QUALITY'=>0);
				$sqlString = update_row_sGet($sqlo, $tablename, $argu, $idarr, $option);
				$this->_infoNow( '2. SQL:', print_r($argu,1).' sql:'.htmlSpecialChars($sqlString) );
				break;
			case  'sys_error_my':
				$this->_sys_error_subMehtod();
				break;
			default:
				$this->_infoNow( 'SubTest', 'No SubTest given!' );
				break;
		}
	}
	
	
	function dotest( &$sqlo, $options ) {
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
		// $tests  = array('update_row_sGet', 'sys_error_my');
		$subtest = $options['subTest'];
		
		if ($subtest!=NULL) {
		    $this->_doSubTest($sqlo, $subtest);
		}
		
		debugOut('Test-log-file.', $FUNCNAME, 0, 1);
		
		$retval=1;
		return ($retval);
	}

}
