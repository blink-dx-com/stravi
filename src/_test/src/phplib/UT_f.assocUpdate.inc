<?php

class UT_f_assocUpdate_inc extends gUnitTestSub {

function dotest( &$sqlo, $options ) {
    
	global $error;

	$CONCRETE_PROTO_ID = $this->_getExistObjID( "CONCRETE_PROTO", "assocUpdate" );
	
	$ABSTRACT_PROTO_ID = glob_elementDataGet( $sqlo,  "CONCRETE_PROTO",  "CONCRETE_PROTO_ID", 
			$CONCRETE_PROTO_ID, 'ABSTRACT_PROTO_ID');
	
	$this->_infoNow('Objects: ', 'CONCRETE_PROTO_ID:'.$CONCRETE_PROTO_ID. ' ABSTRACT_PROTO_ID:'.$ABSTRACT_PROTO_ID);
	
	$pra_steps = array();
	$sqlsel = "* from ABSTRACT_PROTO_STEP where ABSTRACT_PROTO_ID=".$ABSTRACT_PROTO_ID.' order by STEP_NR';
	$sqlo->Quesel($sqlsel);
	while ( $sqlo->ReadArray() ) {
	    $pra_steps[] = $sqlo->RowData;
	}

	$tmplib = new fAssocUpdate();
	$tmplib->setObj( $sqlo, 'CONCRETE_PROTO_STEP', $CONCRETE_PROTO_ID );
	$tmplib->set_log_level(fAssocUpdate::MASS_LOG_ACT_CHANGE);
	$tmplib->delMore($sqlo); // del all steps
	$tmplib->resetTouched();
	
	
	
	$iniarr = array( 'STEP_NR'=>$pra_steps[0]['STEP_NR'], 'ABSTRACT_PROTO_ID'=>$ABSTRACT_PROTO_ID );
	$iniarr2= array( 'STEP_NR'=>$pra_steps[1]['STEP_NR'], 'ABSTRACT_PROTO_ID'=>$ABSTRACT_PROTO_ID );
	$iniarr3= array( 'STEP_NR'=>$pra_steps[2]['STEP_NR'], 'ABSTRACT_PROTO_ID'=>$ABSTRACT_PROTO_ID );
	$iniarr4= array( 'STEP_NR'=>$pra_steps[3]['STEP_NR'], 'ABSTRACT_PROTO_ID'=>$ABSTRACT_PROTO_ID );
	$argu1 = array( 'QUANTITY'=>1.1, 'NOTES'=>'hallo 1' );
	$argu2 = array( 'QUANTITY'=>2.1, 'NOTES'=>'hallo 2' );
	$argu3 = array( 'QUANTITY'=>3.1, 'NOTES'=>'hallo 3' );
	$argu4 = array( 'QUANTITY'=>4.1, 'NOTES'=>'hallo 4' );
	
	$tmplib->updateInsert( $sqlo, $argu1, $iniarr);
	if ( $error->Got(READONLY) )  return;
	$tmplib->updateInsert( $sqlo, $argu2, $iniarr2);
	if ( $error->Got(READONLY) )  return;

	
	$sqlsel = "count(1) from CONCRETE_PROTO_STEP where CONCRETE_PROTO_ID=".$CONCRETE_PROTO_ID;
	$sqlo->Quesel($sqlsel);
	$sqlo->ReadRow();
	$num = $sqlo->RowData [0];
	$now_result=0;
	if ($num==2) {
	    $now_result=1;
	}
	$this->_saveTestResult('TEST01a', $now_result);
	
	$tmplib->resetTouched();
	
	
	
	$argu1  = array( 'QUANTITY'=>5.1, 'NOTES'=>'hallo' );
	$tmplib->updateInsert( $sqlo, $argu1, $iniarr);
	if ( $error->Got(READONLY) )  {
	    return;
	}
	$this->_saveTestResult('TEST02', 1);
	
	// update again ...
	$tmplib->updateInsert( $sqlo, $argu1, $iniarr);
	if ( $error->Got(READONLY) )  {
		return;
	}
	$this->_saveTestResult('TEST03', 1);

	$idarr = array( 'STEP_NR'=>$iniarr['STEP_NR'] ); // only one PK, one PK missing
	$tmplib->delOneRow( $sqlo, $idarr );
	if ( !$error->Got(READONLY) )  {
		return;
	}
	$error->reset();
	$this->_saveTestResult('TEST10', 1);

	
	$idarr = array( 'STEP_NR'=>$iniarr['STEP_NR'], 'ABSTRACT_PROTO_ID'=>$iniarr['ABSTRACT_PROTO_ID'] ); // only one PK
	$tmplib->delOneRow( $sqlo, $idarr );
	if ( $error->Got(READONLY) )  {
		$retval = -3; // error got
		return;
	}
	$this->_saveTestResult('TEST1', 1);

	$pk_array = $iniarr;
	$pk_array['CONCRETE_PROTO_ID'] = $CONCRETE_PROTO_ID;
	$data = gElement_exist2($sqlo, 'CONCRETE_PROTO_STEP', $pk_array);
	if ( $data>0 ) {
	    $this->_infoNow('Element exists', print_r($pk_array,1). ': result:'.$data );
		$retval = -4; // error got
		return;
	}
	$this->_saveTestResult('TEST12', 1);
	$retval = 1;
	
	$touch_stats = $tmplib->getTouchStats();
	$this->_infoNow('Touch Statistics', print_r($touch_stats,1) );
	$tmplib->close($sqlo);
	
	// logging tests
	$tmplib->resetTouched();
	$touch_stats = $tmplib->getTouchStats();
	$this->_infoNow('Touch Statistics (before MASS_LOG_POS_RANGE)', print_r($touch_stats,1) );
	
	$tmplib->set_log_level(fAssocUpdate::MASS_LOG_POS_RANGE);
	
	$tmplib->updateInsert( $sqlo, $argu1, $iniarr);
	if ( $error->Got(READONLY) )  return;
	$tmplib->updateInsert( $sqlo, $argu2, $iniarr2);
	if ( $error->Got(READONLY) )  return;
	$tmplib->updateInsert( $sqlo, $argu3, $iniarr3);
	if ( $error->Got(READONLY) )  return;
	$tmplib->updateInsert( $sqlo, $argu4, $iniarr4);
	if ( $error->Got(READONLY) )  return;
	$tmplib->close($sqlo);
	$this->_saveTestResult('TEST13', 1);
	
	$touch_stats = $tmplib->getTouchStats();
	$this->_infoNow('Touch Statistics (after MASS_LOG_POS_RANGE)', print_r($touch_stats,1) );
	
	$tmplib->resetTouched();
	$tmplib->set_log_level(fAssocUpdate::MASS_LOG_ALL_POS);
	
	$tmplib->updateInsert( $sqlo, $argu1, $iniarr);
	if ( $error->Got(READONLY) )  return;
	$tmplib->updateInsert( $sqlo, $argu2, $iniarr2);
	if ( $error->Got(READONLY) )  return;
	$tmplib->updateInsert( $sqlo, $argu3, $iniarr3);
	if ( $error->Got(READONLY) )  return;
	$tmplib->close($sqlo);
	$this->_saveTestResult('TEST14', 1);
	
	$tmplib->resetTouched();
	$tmplib->set_log_level(fAssocUpdate::MASS_LOG_ALL);
	$tmplib->updateInsert( $sqlo, $argu1, $iniarr);
	if ( $error->Got(READONLY) )  return;
	$tmplib->updateInsert( $sqlo, $argu2, $iniarr2);
	if ( $error->Got(READONLY) )  return;
	$tmplib->updateInsert( $sqlo, $argu3, $iniarr3);
	if ( $error->Got(READONLY) )  return;
	$tmplib->close($sqlo);
	$this->_saveTestResult('TEST15', 1);
	
	$tmplib->resetTouched();
	$tmplib->set_log_level(fAssocUpdate::MASS_LOG_POS_RANGE);
	$argu_many = array( 'QUANTITY'=>6.6 );
	$whereAdd='STEP_NR>30';
	$tmplib->updateMore( $sqlo, $argu_many, $whereAdd);
	$tmplib->close($sqlo);
	$this->_saveTestResult('TEST16', 1);
	
	
	return ($retval);
}

}