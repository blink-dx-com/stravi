<?php


class UT_o_PROTO_subs_inc extends gUnitTestSub {


/**
 * one atomic test to update a STEP
 * @param  array $newvals    : update data  : array("quantity"=>0.2, "notes"=>'hello', "subst"=>NULL );
 * @param  array $compareVal : expected data: array("QUANTITY"=>0.2, "NOTES"=>'hello' );
 * @param  string $testName e.g. 01
 * @param  int $step_nr
 * @param  int $defColFlag 0,1 : just update defined columns in $new
 * @return NULL
 */
function oneStepTest(&$testlib, &$sqlo, $testName, $step_nr, $newvals, $compareVal, $defColFlag=0) {
	global $error;
	$mainTable = 'CONCRETE_PROTO_STEP';
	
	$ABSTRACT_PROTO_ID = $this->ABSTRACT_PROTO_ID;
	$CONCRETE_PROTO_ID = $this->CONCRETE_PROTO_ID;
	
	$iniarr = array( 'STEP_NR'=>$step_nr, 'ABSTRACT_PROTO_ID'=>$ABSTRACT_PROTO_ID );
	$pkfull = array_merge( array('CONCRETE_PROTO_ID'=>$CONCRETE_PROTO_ID ), $iniarr);
	
	$stepopt = NULL;
	if ($defColFlag>0) $stepopt['defcols']=1;
	$testlib->oneStepSave( $sqlo, $newvals, $step_nr, $stepopt);
	if ( $error->Got(READONLY) )  {
		// hard error
		return -2;
	}
	
	$askKeys = array_keys($compareVal);
	$answer  = glob_elemDataGet3($sqlo, $mainTable, $pkfull, $askKeys );
	$diffarr = $this->_compare_Array_floatVals( $compareVal, $answer );
	$this->_infoNow( 'Test:'.$testName, 'STEP_NR: '.$step_nr.' result:'. print_r($answer,1));
	if ( is_array($diffarr) ) {
		$test_info = glob_array2String( $diffarr );
		$testresult=0;
	} else {
		$test_info = 'ok';
		$testresult=1;
	}
	$this->_saveTestResult('Test:'.$testName, $testresult, 'Diff-Result:'. $test_info );
	
}

// return: 0 : not passed, 1: passed
function dotest( &$sqlo, $options ) {
	global $error;

	$retval = 0;
	$mainTable = 'CONCRETE_PROTO_STEP';
	
	$ABSTRACT_PROTO_ID = $this->_getExistObjID( "ABSTRACT_PROTO", "default" );
	if (!$ABSTRACT_PROTO_ID) return -1;
	
	// create a new protocol
	$args = array('vals'=>array('ABSTRACT_PROTO_ID'=>$ABSTRACT_PROTO_ID, 'NAME'=>'UT '.__CLASS__) );
	$CONCRETE_PROTO_ID = $this->_createObj( "CONCRETE_PROTO", $args );
	if (!$CONCRETE_PROTO_ID) return -1;
	
	$this->_infoNow( 'Input-Data:',"CONCRETE_PROTO_ID: $CONCRETE_PROTO_ID,  ABSTRACT_PROTO_ID:".$ABSTRACT_PROTO_ID);

	
	$this->ABSTRACT_PROTO_ID=$ABSTRACT_PROTO_ID;
	$this->CONCRETE_PROTO_ID=$CONCRETE_PROTO_ID;

	$tmplib = new fAssocUpdate();
	$tmplib->setObj( $sqlo, 'CONCRETE_PROTO_STEP', $CONCRETE_PROTO_ID );
	if ( $error->Got(READONLY) )  {
		return -1;
	}
	
	// delete all steps
	$tmplib->delMore($sqlo);
	
	$iniarr = array( 'STEP_NR'=>10, 'ABSTRACT_PROTO_ID'=>$ABSTRACT_PROTO_ID );
	$pkfull = array_merge( array('CONCRETE_PROTO_ID'=>$CONCRETE_PROTO_ID ), $iniarr);

	$firstargu = array( 'QUANTITY'=>5.1, 'NOTES'=>'hallo' );
	$tmplib->updateInsert( $sqlo, $firstargu, $iniarr);
	if ( $error->Got(READONLY) )  {
		return -1;
	}

	$testlib = new oCprotoMani();
	$testlib->setCprotoID ( $sqlo, $CONCRETE_PROTO_ID, $ABSTRACT_PROTO_ID );

	$step_nr=10;
	$newvals  = array("quantity"=>0.2, "notes"=>'hello', "subst"=>NULL );
	$newvalRaw= array("QUANTITY"=>0.2, "NOTES"=>'hello' );
	$result = $this->oneStepTest($testlib, $sqlo, '01 set', $step_nr, $newvals, $newvalRaw);
	

	$newvals  = array("quantity"=>NULL, "subst"=>NULL );
	$newvalRaw= array("QUANTITY"=>NULL, "CONCRETE_SUBST_ID"=>NULL, 'NOTES'=>'hello' );
	$result = $this->oneStepTest($testlib, $sqlo, '02 update some to NULL', $step_nr, $newvals, $newvalRaw, 1);
	
	
	$newvals  = array("quantity"=>NULL );
	$newvalRaw= array("QUANTITY"=>NULL, "CONCRETE_SUBST_ID"=>NULL, 'NOTES'=>NULL );
	$result = $this->oneStepTest($testlib, $sqlo, '03 update all to NULL', $step_nr, $newvals, $newvalRaw, 0);
	
	
	$step_nr  = 20;
	$newvals  = array("quantity"=>0.0, "notes"=>"next test" );
	$newvalRaw= array("QUANTITY"=>0.0, "CONCRETE_SUBST_ID"=>NULL, 'NOTES'=>'next test' );
	$result = $this->oneStepTest($testlib, $sqlo, '04 new step; quant=0', $step_nr, $newvals, $newvalRaw, 0);
	
	$SUB_TEST='05 no Step created';
	$step_nr  = 30;
	$pkfull['STEP_NR'] = $step_nr;
	$newvals  = array("quantity"=>NULL, "notes"=>"" );
	$newvalRaw= array("QUANTITY"=>NULL, "CONCRETE_SUBST_ID"=>NULL, 'NOTES'=>NULL );
	$result = $this->oneStepTest($testlib, $sqlo, $SUB_TEST, $step_nr, $newvals, $newvalRaw, 0);
	$rowHasData = $testlib->get_rowHasData();
	$tmp_result = gElement_exists2($sqlo, $mainTable, $pkfull);  // check if STEP exists
	
	
	$this->_infoNow( 'Test:'.$SUB_TEST, 'STEP_NR: '.$step_nr.' rowHasData:'.$rowHasData .' STEP-exists:'.$tmp_result);
	
	$testresult=0;
	$tmp_errtext='';
	if (!$tmp_result) {
	    $testresult=1;
	} else {
	    $tmp_errtext='Step exists, but should not exist.';
	}
	$this->_saveTestResult('Test:'.$SUB_TEST, $testresult, $tmp_errtext);
	
	
	
	// test, if INPUT: $new=NULL possible
	$step_nr  = 40;
	$newvals  = NULL;
	$newvalRaw= array("QUANTITY"=>NULL, "CONCRETE_SUBST_ID"=>NULL, 'NOTES'=>NULL, 'DEV_ID'=>NULL, 'NOT_DONE'=>NULL );
	$result = $this->oneStepTest($testlib, $sqlo, '06 St:1: empty step', $step_nr, $newvals, $newvalRaw, 0);
	
	
	// test, if INPUT: $new=NULL possible, step will be created
	$newvals  = NULL;
	$step_nr  = 60;
	$newvalRaw= array("QUANTITY"=>NULL, "CONCRETE_SUBST_ID"=>NULL, 'NOTES'=>NULL, 'DEV_ID'=>NULL, 'NOT_DONE'=>0 );
	$result = $this->oneStepTest($testlib, $sqlo, '07 St:2: step exists', $step_nr, $newvals, $newvalRaw, 1);
	
	$step_nr  = 50;
	$newvals  = array("quantity"=>1.5, "notes"=>"",   "CONCRETE_SUBST_ID"=>'00000' );
	$newvalRaw= array("QUANTITY"=>1.5, 'NOTES'=>NULL, "CONCRETE_SUBST_ID"=>NULL    );
	$result = $this->oneStepTest($testlib, $sqlo, '08 Subst-ID: 000000', $step_nr, $newvals, $newvalRaw, 0);
	
	$testlib->close($sqlo);

	$retval = 1;
	return $retval;
}

}