<?php

require_once 'f.clipboard.inc';
require_once 'o.ABS_WORKLIST.subs.inc';
require_once 'o.WORKLIST.subs.inc';
require_once 'o.WORKLIST.mod.inc';

/**
 * paste objects, only objects !!!
 * 
 * 
 * - go=0
 * - check for clip-board: WORKLIST-object : SRC
 * - check if DEST != SRC
 * - SRC,DEST must be same type
 * - select column-name
 * - go=1:
 * - check destination-data
 * - paste data from SOURCE=>DESTINATION
 *    
 * @package o.WORKLIST.pasteColDat.inc
 * @swreq   UREQ:0001053: o.WORKLIST > paste data of one column from source worklist 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param   $_REQUEST['id'] id of destination WORKLIST
 * @param   $_REQUEST['go'] 0,1
 * @param   $_REQUEST[parx]['colid'] column-id
 *   
 */


// worker class
class _pasteColumnC {
	function __construct(&$_html) {
		$this->_html=&$_html;
	}
	
	/**
	 * initial error checks
	 * @param $sqlo
	 * @param $objid
	 * @return array(code,text=
	 *    code > 0 : o.k.
	 *    code < 0 : error
	 */
	function setObj(&$sqlo, $objid) {
		$this->destID = $objid;
		
		$tablename='WORKLIST';
		
		$workNice = tablename_nice2($tablename) ;
		// get source
		$answer = clipboardC::obj_get( 'WORKLIST' );
		if (!$answer[0]) {
			return array(-1, 'keine QUELLEN-'.$workNice.' im Clipboard.'); 
		}
		$this->srcID = $answer[0];
		
		if ($this->destID==$this->srcID) {
			return array(-2, 'ID of source-worklist is the same ID of destination-worklist'); 
		}
		
		// check types
		$typeSrc  = glob_elementDataGet( $sqlo, $tablename, 'WORKLIST_ID', $this->srcID, 'ABS_WORKLIST_ID');
		$typeDest = glob_elementDataGet( $sqlo, $tablename, 'WORKLIST_ID', $this->destID, 'ABS_WORKLIST_ID');
		if (!$typeSrc or ($typeSrc!=$typeDest)) {
			return array(-3, 'worklists must be of same type'); 
		}
		
		$this->abs_work_id = $typeSrc;
		$this->absWorkLib = new oABS_WORKLIST_subs();
		$this->absWorkLib->setObj($this->abs_work_id);
		
		return array(1,'');
		
	}
	
	function getSrcID() {
		return $this->srcID;
	}
	
	/**
	 * get column names
	 * - only substances allowed ... 
	 * @param $sqlo
	 */
	function getColArr(&$sqlo) {
		
		$this->_html->line_out('... search for selectable columns; only substance-columns  are allowed.');
		
		$nameAttribs = $this->absWorkLib->getNameAttribs($sqlo);
		if (!sizeof($nameAttribs)) {
			return;
		}
		
		
		$nameFormArr=NULL;
		foreach( $nameAttribs as $xcol=>$valarr) {
			if ($valarr['t']=='CONCRETE_SUBST') {
				// this object type is allowed ...
				$nameFormArr[$xcol]=$valarr['title'];
			}
		}
		reset ($nameAttribs); 
		return $nameFormArr;
	}
	
	/**
	 * - hat source Daten ???
	 * - check length
	 * @param $sqlo
	 */
	function _preCheckPaste(&$sqlo, $colid) {
		
	}
	
	/**
	 * - hat source Daten ???
	 * - check length
	 * @param $sqlo
	 */
	function pasteColumn(&$sqlo, $colid) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$nameAttribs = $this->absWorkLib->getNameAttribs($sqlo);
		$thisColAttribs = $nameAttribs[$colid];
		$thisTableType = $thisColAttribs['t'];
		
		if ($thisTableType==NULL) {
			$error->set( $FUNCNAME, 1, 'No object-table defined for column-id '.$colid );
			return;
		}
	
		
		$this->srcAnaLib  = new oWORKLIST_subs($this->srcID);
		$srcAttribs = $this->srcAnaLib->init($sqlo);
		$this->destAnaLib = new oWORKLIST_subs($this->destID);
		$destAttribs = $this->destAnaLib->init($sqlo);
		
		if ($srcAttribs['maxy']>$destAttribs['maxy']) {
			$error->set( $FUNCNAME, 1, 'Die QUELLE hate mehr Zeilen als das ZIEL.' );
			return;
		}
		
