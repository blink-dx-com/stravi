<?php
// example for testing: csv.substance.lib.xarray.inc

class UT_PythonApi_inc extends gUnitTestSub {
	
function __construct() {

}

// return: 0 : not passed, 1: passed
function dotest( &$sqlo, $options ) {
	global $error;
	$FUNCNAME= $this->__CLASS__.':dotest';
	
	require_once ('f.workdir.inc');
	$workdirObj = new workDir();
	$workdir    = $workdirObj->getWorkDir ( "test.PythonApi" );
	
	//OLD: 'AppLabLibDir' /project/129/stats/test.py';
	$pythonCode     = $_SESSION['s_sessVars']['AppRootDir'].'/www/pionir/rootsubs/test/appserver/test_python.py'; 
	$statsPythonPck = $_SESSION['s_sessVars']['AppRootDir'].'/pythonlib/StatsHelp.py'; // needed package
	
	$inOptDict=array('inparam1'=>'hello', 'inparam2'=>45.677);
    
    $pyLib	= new PythonApi( );
    
    if (!$pyLib->prepare2($workdir)) {
        $error->set( $FUNCNAME,3, "prepare2 failed.");
        return;
    }
    
    if (!$pyLib->prepareScript($pythonCode)) {
        $error->set( $FUNCNAME,3, "script prepare failed.");
        return;
    }   
    $pyLib->addPythonModule($statsPythonPck);
   
    $result = $pyLib->runScript($infileShort,$inOptDict);
    if ($error->Got(READONLY))  {
        $error->set( $FUNCNAME,4, "running script failed.");
        return;
    }   
    $this->_infoNow( "startPythonCode:result: ",print_r($result,1) );
	
	$this->_info( 'answer', $answer ); // just info for the test
	
	$retval = 1;
	
	return ($retval);
}

}
