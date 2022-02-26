<?php
/**
 * edit one element of GLOBALS, called by glob.syscheck.inc
 * - optional: include VARNAME special code
 * - file: o.GLOBALS.edit/VARNAME.inc
 * @package obj.globals.editelem.php
 * @swreq UREQ:0001115
 * @see 89_1002_SDS_code.pdf
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param    $name 
  		 $value
		 $notes
		 $go
		 $xact = "remove"
 * @version $Header: trunk/src/www/pionir/rootsubs/obj.globals.editelem.php 59 2018-11-21 09:04:09Z $
 */



extract($_REQUEST); 
session_start(); 


require_once ('db_access.inc');
require_once ('globals.inc');
require_once ('func_head.inc');
require_once ('access_check.inc');
require_once ('table_access.inc');
require_once ('func_form.inc'); 
require_once ('insert.inc'); 
require_once ('f.update.inc');

/**
 * abstract class
 * @author steffen
 *
 */
class oGLOBALS_xEdit {
	function __construct(&$sqlo) {
		
	}
	
	public function setParams($parx) {
	
	}
	
	public function preActions(&$sqlo) {
		
	}
	
	public function postActions(&$sqlo, $realData) {
	
	}
	
	/**
	 * show some infos
	 * @param  $sqlo
	 */
	public function showInfos(&$sqlo, $realData) {
	
	}
}

/**
 * default implemenataion, if no INCLUDE-file exists for VAR
 * @author steffen
 *
 */
class _DEFAULT_XGED extends oGLOBALS_xEdit {
}

class oGLOBALS_edHelp {
	
	/**
	 * 
	 * @param string $mod VARNAME
	 * @return array ('class'=>, 'found'=>0,1)
	 */
	function includeMod($mod) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	
		if ($mod==NULL) {
			$error->set( $FUNCNAME, 1, 'no VARNAME given.' );
			return;
		}
		
		$BASE_DIR = 'o.GLOBALS.edit';
		$relFile  = $BASE_DIR.'/'.$mod.'.inc';
	
		if (!file_exists($relFile)) {
			return array('class'=>'_DEFAULT_XGED', 'found'=>0);
		}
		
		require_once($relFile);
		$classname = $mod .'_XGED';
		$classname = str_replace('.','_',$classname);
		if (!class_exists($classname)) {
			$error->set( $FUNCNAME, 5, 'Plugin "'.$relFile.'": class:'.$classname.' not found.' );
			return;
		}
	
		return array('class'=>$classname, 'found'=>1 );
	}
}

$error = & ErrorHandler::get();
$sqlo   = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();


$title       = 'Edit one elements of globals';

$infoarr=array();
$infoarr["title"] = $title;
$infoarr["form_type"]= "tool";
$infoarr["obj_name"] = "GLOBALS";
$infoarr["obj_id"]   = $name;
$infoarr['locrow']   = array( array('../glob.syscheck.php', 'System check') );
$infoarr["show_name"]= 1;


$pagelib = new gHtmlHead();
$pagelib->startPage($sqlo, $infoarr);

$infox["DbLoginDeny"] = array(  "val"=>"", "notes"=>"If a text is set, it denies the login for all users.\nThe text can be the reason for denial.",
								"info" => "set it to deny logins");

echo "<ul>";

if ( $_SESSION['sec']['appuser'] != "root" ) { // !$_SESSION['s_suflag'] 
     htmlErrorBox( "Error",   
     "Only root can execute this!",
     "For security reason it is not allowed for common users" );
     htmlFoot();
}

if ( $name=="" ) htmlFoot('ERROR', 'Please give the NAME of the element');

$orglib     = new oGLOBALS_edHelp();
$inludeInfo = $orglib->includeMod($name);
if ($error->Got(READONLY))  {
	$error->set( $FUNCNAME, 1, 'Error on VARIABLE-Plugin for variable "'.$name.'"' );
	$pagelib->chkErrStop();
	exit;
}

$classname = $inludeInfo['class'];
if ($inludeInfo['found']) {
	echo 'INFO: variable specific plugin found.<br>'."\n";
}

