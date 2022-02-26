<?php

class UT_f_directorySub_inc extends gUnitTestSub {
	

function dotest( &$sqlo, $options ) {

    $tstcls = new fDirextoryC();
    
    $in='A_B.C\\D/E^F(G)H[I]J K 1234590_hallo du µmeter';
    $out = $tstcls->str2path($in);
    $this->_infoNow('TEST01', 'IN:'.$in.' OUT:'.$out);
    $result=1;
	$this->_saveTestResult('TEST01', $result); 
	
	$in=array();
	$in[0]  = '/testpath/end sep/';
	$in[2]  = 'path2/text1.txt';
	$expect = '/testpath/end sep/path2/text1.txt';
	$out = $tstcls->join_path($in);
	$this->_infoNow('TEST02', 'IN:'.print_r($in,1).' OUT:'.$out);
	$result = 0;
	if ($out==$expect) $result=1;
	$this->_saveTestResult('TEST02', $result, 'expect: '.$expect); 
	
	$data_dir_base = $this->_getTestDir_path();
	$data_dir = $data_dir_base .'/phplib';
	$test2_lib = new fDirexScanC($data_dir);
	$test2_lib->scanDirx('.');
	$answer = $test2_lib->getDirArr();
	$this->_infoNow('TEST03_scan_rec', 'IN:'.$data_dir.' OUT:'.print_r($answer,1));
	$answer2 = $test2_lib->getAbsDirArr();
	$this->_infoNow('TEST03_scan_rec(full)', 'IN:'.$data_dir.' OUT:'.print_r($answer2,1));
	
	$result = 0;
	if (!empty($answer)) $result=1;
	$this->_saveTestResult('TEST03_scan_rec', $result, 'expect: list'); 
	
	
	return (1);
}

}
