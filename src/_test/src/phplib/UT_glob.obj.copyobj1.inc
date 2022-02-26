<?php

class UT_glob_obj_copyobj1_inc extends gUnitTestSub {
	
// return: 0 : not passed, 1: passed
function dotest( &$sqlo, $options ) {
	global $error;
	$FUNCNAME='UT_glob_obj_copyobj1_inc';
	
	$sqlo2  = logon2( );
	$retval = 0;
	
	// test
	$table ='CONCRETE_PROTO';
	$srcid = $this->_getDefObjID( $table, 'assocUpdate' );
	
	
	$UseLib = new gObjCopyS();
	$newid = $UseLib->copyOne($sqlo, $sqlo2, $table, $srcid );
	
	$result=0;
	if ($newid>0) {
	    $result = 1;
	}
	$this->_saveTestResult('Proto', $result, 'object created: t:'.$table.' SRCID:'.$srcid. ' NEWID:'.$newid );
	
	// with EXTRA_CLASS params
	$table='ABSTRACT_SUBST'; // 
	$srcid = $this->_getDefObjID( $table, "with_Class" );
	$UseLib = new gObjCopyS();
	$newid = $UseLib->copyOne($sqlo, $sqlo2, $table, $srcid );
	
	$result=0;
	if ($newid>0) {
	    $result = 1;
		$this->_saveTestResult('Proto', $result, 'object created: t:'.$table.' SRCID:'.$srcid. ' NEWID:'.$newid );
	}
	
	return 1;
}

}