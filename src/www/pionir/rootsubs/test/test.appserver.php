<?php 
/**
 * test the application server
 * - for some tests: works without database !!!
 * @package test.appserver.php
 * @swreq SREQ:0001146: g > appserver test 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param   int go
 * @param int testid
 * 	 'email' : do email test
 * @parx['emailaddress'] : emailaddress
 * @version $Header: trunk/src/www/pionir/rootsubs/test/test.appserver.php 59 2018-11-21 09:04:09Z $
 */

extract($_REQUEST); 
session_start(); 

require_once ('reqnormal.inc');
require_once 'f.email.inc';
require_once ('f.workdir.inc');


/**
 * test the appserver:
 *  - php
 *  - python
 * @author steffen
 *
 */
class gAppserverTestC {
	
	private $infolevel;
	
	function __construct($infolevel, $parx) {
		
		$this->infolevel= $infolevel;
		$this->parx     = $parx;
		$this->isLoggedin=0;
		$this->emailAddress = NULL;
		
		if ($parx['emailaddress']!=NULL ) $this->emailAddress = $parx['emailaddress'];
		
		if ( glob_loggedin() ) {
			$this->isLoggedin=1;
		}
		echo 'loggedin? '.$this->isLoggedin.'<br>';
	}
	
	function init() {
		$this->phpextArr = array('pgsql', 'json', 'ldap', 'pcre', 'xmlrpc','gd', 'sqlite3'); // , 'pam'
	}
	
	function _tableStart() {
		echo '<table border=1>';
	}
	function _tableStop() {
		echo '</table>'."\n";
	}
	
	private function _getError() {
		global $error;
		
		if ($this->infolevel>1) {
			$error_txt = $error->getAllAsText();
		} else {
			$errLast   = $error->getLast();
			$error_txt = $errLast->text;
		}
		$error->reset();
		$infoStr  = 'ERROR: '.$error_txt;
		return $infoStr;
	}
	
	/**
	 * out info line
	 * @param $status 
	 * 	0 : tbd
	 *  1 : ok
	 *  -1: error
	 * @param $content
	 * @param $notes
	 */
	function _infoLine($key,$status, $content, $notes) {
		
		$statsarr = array(
			0=>'<font color=gray>Not defined</font>',
			1=>'<font color=green>o.k.</font>',
			-1=>'<font color=red>error</font>',
			);
		echo '<tr valign=top>';
		echo '<td>'.$key.'</td>';
		echo '<td>'.$statsarr[$status].'</td>';
		echo '<td>'.$content.'</td>';
		echo '<td>'.$notes.'</td>';
		echo '</tr>';
	}
	
	function do_phpFuncs() {
		$testFuncs= array('preg_match');
		$komma=NULL;
		$sumFound = 1;
		foreach( $testFuncs as $oneFunc) {
			$found=0;
			if(function_exists($oneFunc)){
				$found=1;
			} else {
				
			}
			$infoStr .= $komma . $oneFunc.':'.$found;
			$komma='<br />';
			if (!$found) $sumFound=0;
		}
		if ($sumFound) $status=1;
		else $status=-1;
		
		$this->_infoLine('php-functions',$status, $infoStr, 'check for functions');
	}
	
	/**
	 * test php extensions
	 * Enter description here ...
	 */
	function do_phpext() {
		$loadedModules = get_loaded_extensions();
		$loadedModules = array_map('strtolower', $loadedModules);
		$komma=NULL;
		$sumFound = 1;
		foreach( $this->phpextArr as $expectStr) {
			$found=0;
			if(in_array($expectStr, $loadedModules)){
				$found=1;
			} else {
				
			}
			$infoStr .= $komma . $expectStr.':'.$found;
			$komma='<br />';
			if (!$found) $sumFound=0;
		}
		
		if ($sumFound) $status=1;
		else $status=-1;
		
		$this->_infoLine('php-ext-packages',$status, $infoStr, 'check for packages');
	}
	
