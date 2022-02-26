<?php
// for glob.obj.update.inc
require_once 'object.info.inc';
require_once 'insert.inc';


class UT_glob_obj_update_inc extends gUnitTestSub {
	

function _showDiff($title, &$answer1, &$answer2) {
	$diffArr=NULL;
	$testkeys=array('vals','xobj', 'vario');
	foreach( $testkeys as $dummy=>$key) {
		$diffLoop = array_diff($answer1[$key],$answer2[$key]);
		if ($key=='xobj') {
			$tmpArr1 = $answer1[$key]['values'];
			$tmpArr2 = $answer2[$key]['values'];
			
			if ($tmpArr1===NULL) $tmpArr1=array();
			if ($tmpArr2===NULL) $tmpArr2=array();
			
			$diffLoop['values'] = array_diff_assoc($tmpArr1,$tmpArr2);
		}
		if (sizeof($diffLoop)) $diffArr[$key]=$diffLoop;
	}
	
	$this->_infoNow( $title.': Difference', print_r($diffArr,1) );
}

function _badTest(&$sqlo) {
	$table   = 'H_USAGE';
	$nameTmp = 'UnitTest1';
	$objid = glob_elementDataGet($sqlo, $table, 'NAME', $nameTmp, 'H_USAGE_ID');
	if (!$objid) {
		$objid = insert_row($sqlo, $table, array('NAME'=>'UnitTest1') );
		if (!$objid) return;
	}
	
	$updateLib = new globObjUpdate();
	$args=array( 'vals'=>array('NOTES'=>'hello') );
	
	$updateLib->update_meta( $sqlo, $table, $objid, $args );
}

// return: 0 : not passed, 1: passed
function dotest( &$sqlo, $options ) {
	global $error, $varcol;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;

	$varcol = &Varcols::get();
	$tablename='ABSTRACT_SUBST';
	$ORI_ID = $this->_getExistObjID( $tablename, "with_Class" );
	$this->_infoNow( 'Info', "ORI_ID: $ORI_ID");

	$SUA_NAME = __CLASS__;
	
	
	$class_name='UT_class';
	$clsid = $varcol->class_name_to_id ( $tablename , $class_name );
	if ($error->Got(READONLY))  {
		$error->set( $FUNCNAME, 1, 'class missing.' );
		return;
	}
	
	$rawopt = array("vario"=>1, "xObjParById"=>1);
	$obj_oriparam = objInfoC::getParamsRaw($sqlo, $tablename, $ORI_ID, $rawopt);
	unset($obj_oriparam['vals']['ABSTRACT_SUBST_ID']);
	$obj_oriparam['vals']['NAME'] = $SUA_NAME;
	
	$SUA_ID = $this->_createObj_checkExists($sqlo, $tablename, 'NAME', $SUA_NAME, $obj_oriparam);
	
	
	$args = array(
		'vals'=> array( 
		    'NOTES'=>NULL,
		    'PROD_CONTACT_ID'=>NULL   // test NULL on an integer 
		),
		'vario'=>array( 'testkey'=>NULL ),
	);
	
	$updateLib = new globObjUpdate();
	$update_opt0=array("noRightCheck"=>1);
	$updateLib->update_meta( $sqlo, $tablename, $SUA_ID, $args, $update_opt0 );
	if ($error->Got(READONLY))  {
		$error->set( $FUNCNAME, 1, 'error on Init.' );
		return;
	}
	
	$testLib   = new objInfoC ();
	$rawoption = array ("typeAuto"=>1 , "vario"=>1);
	
	// now start the test
	
	// Test0: access rights ignore
	
	$answer1   = $testLib->getParamsRaw ($sqlo , $tablename, $SUA_ID, $rawoption );
	
	$args = array(
		'vals'=> array( 'NOTES'=>date('Y-m-d H:m'). ' access ignore'),
	);
	$updateLib = new globObjUpdate();
	$update_opt0=array("noRightCheck"=>1);
	$updateLib->update_meta( $sqlo, $tablename, $SUA_ID, $args, $update_opt0 );
	if ($error->Got(READONLY))  {
		$error->set( $FUNCNAME, 1, 'error on Test0' );
		return;
	}
	$answer2 = $testLib->getParamsRaw ($sqlo , $tablename, $SUA_ID, $rawoption );
	$this->_showDiff('Test0', $answer1, $answer2);
	$this->_infoNow('Test0: new notes:', $answer2['vals']['NOTES']);
	
	
	// Test1: update more
	
	$answer1   = $testLib->getParamsRaw ($sqlo , $tablename, $SUA_ID, $rawoption );
	
	$args = array(
		'vals'=> array( 'NOTES'=>date('Y-m-d h:m')),
		'vario'=>array( 'testkey'=>'testval', 'Country'=>'DE' ),
		"xobj" => array ( 
			'extra_class_id' => $clsid,
			'values'=> array('val1'=>456)
			)
	);
	
	$updateLib = new globObjUpdate();
	$updateLib->update_meta( $sqlo, $tablename, $SUA_ID, $args );
	
	$answer2 = $testLib->getParamsRaw ($sqlo , $tablename, $SUA_ID, $rawoption );
	
	$this->_infoNow( 'Data1', '<pre>'. print_r($answer1,1).'</pre>' );
	$this->_infoNow( 'Data2', '<pre>'. print_r($answer2,1).'</pre>' );
	
	$this->_showDiff('Test1', $answer1, $answer2);
	
	$this->_infoNow( 'Data3', 'Now Bad Input: NON-BO ... ' );
	$this->_badTest($sqlo);
	$this->_infoNow( 'Data3', 'Now Bad Input: NON-BO <end /> ' );

	$retval = 1;
	
	
	return ($retval);
}

}