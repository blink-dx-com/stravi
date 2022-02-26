<?php
require_once('o.PROJ.modi.inc');
require_once 'o.PROJ.addelems.inc';

class UT_o_PROJ_subs_inc extends gUnitTestSub {
	

// return: 0 : not passed, 1: passed
function dotest( &$sqlo, $options ) {

	$ok=1;
	$PROJ_ID = $this->_getDefObjID( "PROJ", "default" );
	$SUA_ID  = $this->_getDefObjID( "ABSTRACT_SUBST", "default" );
	
	$name='FREE';
	$name2 = 'UnitTest_PROJ_p2';
	$modLib = new oPROJ_modi();
	$modLib->setProj( $sql, $PROJ_ID );
	$proj_id_temp = $modLib->getProjByName($sqlo, $name);
	if (!$proj_id_temp) {
		$newProjId = $modLib->makeProj($sqlo, $name); // create the project
	}
	
	$newProjId2 = $modLib->getProjByName($sqlo, $name2);
	if (!$newProjId2) {
		$newProjId2 = $modLib->makeProj($sqlo, $name2); // create the project
	}

	$testLib = new cProjSubs();
	$answer = $testLib->getProjByName($sqlo, $PROJ_ID, $name);
	if ($answer<=0) $ok = 0;
	$this->_info( 'project exists', $answer ); // just info for the test
	
	$answer = $testLib->getProjByName($sqlo, $PROJ_ID, 'not exists');
	if ($answer>0) $ok = 0;
	$this->_info( 'project not exists', $answer ); // just info for the test
	
	$answer = $testLib->getProjByName($sqlo, 783783747545, $name);
	if ($answer>0) $ok = 0;
	$this->_info( 'mother-proj not exists', $answer ); // just info for the test
	
	// add exp to TWO prpjects
	$projElem_lib = new oProjAddElem($sqlo, $PROJ_ID);
	$projElem_lib->addObj($sqlo, "ABSTRACT_SUBST", $SUA_ID);
	
	$projElem_lib = new oProjAddElem($sqlo, $newProjId2);
	$projElem_lib->addObj($sqlo, "ABSTRACT_SUBST", $SUA_ID);
	
	$answer = $testLib->getProjectsByObject($sqlo, "ABSTRACT_SUBST", $SUA_ID);
	$this->_info( 'SUA '.$SUA_ID.' found in projects', print_r($answer,1) );
	
	if (sizeof($answer)>=2) {
		
	} else {
		$ok = 0;
	}
	
	if ($ok>0) $retval = 1;
	else  $retval = 0;
	
	return ($retval);
}

}
