<?php
// example for testing:

class UT_varcols_inc extends gUnitTestSub {

// return: 0 : not passed, 1: passed
function dotest( &$sqlo, $options ) {


	$error = & ErrorHandler::get();
	$varcol = & Varcols::get();
	
	$classname='UT_class';
	$table_name='ABSTRACT_SUBST';
	
	$class_id	= $varcol->class_name_to_id( $table_name, $classname );
	$result = $class_id>0 ? 1 : 0;
	$infotext= 'Class-id of class "'.$classname.'": '.$class_id;
	$this->_saveTestResult('class_name_to_id', $result, $infotext);
	
	$varcol = & Varcols::get();
	
	if ($class_id) {
		$classname_tmp  = $varcol->class_id_to_name( $class_id );
		$result = $classname_tmp==$classname ? 1 : 0;
		$infotext='Class-Name of class-id "'.$class_id.'": '.$classname_tmp;
		$this->_saveTestResult('class_id_to_name', $result, $infotext);
	}

	$retval = 1;
	
	return ($retval);
}

}