$varActionLib = new $classname($sqlo);


$isSet = 0; // row exists ?
$sqlo->query("select value, notes from globals where NAME='".$name."'");
if ($sqlo->ReadRow() ) {
	$isSet   = 1;
}

if ($go) {
	
	// do UPDATE
	
	if (!isset($value)) htmlFoot('ERROR', 'Missing value!');
	
	$argu = NULL;
	$argu["NAME"]  = $name;
	$argu["VALUE"] = $value;
	$argu["NOTES"] = $notes;
	
	$varActionLib->setParams($argu);
	$varActionLib->preActions($sqlo);
	
	if ($error->Got(READONLY))  {
		$error->set( $FUNCNAME, 10, 'Error on preActions of VARIABLE-Plugin' );
		$pagelib->chkErrStop();
		exit;
	}
	
	if ($xact=="remove") {
		$sqls = "delete from GLOBALS where name='".$name."'";
		$sqlo->query($sqls);
		unset($value);
		unset($notes);
		echo "<font color=green>...deleted...</font><br><br>";
	} else {
		if ($isSet) $retval = gObjUpdate::update_row($sqlo, "GLOBALS", $argu);
		else $retval = insert_row($sqlo, "GLOBALS", $argu);
		echo "<font color=green>...updated...</font><br><br>";
	}
	if ($error->Got(READONLY))  {
		$error->set( $FUNCNAME, 11, 'Error on update.' );
		$pagelib->chkErrStop();
		exit;
	}
	
	$realData=array();
	$sqlo->query("select value, notes from globals where NAME='".$name."'");
	if ($sqlo->ReadRow() ) {
		$realData['VALUE'] = $sqlo->RowData[0];
		$realData['NOTES'] = $sqlo->RowData[1];
	}
	
	$varActionLib->postActions($sqlo, $realData);
	
	if ($error->Got(READONLY))  {
		$error->set( $FUNCNAME, 10, 'Error on postActions of VARIABLE-Plugin' );
		$pagelib->chkErrStop();
		exit;
	}
	
}

$realData=array();

$sqlo->query("select value, notes from globals where NAME='".$name."'");
if ($sqlo->ReadRow() ) {
	$realData['VALUE'] = $sqlo->RowData[0];
	$realData['NOTES'] = $sqlo->RowData[1];
	
	$isVal   = $realData['VALUE'];
	$isNotes = $realData['NOTES'];
	
} else {
	$isVal   = $infox[$name]["val"];
	$isNotes = $infox[$name]["notes"];
}

$showVal  = $value;
$showNotes= $notes;
if (!isset($value))  $showVal  = $isVal;
if (!isset($notes))  $showNotes= $isNotes;


$initarr   = NULL;
$initarr["action"]      = $_SERVER['PHP_SELF'];
$initarr["title"]       = "Edit globals variable";
$initarr["submittitle"] = "Submit";
$initarr["tabwidth"]    = "AUTO";
$initarr["fsize"]    	= "50";
$hiddenarr = NULL;

$formobj = new formc($initarr, $hiddenarr, 0);

$fieldx = array ( "title" => "Variable", "name"  => "name", "namex" => 1,
		"object" => "info2", "val"   => $name );
$formobj->fieldOut( $fieldx );

$fieldx = array ( "title" => "Value", "name"  => "value", "namex" => 1, 'fsize'=>120,
		"object" => "text", "val"   => $showVal );
$formobj->fieldOut( $fieldx );

$fieldx = array ( "title" => "Notes", "name"  => "notes", "namex" => 1,
		"object" => "textarea", "val"   => $showNotes, "inits" => array("cols"=>50) );
$formobj->fieldOut( $fieldx );

$fieldx = array ( "title" => "Delete it", "name"  => "xact", "namex" => 1,"colspan"=>2,
		"object" => "checkbox", "val"   => "", "inits" => "remove", "notes"=>"remove the variable" );
$formobj->fieldOut( $fieldx );

$formobj->close( TRUE );

echo "<br>\n";

$varActionLib->showInfos($sqlo, $realData);


htmlFoot();

