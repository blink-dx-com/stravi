<?php

class UT_glob_obj_copyobj2_inc extends gUnitTestSub {
	
// return: 0 : not passed, 1: passed
function dotest( &$sqlo, $options ) {
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	$sqlo2  = logon2( );
	$sqlo3  = logon2( );
	
	
	// test
	$table ='CONCRETE_SUBST';
	$srcid = $this->_getDefObjID( $table, 'default' );
	$src_id_TST2 = $this->_getDefObjID( $table, 'proto' );
	
	$obj_tmpl_id = $this->_getDefObjID( 'ABSTRACT_SUBST', 'default' );
	
	$new_args= array(
	    'vals'=>array(
	        'NAME'=> 'copy of ORIGINAL',
	        'NOTES'=>'do objCreaWiz::objCreate()'
	    )
	);
	
	
	$deep_copy=array();
	$UseLib = new objCreaWiz($table);
	$newid = $UseLib->objCreate( $sqlo,  $sqlo2, $sqlo3, $srcid, $new_args, $deep_copy ) ;
	
	$this->_saveTestResult('TEST01', 1, 'object created: t:'.$table.' SRCID:'.$srcid. ' NEWID:'.$newid );
	
	$proj_id=0;
	$new_args= array(
	    'vals'=>array(
	        'NAME'=> 'create from TEMPLATE',
	        'NOTES'=>'do objCreaWiz::objCreateFrTmpl()'
	    )
	);
	$newid = $UseLib->objCreateFrTmpl( $sqlo, $sqlo2, $obj_tmpl_id,  $proj_id, $new_args );
	$this->_saveTestResult('TEST02', 1, 'object created: t:'.$table.' SRCID:'.$srcid. ' NEWID:'.$newid );
	
	
	$srcid=$src_id_TST2;
	$new_args= array(
	    'vals'=>array(
	        'NAME'=> 'copy of ORIGINAL, deep copy',
	        'NOTES'=>'do objCreaWiz::objCreate(), DEEP'
	    )
	);
	$deep_copy=array('CS_HAS_PR'=>1);
	$UseLib = new objCreaWiz($table);
	$newid = $UseLib->objCreate( $sqlo,  $sqlo2, $sqlo3, $srcid, $new_args, $deep_copy ) ;
	
	$this->_saveTestResult('TEST03-Deep', 1, 'object created: t:'.$table.' SRCID:'.$srcid. ' NEWID:'.$newid );
	
	return 1;
}

}