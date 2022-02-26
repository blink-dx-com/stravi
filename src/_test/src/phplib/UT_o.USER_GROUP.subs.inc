<?php
// test for /group/it/cvs/partisan/phplib/o.USER_GROUP.subs.inc

class UT_o_USER_GROUP_subs_inc extends gUnitTestSub {
	
function __construct() {

}

// return: 0 : not passed, 1: passed
function dotest( &$sqlo, $options ) {

	$groupid = $this->_getDefObjID( "USER_GROUP", "default" );
	
	$sqlsel = 'count(1) from DB_USER_IN_GROUP where USER_GROUP_ID='.$groupid;
	$sqlo->Quesel($sqlsel);
	$sqlo->ReadRow();
	$numusers = $sqlo->RowData[0];
	
	if (!$numusers) {
		// add one user
		// $_SESSION['sec']['db_user_id'] = 1;
	    $argu = array('USER_GROUP_ID'=>$groupid, 'DB_USER_ID'=>$_SESSION['sec']['db_user_id']);
	    insert_row_s($sqlo, 'DB_USER_IN_GROUP', $argu);
		
	}

	$testLib = new oUSER_GROUP_subs();
	$answer = $testLib->getUsersOfGroup($sqlo, $groupid);
	
	$this->_info( 'answer', 'USER_GROUP:'.$groupid.' answer: '.print_r($answer,1) ); // just info for the test
	
	if ( is_array($answer) and sizeof($answer) ) $retval = 1;
	else  $retval = 0;
	$this->_saveTestResult('TEST01', $retval, 'number of users: '.sizeof($answer) );
	
	return 1;
}

}
