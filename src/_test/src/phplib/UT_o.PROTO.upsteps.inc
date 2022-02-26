<?php


class UT_o_PROTO_upsteps_inc extends gUnitTestSub {
	


/**
 * one test
 * @param $testName e.g. 01
 * @param  $newvalRaw  array( STEPNR=> array("QUANTITY"=>0.2, "NOTES"=>'hello') );
 * @param  $compareVal array( STEPNR=> array("QUANTITY"=>0.2, "NOTES"=>'hello') );
 * @reutrn >0 o.k.
 */
function oneStepTest(&$testlib, &$sqlo, $testName, $newvalRaw, $expectData ) {
	global $error;
	$mainTable = 'CONCRETE_PROTO_STEP';
	
	$ABSTRACT_PROTO_ID = $this->ABSTRACT_PROTO_ID;
	$CONCRETE_PROTO_ID = $this->CONCRETE_PROTO_ID;
	
	$this->_infoNow( 'Test:'.$testName, '');
	
	$this->_infoNow( '- UPDATE-vals: ', glob_array2String( $newvalRaw ) );
	$testlib->protoUpdate($sqlo, $ABSTRACT_PROTO_ID, $newvalRaw );
	if ( $error->Got(READONLY) )  {
		return -2;
	}
	
	
	$diff_sum= array();
	$allkeys = array('CONCRETE_SUBST_ID', 'QUANTITY', ' DEV_ID', 'NOT_DONE', 'NOTES' );
	
	// check number of steps
	$expect_steps = array_keys($expectData);
	// get current keys
	$sqlsel = "STEP_NR from ".$mainTable." where CONCRETE_PROTO_ID=".$CONCRETE_PROTO_ID. ' order by STEP_NR';
	$sqlo->Quesel($sqlsel);
	$proto_steps=array();
	while ( $sqlo->ReadRow() ) {
		$stepnr = $sqlo->RowData[0];
		$proto_steps[]=$stepnr;
		
		
	}
	
	if ( sizeof($proto_steps) != sizeof($expect_steps) ) {
		$this->_saveTestResult($testName, 0, 'Different step numbers! is:'.sizeof($proto_steps).'; expect:'.sizeof($expect_steps) );
		return;
	}
	
	// check for same step numbers
	foreach ($expect_steps as $stepnr) {
		if ( !in_array($stepnr, $proto_steps) ) {
			$this->_saveTestResult($testName, 0, 'Expected Step '.$stepnr.' not found.' );
			return;
		}
	}
	
	foreach($expectData as $step => $expectRow) {
		
		// $askKeys = array_keys($compareVal);
		
		$pkfull    = array('CONCRETE_PROTO_ID'=>$CONCRETE_PROTO_ID, 'STEP_NR'=>$step);
		$step_vals = glob_elemDataGet3($sqlo, $mainTable, $pkfull, $allkeys );
		
		$allkeys_more=$allkeys;
		$allkeys_more[]='STEP_NR';
		$step_vals_out = glob_elemDataGet3($sqlo, $mainTable, $pkfull, $allkeys_more );
		$this->_infoNow( '- EXPECT-Step_data: ST:'.$step.' ', glob_array2String( $expectRow ) );
		$this->_infoNow( '- Step_data', glob_array2String( $step_vals_out ) );
		
		$diffarr = $this->_compare_Array_floatVals( $expectRow, $step_vals );
		if ( is_array($diffarr) ) {
			$diff_sum[$step] = $diffarr;
		}
		
	}
	
	$test_result = 1;
	$test_notes=NULL;
	if ( sizeof($diff_sum) ) {
		$this->_infoNow( 'DiffElements', 'check element, diff: '.glob_array2String( $diff_sum ) );
		$test_result = 0;
		$test_notes = 'number of differences: '.sizeof($diff_sum);
	}
	$this->_saveTestResult($testName, $test_result, $test_notes);

}

// return: 0 : not passed, 1: passed
function dotest( &$sqlo, $options ) {
	global $error;

	$retval = 0;
	$mainTable = 'CONCRETE_PROTO_STEP';
	// $CONCRETE_PROTO_ID = 514660 ; // $this->_getDefObjID( "CONCRETE_PROTO", "default" );
	$ABSTRACT_PROTO_ID = $this->_getExistObjID( "ABSTRACT_PROTO", "default" );
	if (!$ABSTRACT_PROTO_ID) return -1;
	
	// create a new protocol
	$args = array('vals'=>array('ABSTRACT_PROTO_ID'=>$ABSTRACT_PROTO_ID, 'NAME'=>'UT '.__CLASS__) );
	
	$obj_is_id = glob_elementDataGet( $sqlo, 'CONCRETE_PROTO', 'NAME', $args['vals']['NAME'], 'CONCRETE_PROTO_ID');
	if ($obj_is_id) $CONCRETE_PROTO_ID = $obj_is_id;
	else $CONCRETE_PROTO_ID = $this->_createObj( "CONCRETE_PROTO", $args );
	if (!$CONCRETE_PROTO_ID) return -1;
	
	$this->_infoNow( 'Input-Data:',"CONCRETE_PROTO_ID: ".$CONCRETE_PROTO_ID);
	
	$this->SUC_id = $this->_getExistObjID( "CONCRETE_SUBST", "default" );

	
	$this->ABSTRACT_PROTO_ID=$ABSTRACT_PROTO_ID;
	$this->CONCRETE_PROTO_ID=$CONCRETE_PROTO_ID;

	$assocObjLib = new fAssocUpdate();
	$assocObjLib->setObj( $sqlo, 'CONCRETE_PROTO_STEP', $CONCRETE_PROTO_ID );
	if ( $error->Got(READONLY) )  {
		return -1;
	}
	
	// delete all steps
	$assocObjLib->delMore($sqlo);
	
	
	
	$id_arr = array( 'STEP_NR'=>10, 'ABSTRACT_PROTO_ID'=>$ABSTRACT_PROTO_ID );
	$firstargu = array( 'QUANTITY'=>5.1, 'NOTES'=>'hallo_INIT' );
	$assocObjLib->updateInsert( $sqlo, $firstargu, $id_arr);
	if ( $error->Got(READONLY) )  {
		return -1;
	}

	$testlib = new oConcProtoUpSteps();
	$testlib->setProto ( $CONCRETE_PROTO_ID );

	/**
	 * 
	 * @var array $newvalRaw
	 * 
	 * STEP_NR => array ( data )
	 */
	$newvalRaw = array(
		10=> array("QUANTITY"=>0.1, "NOTES"=>'hello1', 'CONCRETE_SUBST_ID'=>$this->SUC_id) ,
		20=> array("QUANTITY"=>0, "NOTES"=>'hello2', 'NOT_DONE'=>1) ,
		30=> array("QUANTITY"=>0.2, "NOTES"=>'hello10')
	);
	$expectData = $newvalRaw;
	
	$this->oneStepTest($testlib, $sqlo, '01 normal'   ,$newvalRaw, $expectData);
	
	$newvalRaw2 = array(
			10=> array("QUANTITY"=>0.1, "NOTES"=>'hello1', 'CONCRETE_SUBST_ID'=>NULL) ,
			20=> array("QUANTITY"=>NULL, ) ,
	);
	$expectData[10]['CONCRETE_SUBST_ID']=NULL;
	$expectData[20]['QUANTITY']=NULL;
	$this->oneStepTest($testlib, $sqlo, '02 unset SUC',$newvalRaw2, $expectData);
	

	$retval = 1;
	return $retval;
}

}