<?php
// example for testing: o.CHIP_READER.serviceInfoRule.inc
require_once ("f.assocUpdate.inc");
require_once ("insertx.inc");

class UT_o_CHIP_READER_serviceInfoRule_inc extends gUnitTestSub {
	
    
    function dotest( &$sqlo, $options ) {
    
    	global $error;
    	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
    	
    	
    	
    	$DB_USER_ID  = $this->_getDefObjID( "DB_USER", "default" );
    	$this->_infoNow('USER_ID', $DB_USER_ID);
    	
    	$A_CHIP_READER_ID      = $this->_getDefObjID( "A_CHIP_READER", "default" );
    	// $CHIP_READER_ID = $this->_getDefObjID( "CHIP_READER", "qc_service_test" );
    	
    	
    
    	$DEV_NAME='UnitTest 001 o_CHIP_READER_serviceInfoRule';
    	$ins_params= array( 
    		"vals"=>array(
    		    'NAME'=>$DEV_NAME,
    		    "A_CHIP_READER_ID"=>$A_CHIP_READER_ID,
    			'RES_USER_ID'=>$DB_USER_ID,
    			'IS_REGUL'=>1
    		), 
    	);
    	$CHIP_READER_ID = $this->_createObj_checkExists($sqlo, 'CHIP_READER', 'NAME', $DEV_NAME, $ins_params);
    	if (!$CHIP_READER_ID) {
    		$error->set( $FUNCNAME, 1, 'could not create CHIP_READER.' );
    		return;
    	}
    	
    	$this->_infoNow('CHIP_READER_ID', $CHIP_READER_ID);
    	
    	//$contact_name = glob_elementDataGet( $sqlo, 'CONTACT', 'CONTACT_ID', $DB_USER_ID, 'NAME');
    	$user_id = glob_elementDataGet( $sqlo, 'DB_USER', 'NICK', 'root', 'DB_USER_ID');
    	
    	// remove logs
    	$sqlo->Deletex('REA_LOG','CHIP_READER_ID='.$CHIP_READER_ID);
    	
    	// add log
    	// 
    	$targetdate_bad_UNX  = time() - date_unixDay2Sec(30);
    	$targetdate_good_UNX = time() - date_unixDay2Sec(40);
    	
    	
    	$argu1 = array(
    			'POS'  =>1,
    			'NAME' =>'Service UnitTest good',
    			'XDATE'=> date_unix2datestr($targetdate_good_UNX,1),
    	        'DB_USER_ID'=> $user_id,
    			'KEY' =>'Service',
    			'ACCEPT'=>4
    			
    			);
    	
    	$argu2 = array(
    			'POS'  =>2,
    			'NAME' =>'Service UnitTest bad',
    			'XDATE'=> date_unix2datestr($targetdate_bad_UNX,1),
    	        'DB_USER_ID'=> $user_id,
    			'KEY' =>'Service',
    			'ACCEPT'=>1
    				
    	);
    	
    	$assoclib = new  fAssocUpdate();
    	$assoclib->setObj( $sqlo, 'REA_LOG', $CHIP_READER_ID );
    	$assoclib->insert( $sqlo, $argu1 );
    	$assoclib->insert( $sqlo, $argu2 );
    	
    
    	$testLib = new oCHIP_READER_servicInfoRule($sqlo);
    	
    	$refDateUnx = time();
    	
    	
    	$expDevices = $testLib->getDevOneContact($sqlo, $DB_USER_ID, $refDateUnx );
    	
    	$this->_infoNow( 'list of urgent devices + dates', "<pre>".print_r($expDevices,1)."</pre>" ); // just info for the test
    	
    	$test_res=1;
    	if (!sizeof($expDevices)) {
    		$test_res = 0;
    	}
    	$this->_saveTestResult('01', $test_res, 'Expected min 1 entry.');
    	
    	// test 2
    	$refDateUnx = time() - date_unixDay2Sec(1); // now!
    	$infos = $testLib->oneDevice_serviceInfo1($sqlo, $CHIP_READER_ID, $refDateUnx);
    	$this->_infoNow( 'Test2: Device, Ref-Date:', 'DEV:'.$CHIP_READER_ID.'; '.date_unix2datestr($refDateUnx,4) );
    	$this->_infoNow( 'short device info', print_r($infos,1) );
    	
    	$retval = 1;
    	
    	return ($retval);
    }

}
