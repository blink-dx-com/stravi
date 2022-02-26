<?php
/**
 * execute a MODULE
 * @namespace core::gui
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param   string xid - id of MODULE 
 */
session_start(); 

require_once ('reqnormal.inc');
require_once ('gui/f.plugin.inc');

class gExeManage {
	var $CLASSNAME='gManagePage';
	
	/**
	 * check the plugin-module
	 * @param $mod
	 * @return string
	 */
	function analyseModID(&$sqlo, $xid) {
		global $error;
		$FUNCNAME = $this->CLASSNAME.':analyseModID';
		
		$baseDir = '../../';
		
		$sqlsel = 'LOCATION  from MODULE where MXID='.$xid ;
		$sqlo->Quesel($sqlsel);
		if ( $sqlo->ReadRow() ) {
			$LOCATION = $sqlo->RowData[0];
		} else $LOCATION=NULL;
		
		if ($LOCATION==NULL) {
			$error->set( $FUNCNAME, 1, 'Plugin with ID: '.$xid.' not found.' );
			return;
		}
		
		$relFile  = $baseDir . $LOCATION;
		
		if (!file_exists($relFile)) {
			$error->set( $FUNCNAME, 2, 'Plugin "'.$relFile.'" not found.' );
			return;
		}
		require_once($relFile);
		
		$pureModPathArr = explode('/',$LOCATION);
		$namePos   = sizeof($pureModPathArr)-1;
		$modFile   = $pureModPathArr[$namePos];
		$pureModName = substr($modFile,0,-4);
		$classname = $pureModName.'_XPL';
		$classname = str_replace('.','_',$classname);
		if (!class_exists($classname)) {
			$error->set( $FUNCNAME, 5, 'Plugin "'.$relFile.'": class:'.$classname.' not found.' );
			return;
		}
		
		return ($classname);
	}
	
	// general error, before the plugin was initialized ...
	function pageInitError(&$sqlo) {
		global $error;
		
		$title		= 'Execute a module';
		$infoarr			 = NULL;
		$infoarr['scriptID'] = '';
		$infoarr['title']    = $title;
		$infoarr['form_type']= 'tool'; // 'tool', 'list'
		$infoarr['design']   = 'norm';
		$infoarr['locrow']   = array( array('home.php', 'home') );
		$pagelib = new gHtmlHead();
		$pagelib->startPage($sqlo, $infoarr);
		
		$error->printAll();
		
		$pagelib->htmlFoot();
		
	}
}


global $error, $varcol;
$FUNCNAME='f.exe.php';

$error = & ErrorHandler::get();
$sqlo  = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();

$xid 		= $_REQUEST['xid'];

$orglib = new gExeManage();
$classname = $orglib->analyseModID($sqlo, $xid);
if ($error->Got(READONLY))  {
	$error->set( $FUNCNAME, 1, 'Error on Plugin: "'.$xid.'"' );
	$orglib->pageInitError($sqlo);
}

$plugLib = new $classname($sqlo);
$plugLib->register();
$plugLib->startHead();
if ($error->Got(READONLY))  {
	$error->printAll();
	$plugLib->htmlFoot();
}

$plugLib->startMain();
if ($error->Got(READONLY))  {
	$error->printAll();
}
$plugLib->htmlFoot();

