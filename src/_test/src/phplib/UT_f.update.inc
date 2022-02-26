<?php
require_once 'o.DB_USER.subs2.inc';
require_once('f.update.inc');

class UT_f_update_inc extends gUnitTestSub {
	


/**
 * 
 */
function dotest( &$sqlo, $options ) {
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	
	// update a SUBST
	$suc_id      = $this->_getExistObjID( "CONCRETE_SUBST", "default" );
	$contact_id  = $this->_getExistObjID( "CONTACT", "default" );

	
	$this->_infoNow('SUC_ID:', $suc_id);
	$this->_infoNow('CONTACT_ID:', $contact_id);
	
	$tablename="CONCRETE_SUBST";
	$idarr=array("CONCRETE_SUBST_ID"=>$suc_id);
	
	$argu=array(
	    'CONTACT_ID'=>$contact_id
	);
	gObjUpdate::update_row_s($sqlo, $tablename, $argu, $idarr);
	if ($error->Got(READONLY))  {
	    $error->set( $FUNCNAME, 1, 'Update failed' );
	    return;
	}
	$new_vals = glob_elemDataGet3( $sqlo, $tablename, $idarr, array('CONTACT_ID') );
	$this->_compareTestResult('TEST01', $contact_id, $new_vals['CONTACT_ID']);
	
	$argu=array(
	    'CONTACT_ID'=>NULL
	);
	gObjUpdate::update_row_s($sqlo, $tablename, $argu, $idarr);
	if ($error->Got(READONLY))  {
	    $error->set( $FUNCNAME, 1, 'Update failed' );
	    return;
	}
	$new_vals = glob_elemDataGet3( $sqlo, $tablename, $idarr, array('CONTACT_ID') );
	$this->_compareTestResult('TEST01', NULL, $new_vals['CONTACT_ID']);
	
	return 1;
}

}