<?php

class UT_f_clipboard_inc extends gUnitTestSub {
	


    // return: 0 : not passed, 1: passed
    function dotest( &$sqlo, $options ) {
    
        $tablename='EXP';
        $objid = 100;
    
        $testLib = new clipboardC();
        $testLib->resetx();
        
        $answer  = $testLib->obj_addone ( $tablename, $objid );
    	
        $this->_infoNow( 'INPUT: ', $tablename.':'.$objid ); // just info for the test
    	$answer = $testLib->getcurrElem();
    	$result=1;
    	if ( $answer[0]!=$tablename or $answer[1]!=$objid ) $result=0;
    	$this->_saveTestResult('TEST01:getcurrElem ', $result,  print_r($answer,1)); 
    	
    	
    	$tablename='CONCRETE_SUBST';
    	$objid = 101;
    	$this->_infoNow( 'INPUT: ', $tablename.':'.$objid ); // just info for the test
    	$answer  = $testLib->obj_addone ( $tablename, $objid );
    	$answer = $testLib->getcurrElem();
    	$result=1;
    	if ( $answer[0]!=$tablename or $answer[1]!=$objid ) $result=0;
    	$this->_saveTestResult('TEST02:getcurrElem ', $result,  print_r($answer,1)); 
    	
    	
    	$answer  = $testLib->getClipBoard();
    	$this->_infoNow( 'ALL_clipboard: ', print_r($answer,1) );
    	
    	
    	$tablename='EXP';
    	$objid = 103;
    	$answer  = $testLib->obj_addone ( $tablename, $objid );
    	$answer  = $testLib->getAllObjects($tablename);
    	
    	$expect = array(100,103);
    	
    	$diff_val = $this->_compareArrayVals($answer, $expect);
    	$result=0;
    	if ($diff_val==NULL) $result=1;
    	
    	$this->_saveTestResult('TEST03:getAllObjects ', $result, print_r($diff_val,1) ); 
    	
    	
    	$answer  = $testLib->obj_get( 'EXP' );
    	$result=0;
    	if ($answer[0]>0) $result=1;
    	$this->_saveTestResult('TEST04:obj_get ', $result, print_r($answer,1) ); 
    	
    	
    	// next ....
    	$expect = array(
    	    array('EXP',100),
    	    array('CONCRETE_SUBST',101),
    	    array('EXP',103),
    	);
    	
    	$testLib->reset();
    	$cnt=0;
    	while ( $elemArr = $testLib->getcurrElem()  ) {
    	    
    	    $result=0;
    	    if ( $elemArr[0]==$expect[$cnt][0] and  $elemArr[1]==$expect[$cnt][1]) $result=1;
    	        
    	    $this->_saveTestResult('TEST05:getcurrElem:'.$cnt, $result, print_r($elemArr,1) ); 
    	     
    	    $cnt++;
    	}
    	
    	return (1);
    }

}
