<?php
// test f.csv.simple.inc
require_once('f.workdir.inc');

class UT_f_csv_inc extends gUnitTestSub {
	
function __construct() {

}

// return: 0 : not passed, 1: passed
function dotest( &$sqlo, $options ) {
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	$scriptID='Unittest';
	$workdirObj = new workDir();
	$tmpdir   = $workdirObj->getWorkDir ( $scriptID );
	if ($error->Got(READONLY) or $tmpdir==NULL)  {
		$error->set( $FUNCNAME, 1, 'error on getWorkDir' );
		return;
	}
	$workSubDir   = $workdirObj->subDirName;
	
	/**
	 * export AUTO types
	 */
	$filenameNice1 = 'test1.autotype.txt';
	$workfilename = $tmpdir.'/'.$filenameNice1;
	$option=array('separator'=>"\t");
	// generate file (constructor parameters are optional)
	$exportObj = new f_export_csv($workfilename, $option);
	$exportObj->openFile();
	$dataRow = array('# comment1');
	$exportObj->addRow($dataRow);
	$dataRow = array ('realNumber', 'Name', 'Surname', 'text');
	$exportObj->addRow($dataRow);
	$dataRow = array ('4.678', '00045676', "Kuben\nneue Zeile", "hell\too");
	$exportObj->addRow($dataRow);
	$exportObj->closeFile();
	
	/**
	 * export STRICT types
	 */
	$filenameNice2 = 'test2.deftype.txt';
	$workfilename = $tmpdir.'/'.$filenameNice2;
	// generate file (constructor parameters are optional)
	$expOptions = array(
		'typeArr' => array( 0=>'numeric', 1=>'string', 2=>'string')
	);
	$exportObj = new f_export_csv($workfilename, $expOptions);
	$exportObj->openFile();
	$dataRow = array('# comment1');
	$exportObj->addHeader($dataRow); // header !
	$dataRow = array ('realNumber', 'Name', 'Surname');
	$exportObj->addHeader($dataRow); // header !
	
	$dataRow = array ('4.678', '00045676', 'Kuben');
	$exportObj->addRow($dataRow);
	$dataRow = array ('badType_trial', '00045676', 'Kuben');
	$exportObj->addRow($dataRow);
	$exportObj->closeFile();
	
	/*
	$i=0;
	$dataRow=NULL;
	while ( $i<254) {
		$dataRow[]= $i+0.5;
		$i++;
	}
	$exportObj->addRow($dataRow);
	$exportObj->addRow($dataRow);
	*/
	
	
	
	$down_url_path = $_SESSION['s_sessVars']['loginURL']."/f_workfile_down.php?file=".
          $workSubDir."/".$filenameNice1;
    $textOut = '<b><a href="'.$down_url_path.'">Download file1</a></b>';
    
    $down_url_path = $_SESSION['s_sessVars']['loginURL']."/f_workfile_down.php?file=".
          $workSubDir."/".$filenameNice2;
    $textOut .= ',  <b><a href="'.$down_url_path.'">Download file2</a></b>';
	
	$this->_info( 'Download-Link:', $textOut );
	
	$answer=1;
	if ($answer!=NULL) $retval = 1;
	else  $retval = 0;
	
	return ($retval);
}

}
