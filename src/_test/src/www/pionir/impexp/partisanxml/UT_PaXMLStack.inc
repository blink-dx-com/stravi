<?php
// module: PaXMLStack.inc


/**
 * dummy class, ... could produce conflict in other tests ?
 * @author steffen
 *
 */
class _htmllib {
	
	function statement($stmt) {
		// do nothing
	}

}

class UT_PaXMLStack_inc extends gUnitTestSub {
	
	function __construct() {
	
	}
	
	// return: 0 : not passed, 1: passed
	function dotest( &$sqlo, $options ) {
		
		
		$data = "";
		
		// prepare data
		// clean T_EXPORT, delete data of user
		$sqlo->Deletex ( 'T_EXPORT', 'DB_USER_ID='.$_SESSION['sec']['db_user_id']);
		
		
		$htmllib = new _htmllib();
		
		$tmplib = new PaXMLStack( $htmllib, $_SESSION['sec']['dbuser'],  $_SESSION['sec']['passwd'], $_SESSION['sec']['db'], $_SESSION['sec']['_dbtype']);
		
		$table  = "EXP";
		$expid  = 100;
		$filter = array("EXP_ID"=>$expid);
		$tmplib->put($table, $filter);
		
		$checkdata = glob_elementDataGet2( $sqlo, 'T_EXPORT', array('DB_USER_ID'=>$_SESSION['sec']['db_user_id'], 'TABLE_NAME'=>'EXP', 'COLUMN_NAME'=>'EXP_ID', 'VALUE'=>$expid), 'VALUE');
		$retval = 0;
		if ($checkdata==$expid) $retval = 1;
		$this->_saveTestResult('T_EXPORT_save_value', $retval);
		
		return (1);
	}

}