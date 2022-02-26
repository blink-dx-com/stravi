<?php
// example for testing: csv.substance.lib.xarray.inc

class UT_object_info_inc extends gUnitTestSub {
	
function __construct() {

}

function _substest1(&$sqlo, &$libx, $testcnt, $rawoption) {
  $table='CONCRETE_SUBST';
  $answer = $libx->getParamsRaw($sqlo, $table, $this->OBJ_ID, $rawoption );
  $this->_infoNow( $testcnt.'. input',  print_r($rawoption,1) );
  $this->_infoNow ( ' answer:<pre>', print_r($answer,1).'</pre>' ); // just info for the test
  $this->_infoNow ( '', '');
}

// return: 0 : not passed, 1: passed
function dotest( &$sqlo, $options ) {

    $table='CONCRETE_SUBST';
	$this->OBJ_ID = $this->_getDefObjID( "CONCRETE_SUBST", "default" );

	$testLib = new objInfoC();

    $this->_substest1($sqlo, $testLib, "typeAuto", array("typeAuto"=>1, "vario"=>1) );
    $this->_substest1($sqlo, $testLib, "some cols", array("cols"=>array('NAME', 'SOME_DATE','NOTES')) );
	$this->_substest1($sqlo, $testLib, "some cols, typeAuto", array("typeAuto"=>1, "cols"=>array('NAME', 'SOME_DATE','NOTES')) );

	// ------------------------
	$testname='TEST_nice_01';
	$params  = $testLib->getParamsRaw($sqlo, $table, $this->OBJ_ID );
	$nice_params   = $testLib->paramRaw2nice($sqlo, $table, $params);
	$this->_infoNow ( $testname.':answer:<pre>', print_r($nice_params,1).'</pre>' ); 
	
	$result=0;
	if (sizeof($nice_params)) {
	    $result = 1;
	}
	$this->_saveTestResult( $testname, $result);
	
	// --------------------
	
	$retval = 1;
	return ($retval);
}

}
