<?php
// test f.xlsxml.simple.inc
require_once('f.workdir.inc');

class UT_f_xlsxml_simple_inc extends gUnitTestSub {
	
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
	
	$filenameNice = 'test1.xls';
	$workSubDir   = $workdirObj->subDirName;
	
	$workfilename = $tmpdir.'/'.$filenameNice;
     

	// generate file (constructor parameters are optional)
	$exportObj = new Excel_XML($workfilename);
	
	
	$exportObj->openFile();
    
	$dataRow = array('# comment1');
	$exportObj->addRow($dataRow);
	
	$dataRow = array ('Name', 'Surname');
	$exportObj->addRow($dataRow);
	$dataRow = array ('00045676', '4.678');
	$exportObj->addRow($dataRow);
	
	$i=0;
	$dataRow=NULL;
	while ( $i<400) {
		$dataRow[]= $i+0.5;
		$i++;
	}
	$exportObj->addRow($dataRow);
	$exportObj->addRow($dataRow);
	
	$exportObj->closeFile();
	
	$down_url_path = $_SESSION['s_sessVars']['loginURL']."/f_workfile_down.php?file=".
          $workSubDir."/".$filenameNice;
    $textOut = '<b><a href="'.$down_url_path.'">Download file</a></b>';
	
	$this->_info( 'Download-Link:', $textOut );
	
	$answer=1;
	if ($answer!=NULL) $retval = 1;
	else  $retval = 0;
	
	return ($retval);
}

}
