<?php
require_once 'o.WORKLIST.mod.inc';
require_once ("insertx.inc");


class UT_o_WORKLIST_subs_inc extends gUnitTestSub {
	


// return: 0 : not passed, 1: passed
function dotest( &$sqlo, $options ) {
    global $error;
    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
    
    

    $WL_name= __CLASS__;
	$WLA_ID = $this->_getExistObjID('ABS_WORKLIST', 'default');
	
	$test_object_id = 123456789; // this object is artificial and is not required to really  exist in the system
	
	$wli_id   = glob_elementDataGet( $sqlo, 'WORKLIST', 'NAME', $WL_name, 'WORKLIST_ID');
	
	if (!$wli_id) {
		// create document
		$insertlib = new insertC();
		$args = array(
				"vals"=> array(
						'NAME'=>$WL_name,
						'ABS_WORKLIST_ID'=>$WLA_ID
				)
			);
			
		$wli_id   = $insertlib->new_meta($sqlo, 'WORKLIST', $args);
	}
	$this->_infoNow('Worklist', 'WLI:'.$wli_id);
	
	$wl_mod_lib = new oWORKLIST_mod();
	$wl_mod_lib->setObj($sqlo, $wli_id);
	
	$params=array('TABLE_NAME'=>'EXP', 'OBJID'=>$test_object_id);
	$wl_mod_lib->updateCell($sqlo, array(3,1), $params);
	$params=array('TABLE_NAME'=>'EXP', 'OBJID'=>2);
	$wl_mod_lib->updateCell($sqlo, array(3,2), $params);

	$testLib = new oWORKLIST_subs();
	$wl_list = $testLib->getWliOfObj($sqlo, 'EXP', $test_object_id );
	
	$testname='Test_WL_found';
	$this->_infoNow($testname, 'search for mother WL for EXP:'.$test_object_id);
	$this->_infoNow($testname, 'Found WLs:'.print_r($wl_list,1));
	do {
		if (!sizeof($wl_list)) {
			$this->_saveTestResult($testname, 0, 'No Mother-Worklist not found for EXP:'.$test_object_id);
			break;
		}
		if (!in_array($wli_id, $wl_list)) {
			$this->_saveTestResult($testname, 0, 'Mother-Worklist found, but not expected one for EXP:'.$test_object_id);
			break;
		}
		$this->_saveTestResult($testname, 1);
	} while (0);
	
	$retval = 1;
	
	return ($retval);
}

}
