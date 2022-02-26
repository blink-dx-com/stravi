<?php
/**
 * reserve entries in container
 * @package obj.container.entryRes.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param string $id
 * @param $parx 
 *  $go
 */ 
session_start(); 

require_once ('reqnormal.inc');
require_once ("f.assocUpdate.inc");
require_once ("f.visuTables.inc");
require_once ('func_form.inc');

class oABS_CONTAINER_sub {
	function setContainer($absContID) {
		$this->id = $absContID;
	}
	
	function getMaxPosEntry(&$sqlo) {
		$absContID = $this->id;
		$sqlsel = 'max(pos) from ABS_CONT_ENTRY where ABS_CONTAINER_ID='.$absContID;
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$maxpos = $sqlo->RowData[0];
		return ($maxpos);
	}
}

class oCONTAINER_entryRes {
	
	function __construct($id, $parx, $go) {
		$this->parx = $parx;
		$this->id = $id;
		$this->go = $go;
		$this->AbsContLib = new oABS_CONTAINER_sub();
		$this->infox = NULL;
	}
	
	function getMaxPos(&$sqlo) {
		$sqlsel = 'max(pos) from CONT_HAS_CSUBST where CONTAINER_ID='.$this->id;
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$maxpos = $sqlo->RowData[0];
		return ($maxpos);
	}
	
	function initCheck(&$sqlo) {
		global $error;
		$FUNCNAME= 'initCheck';
		
		$parx= $this->parx;
		$sqlsel = 'ABS_CONTAINER_ID from CONTAINER where CONTAINER_ID='.$this->id;
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$absContID = $sqlo->RowData[0];
		if (!$absContID) {
			$error->set( $FUNCNAME, 1, 'container must link to an abstract container.' );
			return;
		}
		$this->AbsContLib->setContainer($absContID);
		$AbsMaxpos = $this->AbsContLib->getMaxPosEntry($sqlo);
		if (!$AbsMaxpos) {
			$error->set( $FUNCNAME, 2, 'container (abstract) contains no entries.' );
			return;
		}
		$this->infox['AbsMaxpos']=$AbsMaxpos;
		$ConMaxpos = $this->getMaxPos($sqlo);
		if ($ConMaxpos>=$AbsMaxpos) {
			$error->set( $FUNCNAME, 2, 'container entries are full. ('.$ConMaxpos.')' );
			return;
		}
		$this->infox['ConFreepos']=$ConMaxpos+1;
		
		if ( isset($parx['start']) ) {
			$this->infox['start-pos'] = $parx['start'];
			$this->infox['end-pos'] = $parx['end'];
		}
		
		$visuLib = new fVisuTables();
		$visuLib->showKeyValRaw( $this->infox, 'Input variables');
		echo '<br />';
	}
	
	function form1( ) {
		
	    $parx=array();
		$initarr   = NULL;
		$initarr["action"]      = $_SERVER['PHP_SELF'];
		$initarr["title"]       = "Parameters";
		$initarr["submittitle"] = "Next";
		$initarr["tabwidth"]    = "AUTO";
	
		$hiddenarr = NULL;
		$hiddenarr["id"]     = $this->id;
	
		$formobj = new formc($initarr, $hiddenarr, 0);
	
		$fieldx = array ( 
			"title" => "Start-Pos", 
			"name"  => "start",
			"object"=> "text",
			"val"   => $parx["start"], 
			"notes" => "start-pos"
			 );
		$formobj->fieldOut( $fieldx );
		$fieldx = array ( 
			"title" => "End-Pos", 
			"name"  => "end",
			"object"=> "text",
			"val"   => $parx["end"], 
			"notes" => "end-pos"
			 );
		$formobj->fieldOut( $fieldx );
	
		$formobj->close( TRUE );
	
	}
	
	function form2( ) {
		$initarr   = NULL;
		$initarr["action"]      = $_SERVER['PHP_SELF'];
		$initarr["title"]       = "Update";
		$initarr["submittitle"] = "Submit";
		$initarr["tabwidth"]    = "AUTO";
	
		$hiddenarr = NULL;
		$hiddenarr["id"]     = $this->id;
	
		$formobj = new formc($initarr, $hiddenarr, 1);
		$formobj->addHiddenParx( $this->parx );
		$formobj->close( TRUE );
	}
	
	function addEntries(&$sqlo) {
		global $error;
		$FUNCNAME= 'addEntries';
		
		$parx= $this->parx;
		
		if ( ($parx['start'] < $this->infox['ConFreepos']) or ($parx['start']> $this->infox['AbsMaxpos'])) {
			$error->set( $FUNCNAME, 1, 'Start must be between: '.$this->infox['ConFreepos'].':'. $this->infox['AbsMaxpos']);
			return;
		} 
		if ( ($parx['end']< $this->infox['ConFreepos']) or ($parx['end']> $this->infox['AbsMaxpos'])) {
			$error->set( $FUNCNAME, 1, 'End must be between: '.$this->infox['ConFreepos'].':'. $this->infox['AbsMaxpos'] );
			return;
		} 
		if ( $parx['end'] < $parx['start'] ) {
			$error->set( $FUNCNAME, 1, 'Start must be bigger than end!' );
			return;
		} 
		
		$assoclib = new  fAssocUpdate();
		$assoclib->setObj( $sqlo, 'CONT_HAS_CSUBST', $this->id );
		
		$pos = $parx['start'];
		
		$argu=array();
		$argu['RESERVED']=1;
		
		$cnt=0;
		while ( $pos <= $parx['end'] ) {
			$argu['POS']= $pos;
			if ( $this->go==2) {
				$assoclib->insert( $sqlo, $argu );
				if ($error->Got(READONLY))  {
					$error->set( $FUNCNAME, 1, 'Error on pos:'.$pos );
					return;
				}
			}
			$pos++;
			$cnt++;
		}
		echo '... ready<br />';
	}
}

// --------------------------------------------------- 
global $error, $varcol;

$error = & ErrorHandler::get();
$sqlo  = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();

$id 		= $_REQUEST['id'];
$parx 		= $_REQUEST['parx'];
$go 		= $_REQUEST['go'];
$tablename	= 'CONTAINER';
$title		= 'reserve entries';

$infoarr			 = NULL;
$infoarr['scriptID'] = '';
$infoarr['title']    = $title;
$infoarr['form_type']= 'obj'; 
$infoarr['design']   = 'norm';
$infoarr['obj_name'] = $tablename;
$infoarr['obj_id']   = $_REQUEST['id'];
$infoarr['checkid']  = 1;
$pagelib = new gHtmlHead();
$headarr = $pagelib->startPage($sqlo, $infoarr);

$accCheckArr = array('tab'=>array('write'), 'obj'=>array('insert') );
$pagelib->do_objAccChk($sqlo, $accCheckArr);
$pagelib->chkErrStop();

$MainLib = new  oCONTAINER_entryRes($id, $parx, $go);
$MainLib->initCheck($sqlo);
$pagelib->chkErrStop();

if ( !$go ) {
	$MainLib->form1( );
	$pagelib->htmlFoot();
}

if ( $go==1 ) {
	$MainLib->form2( );
}

$MainLib->addEntries($sqlo);
$pagelib->chkErrStop();

$pagelib->htmlFoot();
