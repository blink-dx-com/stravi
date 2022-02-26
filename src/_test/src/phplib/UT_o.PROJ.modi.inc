<?php
// 

class UT_o_PROJ_modi_inc extends gUnitTestSub {
	
function __construct() {

}

// return: 0 : not passed, 1: passed
function dotest( &$sql, $options ) {
	global $error;
	$FUNCNAME= ':dotest';
	
	$type = "abstract";
	$data = "";
	
	$PROJ_ID = $this->_getDefObjID( "PROJ", "default" );
	
	
	
	$this->_infoNow('MO', 'Mother-Project-ID', $PROJ_ID );
	$name='UnitTest 002_test';
	$modLib = new oPROJ_modi();
	$modLib->setProj( $sql, $PROJ_ID );
	$oldProjId = $modLib->getProjByName($sql, $name);
	
	if ($oldProjId) {
		// rename project
		require_once ("glob.obj.update.inc");
		$UpdateLib = new globObjUpdate();
		$args=array("vals"=>array('NAME'=>'UnitTest '.rand(1,1000000000)));
		$UpdateLib->update_meta( $sql, 'PROJ', $oldProjId, $args );
		
		if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 1, 'Project with name "'.$name.'" exists.' );
			return;
		}
	}
	
	$newProjId = $modLib->makeProj($sql, $name);
	$this->_infoNow( 'NEW', 'New Project-ID', $newProjId );
	
	if ($newProjId>0) $retval = 1;
	else  $retval = 0;
	
	return ($retval);
}

}