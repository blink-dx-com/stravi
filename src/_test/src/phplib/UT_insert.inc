<?php
require_once 'o.DB_USER.subs2.inc';
require_once('f.update.inc');

class UT_insert_inc extends gUnitTestSub {
	
private function create_init_data(&$sqlo) {
	// create a user or take user
	
    $user_id    = $this->_getDefObjID( "DB_USER", "default" );
    $group_id   = $this->_getDefObjID( "USER_GROUP", "default" );
	
	
	// activate group
	$argu=array('USER_GROUP_ID'=>$group_id, 'INACTIVE'=>0);
	gObjUpdate::update_row($sqlo, 'USER_GROUP', $argu);
	
	// add group to user preferences
	$umask = 'active select update insert delete entail';
	oDB_USER_sub2::writeUmask ( $sqlo, $user_id, $group_id, $umask);
	
	$result = array(
			'user_id'=>$user_id,
			'group_id'=>$group_id
	);
	
	return $result;
}

/**
 * - check userActiveGroupsGet()
 * @see gUnitTestSub::dotest()
 */
function dotest( &$sqlo, $options ) {
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	
	//
	// TEST_A01: test method insert_row_Get
	//
	$test_name='TEST A01 insert_row_Get()';
	$tablename='CONCRETE_PROTO';
	$argu  = array('ABSTRACT_PROTO_ID'=>10, 'NAME'=>'UNIT_TEST');
	$expect = "(ABSTRACT_PROTO_ID, NAME) VALUES ('10', 'UNIT_TEST')";
	
	$result = insert_row_Get( $sqlo, $tablename, $argu );

	
	$this->_infoNow('insert_row_Get:', $result);
	$this->_compareTestResult($test_name, $expect, $result);

	// create one object
	$argu = array('NAME'=> $FUNCNAME);
	$doc_id = insert_row($sqlo, 'LINK', $argu);
	if ($error->Got(READONLY))  {
	    $error->set( $FUNCNAME, 1, 'Insert of LINK-object failed' );
	    return;
	}
	$this->_infoNow('02: created LINK object: ID:', $doc_id);
	
	$object_infos = $this->create_init_data($sqlo);
	$user_id = $object_infos['user_id'];
	$group_id = $object_infos['group_id'];
	
	$this->_infoNow('USER-ID', $user_id);
	$this->_infoNow('GROUP-ID', $group_id);
	
	if (!$user_id or !$group_id) {
		$error->set( $FUNCNAME, 2, 'Initial data not exists.' );
		return;
	}
	
	
	//
	// TEST01: test for an activated group
	//
	
	// get active groups
	$info_arr = userActiveGroupsGet( $sqlo, $user_id);
	
	// analyse result
	$detect_group_row = $info_arr[$group_id];
	$tmp_result=0;
	if (is_array($detect_group_row)) {
		$tmp_result=1;
	}
	$this->_saveTestResult('TEST01_group_entry_exists', $tmp_result);
	
	$tmp_result=0;
	if ($detect_group_row['update']==1) {
		$tmp_result=1;
	}
	$this->_saveTestResult('TEST01_group_data', $tmp_result);
	
	//
	// TEST02: test for an deactivated group
	//
	// deactivate the group
	$argu=array('USER_GROUP_ID'=>$group_id, 'INACTIVE'=>1);
	gObjUpdate::update_row($sqlo, 'USER_GROUP', $argu);
	
	
	// get active groups
	$info_arr = userActiveGroupsGet( $sqlo, $user_id);
	// analyse result, group shopuld not appear now
	$detect_group_row = $info_arr[$group_id];
	$tmp_result=0;
	if (!is_array($detect_group_row)) { // check, that it NOT exists
		$tmp_result=1;
	}
	$this->_saveTestResult('TEST02_group_entry_hidden', $tmp_result);
	
	
	return 1;
}

}