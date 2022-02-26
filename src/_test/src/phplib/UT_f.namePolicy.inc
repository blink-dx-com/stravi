<?php
require_once 'o.GLOBALS.mod.inc';
// example for testing: 

class UT_f_namePolicy_inc extends gUnitTestSub {


function dotest( &$sqlo, $options ) {
    global $error;
	$retval = 1;
	
	$sua_id    = $this->_getDefObjID( "ABSTRACT_SUBST", "default" );

	$testLib   = new fNamePolicy('CONCRETE_SUBST', $sua_id);
	$timeStamp = mktime(0,0,0,1,1,2002);
	$start_val = 345;
	$paramarr=array('NUM'=>345, 'DATE'=>$timeStamp, 'ID'=>'???' );
	
	$this->_infoNow( 'OBJECT: ', 'for CONCRETE_SUBST; SUA-ID:'.$sua_id .' START-VAL:'.$start_val); 
	$testLib->set_ABS_next_val($sqlo, $start_val);
	
	// set start-val
	$gnum_startval=3;
	$GENID=22;
	$genkey= fNamePolicy::GNUM_GEN_KEY. $GENID;
	oGLOBALS_mod::update($sqlo, $genkey, $gnum_startval);
	
	$policyInArr =array( 
		array('input'=>'{NUM,04}_yippie_{DATE}'   , 'expect'=>'0345_yippie_20020101'),
	    array('input'=>'{NUM,05}_yippie_{DATE,2}' , 'expect'=>'00346_yippie_020101'),
	    array('input'=>'test2_{NUM,06}_yippie_{DATE,3}', 'expect'=>'test2_000347_yippie_2002-01-01'),
	    array('input'=>'test2_{NUM,06}_yippie_{DATE,3}-{ID}', 'expect.PRE'=>'test2_000348_yippie_2002-01-01-XXXX',
	        'expect.POST'=>'test2_000348_yippie_2002-01-01-???',
	    ),
	    array('input'=>'test3-{GNUM,03,22}', 'expect'=>'test3-003'),
	    array('input'=>'test3-{GNUM,03,22}', 'expect'=>'test3-004'),  // use same generator
	);
	
	$cnt=1;
	foreach( $policyInArr as $row) {
	    $policyStr = $row['input'];
	    $expect_PRE    = $row['expect'];
	    if (!empty($row['expect.PRE'])) $expect_PRE    = $row['expect.PRE'];
	    
	    $policyArray = $testLib->analysePolicyString($policyStr);
	    $val_result  = $testLib->generate_name_PRE($sqlo, $policyArray, $paramarr);
	    $this->_infoNow( 'test_PRE', 'input:'.$policyStr.' expect:'.$expect_PRE.' out:'.$val_result. ' policyArray:'.print_r($policyArray,1) ); // just info for the test
		if ($error->Got(READONLY))  {
		    $errLast   = $error->getLast();
		    $error_txt = $errLast->text;
		    $error_id  = $errLast->id;
		    $error->reset();
		    $this->_infoNow( 'test', 'Error: '. $error_txt ); 
		}
		$this->_compareTestResult('TEST'.$cnt.'_PRE', $expect_PRE, $val_result, $val_result);
		
		$expect_POST    = $row['expect'];
		if (!empty($row['expect.POST'])) $expect_POST    = $row['expect.POST'];
		$val_result  = $testLib->generate_name_POST($sqlo, $policyArray, $paramarr);
		$this->_infoNow( 'test_POST', 'input:'.$policyStr.' expect:'.$expect_POST.' out:'.$val_result. ' policyArray:'.print_r($policyArray,1) ); // just info for the test
		if ($error->Got(READONLY))  {
		    $errLast   = $error->getLast();
		    $error_txt = $errLast->text;
		    $error_id  = $errLast->id;
		    $error->reset();
		    $this->_infoNow( 'test', 'Error: '. $error_txt );
		}
		$this->_compareTestResult('TEST'.$cnt.'_POST', $expect_POST, $val_result, $val_result);
		
		$cnt++;
	}
	
	
	return ($retval);
}

}
