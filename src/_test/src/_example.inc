<?php
// example for testing: csv.substance.lib.xarray.inc

class UT_csv_substance_lib_xarray_inc extends gUnitTestSub {
	


// return: 0 : not passed, 1: passed
function dotest( &$sqlo, $options ) {

	$ABSTRACT_ARRAY_ID = $this->_getDefObjID( "ABSTRACT_ARRAY", "default" );
	$DEF_NUM_SOCKETS = 5;

	$testLib = new classToTest();
	$answer = $testLib->doSomething();
	
	$this->_info( 'answer', $answer ); // just info for the test
	
	$result=1;
	if ($answer!=$DEF_NUM_SOCKETS) $result=0;
	$this->_saveTestResult('SOCKETS_CREATED', $result); // save test result ...
    
    // create CABNAT
	$CABNAT_ID = $this->_getDefObjID( 'CABNAT', 'default' );
	$crealib  = new insertC();
	$args=array( 'vals'=>array('NAME'=>'UT_'.$new_name_id) );
	$CABNAT_ID = $crealib->new_meta($sqlo,'CABNAT',$args);
	
	
	if (!$absSocID or !$CABNAT_ID) {
		$error->set( $FUNCNAME, 1, 'Input objects ABS_SOCKET_ID or $CABNAT_ID missing in database.' );
		return;
	}
	
	$retval = 0;
	$subTest = $options['subTest'];
	
	
	switch ($subTest) {
		case '001':
			
			$this->_info( 'TEST001: good in put', NULL );
			$retval = 1;
			
			break;
		case '002':
					
			$this->_info( 'TEST002: bad input', NULL );
			$retval = 1;
				
			break;
		
	}

	
	return 1;
}

}
