<?php
// example for testing: csv.substance.lib.xarray.inc

class UT_validate_inc extends gUnitTestSub {
	
function __construct() {

}

private function testloop($in_arr) {
	
	$vallib = new fValidate();
	
	reset ($in_arr);
	foreach( $in_arr as $key=>$row) {
		
		$answer = $vallib->_DATE_iso($row[2], $row[1]);
		$this->_infoNow( 'Answer:', $row[0] .' In:'. $row[2].' out:'.$answer.' expect:'.$row[3] );
		
		$testok=0;
		if ($row[3]==$answer) $testok=1;
		$this->_saveTestResult($row[0], $testok);
		
	}
}

private function testType_DATE($in_arr) {
	global $error;
	
	$table = 'EXP';
	$col   = 'EXP_DATE';
	$adt_id = 12; // DATE
	$vallib = new fValidate();
	
	foreach($in_arr as $row) {
		
		$answer = $vallib->_check_datatype($table, $col, $row[1], $adt_id);
		if ($error->Got(READONLY))  {
			$answer = 0;
		}
		$error->reset();
		
		$this->_infoNow( 'Answer:', $row[0] .' In:'. $row[1].' out:'.$answer.' expect:'.$row[2] );
		
		$testok=0;
		if ($row[2]===$answer) $testok=1;
		$this->_saveTestResult($row[0], $testok);
	}
}

// return: 0 : not passed, 1: passed
function dotest( &$sql, $options ) {
	global $error;
	
	

	$in_arr = array(
		array('DATE1 good', '', '2013-05-06', 1),
		array('DATE1 bad',  '', '2013-05-0 ', 0),
		array('DATE1 bad2', '', '2013-13-05', 0),
		array('DATE2 good','2', '2013-12-05 13:52', 1),
		array('DATE2 bad', '2', '2013-12-05 13:5', 0),
		array('DATE3 bad (Sec fehlt)', '3', '2013-12-05 13:56', 0),
	);
	
	$this->testloop($in_arr);
	

    // change global  date FORMAT !
	$cacheval = $_SESSION['globals']['db.dateFormat'];
	$_SESSION['globals']['db.dateFormat'] = 'DD-MON-YYYY';
	
	$in_arr = array(
		array('DATE1 good',   '02-JAN-2013', 'DATE'),
		array('DATE1 good2',  '30-FEB-2013', 'DATE'),
		array('DATE1 good3',  '30-feb-2013', 'DATE'),
		array('DATE1 bad',  '02-TUI-2013', 0),
	);
	$this->testType_DATE($in_arr);
	
	
	$_SESSION['globals']['db.dateFormat'] = $cacheval;
	$retval=1;
	
	return ($retval);
}

}
