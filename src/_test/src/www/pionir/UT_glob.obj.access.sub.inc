<?php
/*
 * - check checkAuditState() 
*/

class UT_glob_obj_access_sub_inc extends gUnitTestSub {
	
function __construct() {

}

function _checkAction(&$sqlo, &$testLib, $tmparr) {
	
	$testLib->haslog = $tmparr['objSta'];
	
	$testLib->setRights = 'no';
	if ( $tmparr['objEnt'] ) {
		$testLib->setRights = 'entail';
	}
	
	if ( $tmparr['user']=='t') {
		$testLib->setRights = 'yes';
	}
	$testLib->set_objHasEntail( $tmparr['objEnt'] );
	
	// $this->_infoNow( 'Input:',  ': setRights: '.$testLib->setRights.' objHasEntail: ' .$testLib->objHasEntail. ' haslog:'.$testLib->haslog );
	/**
	 * INPUT: 
	 *   $this->haslog 
	 *   $this->setRights
	 *   $this->objHasEntail
	 * OUTPUT: $this->specialAct
	 */
	$testLib->checkAuditState($sqlo);
	
	$ok=1;
	if ($tmparr['act']=='reopen') {
		if ( $testLib->specialAct!='reopen') {
			$ok=0;
		}
	} else {
		if ($testLib->setRights != $tmparr['act']) {
			$ok=0;
		}
		
	}
	$this->_infoNow( 'answer:',  print_r($tmparr,1) . ': maniResult: '.$testLib->setRights.' actResult:'.$testLib->specialAct. '; ok: '.$ok );
	
	return $ok;
}

// return: 0 : not passed, 1: passed
function dotest( &$sqlo, $options ) {

	$objid = $this->_getDefObjID( "CONCRETE_SUBST", "default" );

	$testLib = new gObjAccessSub( $sqlo, 'CONCRETE_SUBST', $objid);
	
	/**
	 * 'user'=>
	 * 	  'n':normal,
	 *    't':table-admin,
	 * 'objEnt'=> has entail right ?
	 *   0: no
	 *   1: yes, 
	 * 'objSta'=> has audit status ?
	 *   0, 
	 *   1
	 * 'act'=> which action ...
	 *   'mani'
	 *   'reopen'
	 *   'no'
	 */
	$inputMatrix = array(
		array('user'=>'n', 'objEnt'=>1, 'objSta'=>0, 'act'=>'entail'),
		array('user'=>'n', 'objEnt'=>1, 'objSta'=>1, 'act'=>'entail'),
		array('user'=>'n', 'objEnt'=>0, 'objSta'=>0, 'act'=>'no'),
		array('user'=>'n', 'objEnt'=>0, 'objSta'=>1, 'act'=>'no'),
		array('user'=>'t', 'objEnt'=>1, 'objSta'=>0, 'act'=>'yes'),
		array('user'=>'t', 'objEnt'=>1, 'objSta'=>1, 'act'=>'yes'),
		array('user'=>'t', 'objEnt'=>0, 'objSta'=>0, 'act'=>'yes'),
		array('user'=>'t', 'objEnt'=>0, 'objSta'=>1, 'act'=>'reopen'),
	);
	
	
	$total=1;
	reset ($inputMatrix);
	foreach( $inputMatrix as $dummy=>$tmparr) {
		$answer_loop = $this->_checkAction($sqlo, $testLib, $tmparr);
		if (!$answer_loop) $total=0;
	}


	return ($total);
}

}