	function _python1_sub() {
		
		require_once ('lev1/PythonApi.inc');
		
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$workdirObj = new workDir();
		$workdir    = $workdirObj->getWorkDir ( "test.PythonApi" );
		
		$pythonCode     = 'appserver/test_python.py';
		$pythonCode2    = 'matplotlib_test.py';
		$statsPythonPck = 'appserver/StatsHelp.py'; // needed package
		
		if (!file_exists($pythonCode)) {
			$error->set( $FUNCNAME, 6, "init error: file missing:".$pythonCode);
			return;
		}
		
		$inOptDict=array('inparam1'=>'hello', 'inparam2'=>45.677);
	    
	    // RInterface
	    $pyLib	= new PythonApi( );
		if ($error->Got(READONLY))  {
	        $error->set( $FUNCNAME,1, "init error.");
	        return;
	    }
	    
	    if (!$pyLib->prepare2($workdir)) {
	        $error->set( $FUNCNAME,3, "prepare2 failed.");
	        return;
	    }
	    
	    if (!$pyLib->prepareScript($pythonCode)) {
	        $error->set( $FUNCNAME,3, "script prepare failed.");
	        return;
	    }
	       
	    // $pyLib->addPythonModule($statsPythonPck);
	    
	    $result = $pyLib->runScript($infileShort,$inOptDict);
	    if ($error->Got(READONLY))  {
	        $error->set( $FUNCNAME,4, "running script1 failed.");
	        return;
	    }
	    
	    $infostr = 'script1:'.$pythonCode;
	    
		if (!is_array($result) ) {
			$infoStr = 'no array result from python.';
			$error->set( $FUNCNAME,5, "script1: no array result from python.");
	        return array($infostr, NULL);
		}
	    
	    // second test MATPLOTLIB
	    /*
	    $pyLib->addPythonModule($pythonCode2);
	 	$result = $pyLib->runScript($infileShort,$inOptDict);
	    if ($error->Got(READONLY))  {
	        $error->set( $FUNCNAME,5, 'running script2 '.$pythonCode2.' failed.');
	        return;
	    }
	    $infostr .= '<br>script2:'.$pythonCode2;
	    */
	    
	    return array($infostr, $result['python.version']);
	}
	
	private function _rscript_sub(&$sqlo) {
		
		require_once ("../../objlib/RInterface.inc");
		
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;

		
		$workdirObj = new workDir();
		$workdir   = $workdirObj->getWorkDir ("R.pack");
		$workdir   .= "/";
		
		
		
		
		$sqls   =  "select VALUE from GLOBALS where NAME='exe.R'";
		$sqlo    -> query($sqls);
		$sqlo    -> ReadRow();
		$rtool  =  $sqlo->RowData[0];
		
		// RInterface
		
		$RObj 	= new RInterface($rtool, '');
		
		if (!$RObj->prepare2($workdir)) {
			//$error->printLast();
		}
		
		$rcode_short='appserver/RScript.R';
		$codefile = dirname(__FILE__).'/'.$rcode_short;
		
		if (!$RObj->prepareRScript($codefile) ) {
			$error->set( $FUNCNAME,3, "R prepare failed.");
			return;
		}
		if (!$RObj->runRScript()) {
			$error->set( $FUNCNAME,4, "running R failed.");
			return;
		}
		
		$outputfile= $workdir.'ROutput';
		
		if ( !file_exists($outputfile)) {
			$error->set( $FUNCNAME,5, 'ROutput file not found.' );
			return array('info'=>'hello', 'workdir'=>$workdir);
		}
		
		$version = file_get_contents($outputfile);
		$infostr = 'script1:'.$rcode_short;
		return array('info'=>$infostr, 'version'=>$version, 'workdir'=>$workdir);
	}
	
	function do_python1() {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$infoArr = $this->_python1_sub();
		
		$infoStr = $infoArr[0];
		$status=0;
		if ($error->Got(READONLY))  {
			$infoStr .= $this->_getError();
			$status  = -1;
		} else {
			$status   = 1;
			
		}
		
		$this->_infoLine('python API', $status, $infoStr, 'check for Python Api');
		$this->_infoLine('python VERSION', $status, $infoArr[1], 'minimum: 2.4');
	}
	
