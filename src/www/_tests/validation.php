<?php
/**
 * validation tool selector
 * test-dir: $_SESSION['s_sessVars']['AppRootDir'].'/_test/valid'
 * 
 * @package validation.php
 * @swreq UREQ:9855; FS-REG02-t1 Support validation: allow automated testing of the software; OQ-tests
 * @link  file://CCT_QM_doc/89_1002_SDS_code.pdf
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param $query (ID of test) :
				"ADM05_01_01" : Table-Admin
   @param $paramx	 optional parameter		
   @param $xmlrpc_server - ID of XMLRPC-server in config-file: xml_rpc_client2.php
   @param $infolevel : 0,1,2,3
 */
extract($_REQUEST); 
session_start(); 

require_once ('reqnormal.inc');
require_once ('f.msgboxes.inc');

/**
 * abstract validation class
 * - use _check.... methods to fill the error stack
 * @author steffen
 * 
 *
 */
class validationAbsC {
	
	var $xmlrpc_server;
	var $paramx;
	var $infolevel;
	
	function _setParamx($paramx, $xmlrpc_server, $infolevel) {
		$this->paramx=$paramx;
		$this->xmlrpc_server = $xmlrpc_server;
		$this->infolevel = $infolevel;
		
		$this->errstack = array();
		$this->lastErrorIndex = 0;
	}
	
	function perform(&$sqlo, &$sqlo2) {}
	
	// show text info, if info-level>0
	function txtout($text) {
		if ($this->infolevel>0) {
			echo "INFO: ".$text.'<br />';
		}
	}
	
	/**
	 * set error pointer to current end of error stack
	 */
	function _errSetStartPoint() {
		if (!sizeof($this->errstack)) {
			$this->lastErrorIndex = 0;
			return;
		}
		
		// else ...
		end($this->errstack);
		$this->lastErrorIndex = key($this->errstack) + 1;
	}
	
	/**
	 * get last errors beginning on $this->lastErrorIndex
	 */
	function _getLastErrors() {
		$part_errors = array_slice($this->errstack, $this->lastErrorIndex);
		return $part_errors;
	}
	
	/**
	 * get ALL errors
	 */
	function _getAllErrors() {
		
		return $this->errstack;
	}
	
	/**
	 * - can check simple vaklues and simple arrays
	 * @param string $checktext
	 * @param unknown $val1 - data in database
	 * @param unknown $val2 - expected data value
	 */
	function _checkDiff($checktext, $val1, $val2) {
	
		$answer='ok';
		
		if (is_array($val1)) {
			
			$diffarr = array_diff($val1, $val2);
			if (is_array($diffarr) and sizeof($diffarr)) {
				$this->errstack[] = $checktext;
				$answer='FAIL';
			}
			
			if ($this->infolevel>0) echo '-CHECK.diff:'.$checktext.': ARRAY:| '.print_r($val1,1).'|'.print_r($val2,1). " | ANSWER: <b>".$answer."</b><br>";
		} else {
			if ( $val1 != $val2 ) {
				$this->errstack[] = $checktext;
				$answer='FAIL';
			}
		
			if ($this->infolevel>0) echo '-CHECK.diff:'.$checktext.': |'.$val1.'|'.$val2. "| ANSWER: ".$answer."<br>";
		}
	}
	
	/**
	 * set an error
	 * @param string $checktext
	 * @param unknown $val1 - data in database
	 * @param unknown $val2 - expected data value
	 */
	function _setError($checktext, $val=NULL, $notes=NULL) {
	
		$answer='FAIL';
		$this->errstack[] = $checktext;
	
		if ($this->infolevel>0) echo '-CHECK.fail:'.$checktext.': |val: '.$val.'|';
		if ($notes!=NULL) echo 'notes: '.$notes.'|';
		echo 'ANSWER: '.$answer."<br>";
	
	}
	
	/**
	 *
	 * @param unknown $checktext
	 * @param unknown $val
	 * @return int
	 *   1 : ok
	 *   -1: fail
	 */
	function _checkExists($checktext, $val) {
	
		$answer='ok';
		$retval = 1;
		if ( !$val ) {
			$this->errstack[] = $checktext;
			$answer='FAIL';
			$retval = -1;
		}
	
		if ($this->infolevel>0) echo '-CHECK.exists:'. $checktext.': |'.$val.'| ANSWER: '.$answer."<br>";
	
		return $retval;
	
	}
}

class ValidationGui {
	
	var $paramx;  // - params		
	var $CLASSNAME = 'ValidationGui';
	
	function __construct( $go, $query, $paramx, $xmlrpc_server, $infolevel ) {
		
		$this->query = $query;
		$this->paramx	= $paramx;
		$this->go	= $go;
		$this->xmlrpc_server	= $xmlrpc_server;
		$this->infolevel = $infolevel;
		
	}
	
	function queryInfo($query) {
	
		return ($query);
	}
	
