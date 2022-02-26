<?php


class UT_insertx_inc extends gUnitTestSub {
	
function __construct() {

}

// return: 0 : not passed, 1: passed
function dotest( &$sqlo, $options ) {
	global $error;
	$FUNCNAME='UT_insertx_inc';
	
	$retval = 0;
	// test
	$table='G_SYSLOG';
	$prim_name='G_SYSLOG_ID';
	$argu=array(
	    'NAME'=>'UnitText '.$FUNCNAME,
		'LOG_DATE'=>date('Y-m-d'),
	    'KEY'=>'UnitTest'
	);
	
	$InsRLib = new insertRawC();
	$answer = $InsRLib->insertR( $sqlo, $table, $argu, $prim_name);
	
	if ($answer) {
		$retval = 1;
		$this->_info( 'insertRawC', 'object created: t:'.$table.' ID:'.$answer );
	}
	
	return $retval;
}

}