	function do_rscript(&$sqlo) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
		$infoArr = $this->_rscript_sub($sqlo);
	
		$infoStr = $infoArr['info'];
		$status=0;
		if ($error->Got(READONLY))  {
			$infoStr .= $this->_getError();
			$status  = -1;
		} else {
			$status   = 1;
				
		}
	

		$this->_infoLine('Rscript API', $status, $infoStr, 'check R-Package');
		$this->_infoLine('Rscript VERSION', $status, $infoArr['version'], 'minimum: 2.15.2');
	}
	
	function do_phpEmail() {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$status = 0;
		$infoStr= '?';
		
		if ($this->emailAddress!=NULL) {
			
			$emailadd = $this->emailAddress;
			$fromModule = $FUNCNAME;
			$message='Application-Server-Email-Test';
			$mailObj = new fEmailC();
			$mailObj->sendMessage ( $emailadd, 'Test-email', $message, $fromModule, $opt=NULL  );
			
			if ($error->Got(READONLY))  {
				$infoStr = $this->_getError();
				$status  = -1;
			} else {
				$status   = 1;
				$infoStr = 'Sent email. Please check your Inbox now!';
				
			}
			
			$this->_infoLine( 'email', $status, $infoStr, 'check for email-function; email-Address: '.$emailadd.'. ' );
		} else {
			$infoStr =
			 "<form style=\"display:inline;\" method=\"post\" " .
				 " name=\"editform\"  action=\"".$_SERVER['PHP_SELF']."\" >\n" .
			 "<input type=hidden name='go' value='1'>\n" .
			 "<input type=text name='parx[emailaddress]' value=''>\n" .
			 "<input type=submit value=\"Send Test-Email\">\n" .
			 "</form>";
			
			$this->_infoLine( 'email', $status, $infoStr, 'check for email-function' );
		}
	}
	
	function do_ALL(&$sqlo) {
		$this->_tableStart();
		
		$this->do_phpext();
		$this->do_phpFuncs();
		// can do Python tests ?
		if ( $this->isLoggedin ) {
			$this->do_python1();
		}
		if ( $this->isLoggedin ) {
			$this->do_rscript($sqlo);
		}
		$this->do_phpEmail();
		
		$this->_tableStop();
	}
}

function this_showForm($infolevel) {
	echo '<form name=editform ACTION="'.$_SERVER['PHP_SELF'].'" METHOD=POST>'."\n";
	echo "Infolevel: <input name='infolevel' size=2 value='".$infolevel."'>\n";
	echo "<input type=hidden name='go' value='1'>\n";
	echo " <input type=submit value=\"Submit\">\n"; // SUBMIT
	echo "</form><br> \n";
}

 
global $error, $varcol;

$error = & ErrorHandler::get();
if ( glob_loggedin() ) {
	$sqlo  = logon2( $_SERVER['PHP_SELF'] );
} else {
	$sqlo = NULL;
}
if ($error->printLast()) htmlFoot();

$infolevel=1;

$parx = $_REQUEST['parx'];
$go   = $_REQUEST['go'];
if ($_REQUEST['infolevel']>0) $infolevel = $_REQUEST['infolevel'];

$title		= 'Test: application server';

$infoarr			 = NULL;
$infoarr['scriptID'] = '';
$infoarr['title']    = $title;
$infoarr['form_type']= 'tool';
$infoarr['design']   = 'norm';
$infoarr['locrow']   = array( array('index.php', 'test home') );
$pagelib    = new gHtmlHead();
$pagelib->set_loginUrl('../..');

// $pagelib->_PageHead ( $infoarr["title"],  $infoarr );
$headarr    = $pagelib->startPage($sqlo, $infoarr);

echo "Info: Test the appserver without database and (some php-testswithout login).<br>\n";


this_showForm($infolevel);

if (!$go) {
    $pagelib->htmlFoot();
}


$mainLib = new gAppserverTestC($infolevel, $parx);
$mainLib->init();
$mainLib->do_ALL($sqlo);

$pagelib->htmlFoot();
