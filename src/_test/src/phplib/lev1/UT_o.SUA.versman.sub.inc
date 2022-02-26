<?php
//  o.SUA.versman.sub.inc

class UT_o_SUA_versman_sub_inc extends gUnitTestSub {
	
function __construct() {

}

// return: 0 : not passed, 1: passed
function dotest( &$sqlo, $options ) {

	$SUA_ID = 157667;// $this->_getDefObjID( "ABSTRACT_SUBST", "default" );

	$testLib = new oSUA_versman_child();
	$answer = $testLib->setChild($sqlo, $SUA_ID);
	$flag = $testLib->getValidFlag();
	$feats = $testLib->getVersFeats();
	$this->_infoNow( 'SUA:'.$SUA_ID, print_r($feats,1).' FLAG:'.$flag ); // just info for the test
	
	$retval = 1;
	
	return ($retval);
}

}
