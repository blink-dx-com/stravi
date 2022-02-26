<?php
/**
 * - create a text list for TEST-Tool _tests/www/test/test_touchPages.php
 * - contains URLs of GUI-Scripts
 * - focus on obj.[TABLE].xedit.php - files
 * $Header: trunk/src/www/_tests/www/test/test_GUI_getList01.php 59 2018-11-21 09:04:09Z $
 * @package test_GUI_getList01.php
 * @author  qbi
 * @version 1.0
 * @param string $go
 */
session_start(); 

require_once ("reqnormal.inc");
require_once 'f.directorySub.inc';

class Test_getList01 {
		
		
	function form1() {
		require_once ('func_form.inc');
		
		$initarr   = NULL;
		$initarr["action"]      = $_SERVER['PHP_SELF'];
		$initarr["title"]       = "Start";
		$initarr["submittitle"] = "Submit";
		$initarr["tabwidth"]    = "AUTO";
	
		$hiddenarr = NULL;
	
		$formobj = new formc($initarr, $hiddenarr, 1);
	
		$formobj->close( TRUE );
	}
	
	/**
	 * obj.TABLE.xmode.[MODE].inc
	 * @param unknown $onefile
	 */
	private function _ana_oneFileXmode($onefile) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$fileName_array=explode('.',$onefile);
		$tablename_low = $fileName_array[1];
		if ($tablename_low==NULL) {
			$error->set( $FUNCNAME, 1, $onefile.': kein TABLENAME im Namen!' );
			return;
		}
		
		$mode =$fileName_array[3];
		if ($mode==NULL) {
			$error->set( $FUNCNAME, 2, $onefile.': kein MODE im Namen!' );
			return;
		}
		
		$tablename = strtoupper($tablename_low);

		$this->filearr[$tablename]['mode'][]=$mode;
	}
	
	/**
	 * obj.TABLE.xmode.[MODE].inc
	 * @param unknown $onefile
	 */
	private function _ana_oneFileXedit($onefile) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
		$fileName_array=explode('.',$onefile);
		$tablename_low = $fileName_array[1];
		if ($tablename_low==NULL) {
			$error->set( $FUNCNAME, 1, $onefile.': kein TABLENAME im Namen!' );
			return;
		}
	
		$tablename = strtoupper($tablename_low);
	
		$this->filearr[$tablename]['mode'][]='norm';
	}
	
	private function scan_Xmode($start_dir) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$pattern   = '/obj\..*\.xmode\..*\.inc/';
		
		echo "Directory: ".$start_dir."; Pattern:".$pattern."<br>";
		$dirScanLib = new fDirextoryC();
		$sortarr = $dirScanLib->scanDir($start_dir.'/', NULL);
		
		foreach($sortarr as $onefile) {
			$matches=NULL;
			$answer = preg_match($pattern, $onefile, $matches);
			if ($answer) {
				$this->_ana_oneFileXmode($onefile);
				if ($error->Got(READONLY))  {
					$error->printAllEasy();
					$error->reset();
				}
			}
		}
	}
	
	
	/**
	 * create list
	 * @param unknown $sqlo
	 */
	function runx(&$sqlo) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$dirScanLib = new fDirextoryC();
		$this->filearr=array();
		
		$start_dir = $_SESSION['s_sessVars']['loginPATH']; // pionir directory
		$this->scan_Xmode($start_dir);
		
		$start_dir = $_SESSION['s_sessVars']['AppLabLibDir']; // LAB directory
		$this->scan_Xmode($start_dir);
		
		$start_dir = $_SESSION['s_sessVars']['loginPATH']; // pionir directory
		$pattern   = '/obj\..*\.xedit\.php/';
		echo "Directory: ".$start_dir."<br>";
		$sortarr = $dirScanLib->scanDir($start_dir.'/', NULL);
		
		
		foreach($sortarr as $onefile) {
			$matches=NULL;
			$answer = preg_match($pattern, $onefile, $matches);
			if ($answer) {
				$this->_ana_oneFileXedit($onefile);
				if ($error->Got(READONLY))  {
					$error->printAllEasy();
					$error->reset();
				}
			}
		}
		
		echo "Ready<br>";
		echo "<pre>";
		print_r($this->filearr);
		echo "</pre>";
		 
	}
	
	
	

}

// --------------------------------------------------- 
global $error, $varcol;

$error = & ErrorHandler::get();
$sqlo  = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();

ob_implicit_flush  (TRUE);

$parx = $_REQUEST["parx"];
$go   = $_REQUEST["go"];
$meta = NULL;


$title				 = "Create List of OBJECT-GUI-Files for TouchTest";

$infoarr			 = NULL;
$infoarr["title"]    = $title;
$infoarr["form_type"]= "tool"; // "tool", "list"
$infoarr["locrow"] = array( array("index.php", "Unittests") );
if ( $meta !="" )  $infoarr["headIn"] = $meta;

$pagelib = new gHtmlHead();
$headarr = $pagelib->startPage($sqlo, $infoarr);

$mainlib = new Test_getList01();

if ( !$go ) {
	$mainlib->form1();
	htmlFoot();
}
$mainlib->runx($sqlo);

htmlFoot("<hr>");