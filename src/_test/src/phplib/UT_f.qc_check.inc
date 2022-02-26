<?php 
/**
 * 
 * @param unknown $f_name
 * @param unknown $f_param
 * @return number
 */
require_once ("insertx.inc");

/**
 * test class for phplib/f.qc_check.inc
 * @author steffen
 *
 */
class UT_f_qc_check_inc extends gUnitTestSub {
	
function __construct() {
}

function dotest( &$sqlo, $options ) {
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	$retval = 1;
	
	$apid = $this->_getDefObjID( 'ABSTRACT_PROTO', 'default' );
	
	$cp1_NAME='UT_'.$FUNCNAME.'_1';
	
	$args=array( 
		'vals'=>array(
			'ABSTRACT_PROTO_ID'=>$apid,
		    'NAME'=>$cp1_NAME
		) 
	);
	
	$cpid1 = $this->_createObj_checkExists($sqlo, 'CONCRETE_PROTO', 'NAME', $cp1_NAME, $args);
	
	if (!$cpid1) {
		$error->set( $FUNCNAME, 1, 'error on creation.' );
		return;
	}

	
	$qcCheckLib = new fQC_checkC();
	
	$chopt=array();
	$answer = $qcCheckLib->protoCheckBase( $sqlo, $cpid1, NULL, $chopt );
	
	// analyse: array('all'=>$statusAll, 'text'=>qcProblem)
	$sub_test_val = 1;
	if ($answer['all']<0 ) {
		$sub_test_val = 0;
	}
	$this->_saveTestResult('protoCheckBase', $sub_test_val, print_r($answer, 1) );
	
	return ($retval);
}

}
