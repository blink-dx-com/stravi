<?php
require_once('insertx.inc');
require_once 'o.GLOBALS.mod.inc';
require_once('f.update.inc');

class UT_glob_obj_create_subs_inc extends gUnitTestSub {
	

function _creaSub(&$sqlo, $abstract_subst, $testName, $workflow_id_PLAN ) {
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
    // create
	$inslib = new insertC();
	$args=array(
		'vals' => array(
			'NAME'=>'unittest: UT_glob_obj_create_subs_inc:'.$testName,
			'ABSTRACT_SUBST_ID' => $abstract_subst
			)
	);
	$CONCRETE_SUBST = $inslib->new_meta( $sqlo, 'CONCRETE_SUBST', $args);
	
	if (!$CONCRETE_SUBST) {
		$error->set( $FUNCNAME, 1, 'can not create test-subst.' );
		return;
	}
	
	
	$crea2Lib = new gObjCreaSubs();
	
	$tmpNotes=NULL;
	$result = 0;
	$crea2Lib->addUserWorkflow($sqlo, "CONCRETE_SUBST", $CONCRETE_SUBST);
	if ($error->Got(READONLY))  {
		$allErrTxt = $error->getAllAsText();
		$tmpNotes = 'ERROR: '.$allErrTxt;
		$error->reset();
	} else {
		$result = 1;
	}
	$this->_saveTestResult($testName.':1', $result, $tmpNotes);
	
	$acc_info = access_data_get($sqlo, 'CONCRETE_SUBST', 'CONCRETE_SUBST_ID', $CONCRETE_SUBST);
	$workflow_id_NOW = $acc_info['wflow_id'];
	$this->_compareTestResult( $testName.':2', $workflow_id_PLAN, $workflow_id_NOW);
	
	$this->_infoNow( $testName, ':SUC-ID:'.$CONCRETE_SUBST.' Workflow_now:'.$workflow_id_NOW );

	
}

/* return: 0 : not passed, 1: passed
 * - create substance
 * - set USER-trigger = 0
 * - add trigger
 */
function dotest( &$sqlo, $options ) {
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	$tablename = 'CONCRETE_SUBST';
	$abstract_subst = $this->_getExistObjID('ABSTRACT_SUBST', 'default');
	
	// $workflow_global = 15;
	// $workflow_user   = 16;
	
	$workflow_name='UT_'.__CLASS__.':MOD1';
	$ins_params=array( 'vals'=> array(
			'NAME'=>$workflow_name,
			'LOCATION'=>'g.STANDARD1.inc',
			'TYPE'=>2
		)	
	);
	$workflow_global = $this->_createObj_checkExists($sqlo, 'MODULE', 'NAME', $workflow_name, $ins_params);
	$this->_infoNow('workflow_global', $workflow_global);
	
	$workflow_name='UT_'.__CLASS__.':MOD2';
	$ins_params=array( 'vals'=> array(
			'NAME'=>$workflow_name,
			'LOCATION'=>'g.STANDARD1.inc',
			'TYPE'=>2
	)
	);
	$workflow_user = $this->_createObj_checkExists($sqlo, 'MODULE', 'NAME', $workflow_name, $ins_params);
	$this->_infoNow('workflow_user', $workflow_user);
	
	// start tests

	// test 1
	$glob_workflow_KEY  = 'o.CONCRETE_SUBST.initTriggerID';
	oGLOBALS_mod::update($sqlo, $glob_workflow_KEY, NULL); // unset global workflow ...
	
	$_SESSION['userGlob']["o.".$tablename.".fCreaOpt"] = NULL;
	$this->_creaSub($sqlo, $abstract_subst, '01:no workflow', NULL);

	
	// test 2
	// set global workflow
	oGLOBALS_mod::update($sqlo, $glob_workflow_KEY, $workflow_global);

	$_SESSION['userGlob']["o.".$tablename.".fCreaOpt"] = NULL;
	$this->_creaSub($sqlo, $abstract_subst, '02:global workflow', $workflow_global);
	
	
	
	// test 3
	$var  = 'o.CONCRETE_SUBST.initTriggerID';
	$args = array('NAME'=>$var, 'VALUE'=>NULL);
	gObjUpdate::update_row($sqlo, 'GLOBALS', $args);
	$_SESSION['userGlob']["o.".$tablename.".fCreaOpt"] = serialize( array('trigger'=>$workflow_user) );
	$this->_creaSub($sqlo, $abstract_subst, '03:user workflow', $workflow_user);
	
	
	return (1);
}

}