		// has data ???
		$hasData=0;
		$y=0;
		$x=$colid;
		while ( $y<=$srcAttribs['maxy'] ) {
			$destVals = $this->destAnaLib->getCellVals($sqlo, array($x,$y));
			if ($destVals['OBJID']>0) {
				$hasData=1;
			}
			$y++;
		}
		if ($hasData) {
			$error->set( $FUNCNAME, 2, 'Die ZIEL-Spalte hat Daten. Das ist nicht hier nicht erlaubt.' );
			return;
		}
		
		// now paste ...
		$workModLib = new oWORKLIST_mod();
		$workModLib->setObj($sqlo, $this->destID);
		$y=0;
		$x=$colid;
		while ( $y<=$srcAttribs['maxy'] ) {
			$srcVals = $this->srcAnaLib->getCellVals($sqlo, array($x,$y));
			
			$params=array(
				'TABLE_NAME'=> $srcVals['TABLE_NAME'],
				'OBJID'     => $srcVals['OBJID']
			);
			$workModLib->updateCell($sqlo, array($x,$y), $params);
			if ($error->Got(READONLY))  {
				$error->set( $FUNCNAME, 10, 'error on update on row:'.$y );
				return;
			}
			$y++;
		}
		
	}
}

// plugin
class o_WORKLIST_pasteColDat_XPL extends gPlugin {
	
	function __construct(&$sqlo) {
		parent::__construct($sqlo);
	}
	
	function register() {
		$tablename	= 'WORKLIST';
		$title		= 'Daten aus Spalte einfuegen';
		
		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'obj'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['obj_id']   = $_REQUEST['id'];
		$this->infoarr['checkid']  = 1;
		
		
		$this->infoarr2 = array( 
				'accCheck' => array('tab'=>array('read','write'), 'obj'=>array('insert') )
				);
				
		
	}
	
	function form1($colArr) {
		
		$initarr   = NULL;
		$initarr["title"]       = "Spalte waehlen"; // title of form
		$initarr["submittitle"] = "Submit"; // title of submit button
		$initarr["tabwidth"]    = "AUTO";   // table-width: AUTO

		$hiddenarr = NULL; // hidden form variables
		$hiddenarr["id"]     = $this->objid; // is a required input parameter

		// init form, uses class formc
		$formobj = $this->_html->formInit($initarr, $hiddenarr, 0);

		$fieldx = array ( // form-field definition
			"title"   => "Spalte", 
			"name"    => "colid",
			"object"  => "select",
			"inits"	  => $colArr,
			"val"     => 0, 
			"notes"   => "nur Substance-Spalten sind erlaubt ..."
			 );
		$formobj->fieldOut( $fieldx ); // output the form-field

		$formobj->close( TRUE ); // close form, sshow submit-button
	}

	function startMain() {
		global $error;
		
		$FUNCNAME='startMain';
		
		$tablename = 'WORKLIST';
		
		$this->objid   = $_REQUEST['id'];
		$go      = $_REQUEST['go'];
		$parx    = $_REQUEST['parx'];
		$sqlo   = &$this->_sqlo;
		
		$anaLib = new _pasteColumnC($this->_html);
		$answer = $anaLib->setObj($sqlo, $this->objid);
		$srcid  = $anaLib->getSrcID();
		if ($srcid) {
			require_once ("f.objview.inc");	
			$objLinkLib = new fObjViewC();
			$htmlStr = $objLinkLib->bo_display( $sqlo, $tablename, $srcid );
			$this->_html->line_out('QUELLE: '.$htmlStr);
		}
		
		if ($answer[0]<0) {
			$this->htmlFoot('ERROR',$answer[1], '; errcode:'.$answer[0]);
		}
		
		
		$colArr = $anaLib->getColArr($sqlo);
		if (!sizeof($colArr)) {
			$this->htmlFoot('ERROR','Keine gueltigen Spalten-Definition für diese Aktion gefunden. Frage mit den Worklist-Designer.');
		}
		
		if (!$go) {
			$this->form1($colArr);
			return;
		}
		
		if (!$parx['colid']) {
			$this->htmlFoot('ERROR','Gib eine Spalte an.');
		}
		
		$anaLib->pasteColumn($sqlo, $parx['colid']);
		if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 1, 'error on update' );
			return;
		}
		$this->_html->line_out('paste o.k.');
		
		require_once ( "javascript.inc" );
		js__location_replace('edit.tmpl.php?t=WORKLIST&id='.$this->objid );  
		
	}
	
}