<?php
//
require_once('insertx.inc');
require_once ("o.S_VARIO.mod.inc");

class UT_o_CONTAINER_mod_inc extends gUnitTestSub {
	
function __construct() {

}

/**
 * create data
 * @param  $sqlo
 */
function _createData(&$sqlo) {
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	// $ABS_CONT_ID = $this->_getExistObjID( "ABS_CONTAINER", "default" );
	$ABSTRACT_SUBST_ID  = $this->_getExistObjID( "ABSTRACT_SUBST", "default" );
	
	// create object
	$inslib = new insertC();
	
	$tmp_name='unittest: UT_o_CONTAINER_mod_inc';
	$args=array(
			'vals' => array(
			    'NAME'=>	$tmp_name
			)
	);
	
	$ABS_CONT_ID = $this->_createObj_checkExists($sqlo, 'ABS_CONTAINER', 'NAME', $tmp_name, $args);
	if (!$ABS_CONT_ID) {
		$error->set( $FUNCNAME, 1, 'can not create test-container.' );
		return;
	}
	// remove old entries
	$sqlo->Deletex('ABS_CONT_ENTRY', 'ABS_CONTAINER_ID='.$ABS_CONT_ID);
	
	$rawInsert = new insertRawC();
	
	$insdata= array();
	for ($i=1; $i<20; $i++) {
		$insdata[] = array( 'POS'=>$i, 'SHELF'=>$i, 'RACK'=>1, 'RACK'=>1 );
	}
	
	$argu_fix = array(
		'ABS_CONTAINER_ID'=>$ABS_CONT_ID
	);
	foreach( $insdata as $key=>$valarr) {
		$argu = $valarr;
		$argu = array_merge($argu, $argu_fix);
		$rawInsert->insertR( $sqlo, 'ABS_CONT_ENTRY', $argu, 'ABS_CONTAINER_ID');
	}
	if ($error->Got(READONLY))  {
		$error->set( $FUNCNAME, 5, 'error' );
		return;
	}
	
	$tmp_name='unittest: UT_o_CONTAINER_mod_inc';
	$args=array(
		'vals' => array(
		    'NAME'=>$tmp_name,
			'ABS_CONTAINER_ID' => $ABS_CONT_ID
			)
	);
	$CONT_ID = $this->_createObj_checkExists( $sqlo, 'CONTAINER', 'NAME', $tmp_name, $args);
	if (!$CONT_ID) {
		$error->set( $FUNCNAME, 1, 'can not create test-container.' );
		return;
	}
	
	// VARIO: activate CONTAINER log
	$varioModLib = new oS_VARIO_mod();
	$varioModLib->setObject( $sqlo, 'CONTAINER', $CONT_ID );
	$varioModLib->updateKeyVal($sqlo, 'log.active', 1);
	$varioModLib->updateKeyVal($sqlo, 'storePointer', NULL);
	
	// remove old entries
	$sqlo->Deletex('CONT_HAS_CSUBST', 'CONTAINER_ID='.$CONT_ID);
	
	$tmpname='unittest: UT_o_CONTAINER_mod_inc';
	$args=array(
		'vals' => array(
		    'NAME'=>$tmpname,
			'ABSTRACT_SUBST_ID' => $ABSTRACT_SUBST_ID
			)
	);
	
	if ($error->Got(READONLY))  {
	   return;
	}
	
	$test_subst_id = $this->_createObj_checkExists( $sqlo, 'CONCRETE_SUBST', 'NAME', $tmpname, $args);
	if (!$test_subst_id) {
		$error->set( $FUNCNAME, 1, 'can not create test-substance.' );
		return;
	}
	$this->test_SUC = $test_subst_id;
	
	// prepare data
	$insdata = array(
		array( 'POS'=>1, 'CONCRETE_SUBST_ID'=>$test_subst_id, 'RESERVED'=>0),
		array( 'POS'=>2, 'CONCRETE_SUBST_ID'=>$test_subst_id, 'RESERVED'=>0),
		array( 'POS'=>3, 'CONCRETE_SUBST_ID'=>$test_subst_id, 'RESERVED'=>0),
		array( 'POS'=>4, 'CONCRETE_SUBST_ID'=>NULL, 'RESERVED'=>1),
		array( 'POS'=>5, 'CONCRETE_SUBST_ID'=>NULL, 'RESERVED'=>1),
		array( 'POS'=>6, 'CONCRETE_SUBST_ID'=>NULL, 'RESERVED'=>0),
		array( 'POS'=>7, 'CONCRETE_SUBST_ID'=>NULL, 'RESERVED'=>0),
		array( 'POS'=>8, 'CONCRETE_SUBST_ID'=>NULL, 'RESERVED'=>0),
		array( 'POS'=>9, 'CONCRETE_SUBST_ID'=>$test_subst_id, 'RESERVED'=>0)
	);
	
	$argu_fix = array(
		'CONTAINER_ID'=>$CONT_ID
	);
	
	$rawInsert = new insertRawC();
	
	foreach( $insdata as $key=>$valarr) {
		$argu = $valarr;
		$argu = array_merge($argu, $argu_fix);
		$rawInsert->insertR( $sqlo, 'CONT_HAS_CSUBST', $argu, 'CONTAINER_ID');
	}
	
	
	return $CONT_ID;
}

// return: 0 : not passed, 1: passed
function dotest( &$sqlo, $options ) {
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	$testArray = array(
		array('pos'=>0, 'expectPos'=>6),
		array('pos'=>1, 'expectPos'=>6),
		array('pos'=>6, 'expectPos'=>6),
		array('pos'=>7, 'expectPos'=>7),
		array('pos'=>9, 'expectPos'=>10),
	);
	
	$CONT_ID = $this->_createData($sqlo);
	if ($error->Got(READONLY))  {
		$error->set( $FUNCNAME, 1, 'error on data creation' );
		return;
	}
	
	$this->_infoNow( 'test method: ', 'getFreePos()' );
	$this->_infoNow( 'container-ID ', $CONT_ID);

	$modparx = array();
	$testLib = new oContainerModC($modparx);
	$testLib->setContainer($sqlo, $CONT_ID);
	
	$retval = 1;
	
	reset ($testArray);
	foreach( $testArray as $key=>$valarr) {
		$startpos =  $valarr['pos'];
		$expect   =  $valarr['expectPos'];
		$answer = $testLib->getFreePos($sqlo, $startpos);
		$ok = 1;
		if ($answer != $expect) {
			$retval = 0;
			$ok=0;
		}
		$this->_infoNow( 'TEST01_getpos', 'startPos:'.$startpos. ' result:'.$answer. ' expect:'.$expect.' ok? '. $ok);
	}
	
	
	// only look for syntax error
	$startpos=1;
	$answer = $testLib->getFreePosAndAbs($sqlo, $startpos);
	if ($error->Got(READONLY))  {
		$error->set( $FUNCNAME, 1, 'error on getFreePosAndAbs()' );
		return;
	}
	
	$testLib->activateModi($sqlo);
	
	$pos = $testLib->getFreePos($sqlo);
	$args = array(
	    "CONCRETE_SUBST_ID"=>$this->test_SUC,
	    "ALIQUOT_NO"=>1,
	    "NOTES"=>'ahllo',
	    "QUANTITY"=>5.2
	);
	$testLib->addAliquot($sqlo, $pos, $this->test_SUC, 1, 5.2);
	
	$this->_infoNow( 'TEST02_new', 'POS: '.$pos);
	
	
	return ($retval);
}

}
