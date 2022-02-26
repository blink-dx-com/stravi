<?php
require_once 'f.update.inc';

class UT_db_access_inc extends gUnitTestSub {
	
function __construct() {

}

/**
 * test bulkInsert
 * @param $sqlo
 * @return int
 */
function _sub_BulkInsert(&$sqlo) {
	
	$exp_id = $this->_getDefObjID( 'EXP', 'exp_update_script' );
	$channel_id = 1;
	
	$sqloTest = logon2();
	
	// delete old results
	$sql    = 'delete from EXP_RAW_RESULT where exp_id='.$exp_id;
	$retval = $sqlo->Query($sql);
	
	$sqlsel = "h_exp_raw_desc_id from H_EXP_RAW_DESC where name like 'iconoclust%'";
	$sqlo->Quesel($sqlsel);
	$sqlo->ReadRow();
	$my_hexp_id = $sqlo->RowData[0];
	
	$this->_infoNow( 'exp-id:',  $exp_id);
	
	
	$field_name    = array("SPOT_ID", "V01", "V02");
	$field_data	   = array( 
						array(2,2.29832, 123),
						array(3,3.23, 	  234.3),
						array(4,4.2322, 300),	 	   
						  );
					 
					 
	$field_fix  = array ( "EXP_ID" => $exp_id, "CHANNEL_ID" => $channel_id );
	$field_type = array("number", 'number', 'number');  
	$sqloTest->BulkInsert ( 'EXP_RAW_RESULT', $field_name, $field_type, $field_data, $field_fix, $exp_id);
	
	$this->_infoNow( '- data was:', 'field_names:'.print_r($field_name,1).' data:'.print_r($field_data,1));
	
	
	$field_fix["CHANNEL_ID"] = 2;
	$sqloTest->BulkInsert ( 'EXP_RAW_RESULT', $field_name, $field_type, $field_data, $field_fix, $exp_id);
	
	$this->_infoNow( 'second channel','');
	
}

// return: 0 : not passed, 1: passed
function dotest( &$sqlo, $options ) {
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	
	// $this->_sub_BulkInsert($sqlo);
	
	$retval=1;
	
	$test1='TEST01'; // rollback
	$tablename='EXP';
	$exp_id = $this->_getDefObjID( $tablename, 'default' );
	$this->_infoNow('(69) EXP-ID:', $exp_id);
	
	$argu0=array( 'EXP_ID'=>$exp_id, 'NOTES'=>'UT_Note1');
	gObjUpdate::update_row($sqlo, $tablename, $argu0);
	$arg0_got = glob_elemDataGet3( $sqlo, $tablename, array('EXP_ID'=>$exp_id), array('NOTES') );
	$this->_infoNow('(73) argu:', print_r($arg0_got,1));
	
	if ($arg0_got['NOTES']!=$argu0['NOTES']) {
	    $error->set( $FUNCNAME, 1, '(73) args not saved' );
	    return;
	}
	
	$sqlo->SetAutoCommit(false);
	
	$argu1=array('EXP_ID'=>$exp_id, 'NOTES'=>'UT_Note2');
	gObjUpdate::update_row($sqlo, $tablename, $argu1);
	
	$sqlo->Rollback();
	
	
	
	$argu1_got = glob_elemDataGet3( $sqlo, $tablename, array('EXP_ID'=>$exp_id), array('NOTES') );
	$this->_infoNow('argu:', print_r($argu1_got,1));
	
	if ($argu1_got['NOTES']!=$arg0_got['NOTES']) {
	    $error->set( $FUNCNAME, 2, '(73) rollback not working' );
	    return;
	}
	

    $sqlo->SetAutoCommit (true);
	
	return ($retval);
}

}