	/**
	 * @global $this->query, $this->go
	 * @return unknown_type
	 */
	function linkhead() {
		
		$query = $this->query;
		$paramx = $this->paramx;
		$queryinfo = $this->queryInfo($query);
		$tmptitle  = 'Test: '.$queryinfo;
		$tmptitle .= '; Params: ';
		if ($this->paramx!=NULL) {
			$tmptitle .= $this->paramx;
		} else {
			$tmptitle .= '-';
		}
		
		$tmptitle .= '; Xmlrpc_server: ';
		if ($this->xmlrpc_server!=NULL) {
			$tmptitle .= $this->xmlrpc_server;
		} else {
			$tmptitle .= '-';
		}
		
		
		if ($query!=NULL) {
			$tmptitle .= ' [<a href="'.$_SERVER['PHP_SELF'].'">New tool selection </a>]&nbsp;';
		}
		if ( $this->go ) {
			$tmptitle .= ' [<a href="'.$_SERVER['PHP_SELF'].'?query='.$query.'&paramx='.$paramx.'">Start tool again </a>]&nbsp;';
		}
		echo $tmptitle;
		echo "<br>";
		
	}
	
	function form0() {
		require_once ('func_form.inc');
		
		//DEBUG:
		
		$initarr   = NULL;
		$initarr["action"]      = $_SERVER['PHP_SELF'];
		$initarr["title"]       = "Select compare mode";
		$initarr["submittitle"] = "Submit";
		$initarr["tabwidth"]    = "AUTO";
	
		$hiddenarr = NULL;
		$hiddenarr["tablename"]     = $tablename;
	
		$formobj = new formc($initarr, $hiddenarr, 0);
		
		$fieldx = array (
			"title" => "TestName", 
			"name"  => "query",
			'namex' => TRUE, 
	        "val"   => $this->query, 
	        "notes" => "Choose Test; e.g. QA02_01_03", 
	        "object" => "text" 
			);
		
		$formobj->fieldOut( $fieldx );
			
		//Start of additional param field
		$fieldx2 = array (
			"title" => "Param", 
			"name"  => "paramx",
			'namex' => TRUE, 
	        "inits" => '2', //$inits2, 
	        "val"   => $this->paramx, 
	        "notes" => "Give parameter [optional]", 
	        "object" => "text" 
			);
	 
		$formobj->fieldOut( $fieldx2 );
		//End of additional param field
		
		
		$fieldx2 = array (
				"title" => "XMLRPC-Server",
				"name"  => "xmlrpc_server",
				'namex' => TRUE,
				"val"   => $this->xmlrpc_server,
				"notes" => "[optional]",
				"object" => "text"
		);
		
		$formobj->fieldOut( $fieldx2 );
		//End of additional param field
		
		$fieldx2 = array (
				"title" => "Info-Level",
				"name"  => "infolevel",
				'namex' => TRUE,
				"val"   => $this->info_level,
				"notes" => "[0],1,2,3 [optional]",
				"object" => "text"
		);
		
		$formobj->fieldOut( $fieldx2 );
	
		$formobj->close( TRUE );
	}
	
	function perform( &$sqlo, &$sqlo2) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$testdir= $_SESSION['s_sessVars']['AppRootDir'].'/_test/valid';
		
		if (!file_exists($testdir)) {
		    $error->set( $FUNCNAME, 1, 'Validation-Test-Dir "'.$testdir.'" not found.' );
		    return;
		}
		
		$paramx = $this->paramx;
		$testName = $this->query;
		$className = str_replace("-","_", $testName);
		
		require_once($testdir.'/'.$testName.'.inc');
		
		$className_tmp = 'Test_'.$className;
		
		if ( !class_exists($className_tmp)) {
			$error->set( $FUNCNAME, 3, 'class "'.$className_tmp.'" not found.' );
			return;
		}
		
		$newlib = new $className_tmp();
		$newlib->_setParamx($paramx, $this->xmlrpc_server, $this->infolevel);
		$newlib->perform($sqlo, $sqlo2);
		
		$allCheckErr = $newlib->_getAllErrors();
		if (sizeof($allCheckErr)) {
			$numx = sizeof($allCheckErr);
			$error->set( $FUNCNAME, 2, 'Test-Errors occurred. Num_of_Err:'.$numx );
		}
		
		if ($error->Got(READONLY))  {
			$error->printAll();
			return;
		} else {
			echo "<br>";
			cMsgbox::showBox("ok", "Test successful.");
		}
		
	
	}

}

// --------------------------------------------------- 
global $error, $varcol;

$error = & ErrorHandler::get();
$sqlo  = logon2( $_SERVER['PHP_SELF'] );
$sqlo2 = logon2(  );
if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();

$go 		= $_REQUEST['go'];
$query      = $_REQUEST['query'];
$paramx		= $_REQUEST['paramx'];

$title		= 'Application Validation Tools';

$infoarr			 = NULL;
$infoarr['scriptID'] = '';
$infoarr['title']    = $title;
$infoarr['form_type']= 'tool'; // 'tool', 'list'
$infoarr['design']   = 'norm';
$infoarr['locrow']   = array( array('index.php', 'lab root') );

$pagelib = new gHtmlHead();
$headarr = $pagelib->startPage($sqlo, $infoarr);

if ( !glob_isAdmin() ) {
     htmlErrorBox( "Error",   
     "Only root can execute this!",
     "For security reason it is not allowed for common users" );
     $pagelib->htmlFoot();
}

$workLib = new ValidationGui( $go, $query, $paramx, $_REQUEST['xmlrpc_server'], $_REQUEST['infolevel'] );

if ($query==NULL) {
	$workLib->form0();
	$pagelib->htmlFoot();
}

$workLib->linkhead();

$workLib->perform($sqlo, $sqlo2);
$pagelib->chkErrStop();
$pagelib->htmlFoot();
