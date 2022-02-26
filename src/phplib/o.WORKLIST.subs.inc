<?php
require_once 'o.S_OBJLINK.subs.inc';
require_once 'o.ABS_WORKLIST.subs.inc';
require_once 'o.ABS_WORKLIST.subs2.inc';

/**
 * analysis methods for WORKLIST
 * @package o.WORKLIST.subs.inc
 * @swreq UREQ:0000932 db.WORKLIST: Messauftrag organisieren
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/phplib/o.WORKLIST.subs.inc 59 2018-11-21 09:04:09Z $
 */
class oWORKLIST_subs {
	
	private $wl_pluginLib; // the Plugin class derived from worklistPlugin
	
	function __construct($objid=NULL) {
		$this->objid = NULL;
		$this->wl_pluginLib = NULL;
		if ($objid) $this->setObj($objid);
	}
	
	/**
	 * initialize worklist, prepare for analysis
	 * @param $sqlo
	 * @param $objid
	 * @return -
	 */
	function setObj($objid) {
		$this->objid=$objid;
	}
	
	function getMaxRowCol() {
		return array($this->maxy, $this->maxx);
	}
	
	// init worklist-object
	function init(&$sqlo) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		if (!$this->objid) {
			sys_error_my( 'class '.__CLASS__.' is not initialized.' );
		}
		$this->absEntries = NULL;
	
		$sqlsel = 'max(YPOS), max(XPOS) from WORKLIST_ENTRY where WORKLIST_ID='.$this->objid;
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$this->maxy = $sqlo->RowData[0];
		$this->maxx = $sqlo->RowData[1];
		
		// get column defs
		$sqlsel = 'ABS_WORKLIST_ID from WORKLIST where WORKLIST_ID='.$this->objid;
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$this->absWorkId = $sqlo->RowData[0];
		
		if ($this->absWorkId) {
			$absWorkLib = new oABS_WORKLIST_subs($this->absWorkId);
			$this->colOrder = $absWorkLib->getColOrder($sqlo);
		}
		
		return array(
			'maxy'=>$this->maxy, 
			'maxx'=>$this->maxx, 
			'absWorklistId'=>$this->absWorkId,
			'colOrder' => $this->colOrder // order of columns in ABS_WORKLIST
		); 
	}
	
	/** 
	 * get planned attributes per column
	 * @return array array('t'=>$table, 'typ'=>, 'note'=>, 'collect'=>, 'title'=> )
	 */
	function getPlannedAttribs(&$sqlo) {
		if (!$this->absWorkId) return;
		
		$sqlsel = "XPOS, TABLE_NAME, TYPE, SH_NOTES, AUTO_COLLECT, OPTIONS from ABS_WORKLIST_ENTRY where ABS_WORKLIST_ID=".$this->absWorkId.' order by XPOS';
		$sqlo->Quesel($sqlsel);
		while ( $sqlo->ReadRow() ) {
		    $xpos  = $sqlo->RowData[0];
		    $table = $sqlo->RowData[1];
		    $absEntries[$xpos]=array('t'=>$table, 'typ'=>$sqlo->RowData[2], 'note'=>$sqlo->RowData[3], 'collect'=> $sqlo->RowData[4], 'opt'=>$sqlo->RowData[5]);
		    
		    $OPTIONS = $absEntries[$xpos]['opt'];
	    	if ($OPTIONS!=NULL) {
	    		$colOptArr = oABS_WORKLIST_subs2::option2arr($OPTIONS);
	    		$title     = $colOptArr['title'];
	    		$absEntries[$xpos]['title'] = $title;
	    	}
		}
		return $absEntries;
	}
	
	// get mother object
	function getMotherObj(&$sqlo) {
		if (!$this->objid) {
			sys_error_my( 'class '.__CLASS__.' is not initialized.' );
		}
		
		$answerArr = oS_OBJLINK_subs::getMoObjbyKey($sqlo, 'WORKLIST', $this->objid, 'MOTHER');
		if ($answerArr[0]!=NULL and $answerArr[1]!=NULL)
			return array('t'=>$answerArr[0], 'id'=>$answerArr[1]);
		else return;
	}
	
	// get worklist objects of mother object
	static function getWorklistsOfMothers(&$sqlo, $table, $motherObjid) {
		$workObjects = oS_OBJLINK_subs::getChiObjByChiTab($sqlo, $table, $motherObjid, 'WORKLIST');
		return $workObjects;
	}
	/**
	 * get result of cell entry
	 * @param $sqlo
	 * @param array $xy (x,y)
	 * @return array $entry
	 */
	function getCellVals(&$sqlo, $xy) {
		if (!$this->objid) {
			sys_error_my( 'class '.__CLASS__.' is not initialized.' );
		}
		$sqlsel = '* from WORKLIST_ENTRY where '.
			' WORKLIST_ID='.$this->objid. 
			' and XPOS='.$xy[0].' and YPOS='.$xy[1];
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadArray();
		return $sqlo->RowData;
	}
	
	/**
	 * object exists in worklist, column=$colx ?
	 * @param $sqlo
	 * @param $colx xpos
	 * @param $objid
	 * @return boolean 0,1 not exists/exists
	 */
	function objExistsInList(&$sqlo, $colx, $objid) {
		if (!$this->objid) {
			sys_error_my( 'class '.__CLASS__.' is not initialized.' );
		}
		$sqlsel = '* from WORKLIST_ENTRY where '.
			' WORKLIST_ID='.$this->objid. 
			' and XPOS='.$colx.' and OBJID='.$objid;
		$sqlo->Quesel($sqlsel);
		if ($sqlo->ReadRow() ) return 1;
		else return 0;
	}

	/**
	 * get max row-number of object in xcol 
	 * @param $sqlo
	 * @param $xcol ID of column
	 * @return $xrow
	 */
	function getMaxRowOfCol(&$sqlo, $xcol) {
		$sqlsel = 'max(YPOS) from WORKLIST_ENTRY where WORKLIST_ID='.$this->objid. ' and XPOS='.$xcol;
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$maxy = $sqlo->RowData[0];
		return $maxy;
	}
	
	/**
	 * return 1, if column has data
	 * @param $type [OBJID] or KEY or SH_NOTES or VALUE
	 */
	function colHasData(&$sqlo, $col, $type=NULL) {
		$sqlsel = 'XPOS from WORKLIST_ENTRY where WORKLIST_ID='.$this->objid.
			' and XPOS='.$col.' and OBJID is not NULL';
		$sqlo->Quesel($sqlsel);
		if ( $sqlo->ReadRow() ) {
			$hasData=1;
		} else $hasData=0;
		return $hasData;
	}
	
	/**
	 * get all worklists, where object ($table, $objid) is member
	 * @return array of worklist-IDs
	 */
	function getWliOfObj(&$sqlo, $table, $objid ) {
	
		if ($table==NULL or !$objid) return; // no search possible
	
		$outarr=NULL;
		$sqlsel = 'WORKLIST_ID from WORKLIST_ENTRY '.
			' where TABLE_NAME='.$sqlo->addQuotes($table).' and OBJID='.$objid.' order by WORKLIST_ID';
		$sqlo->Quesel($sqlsel);
		while ( $sqlo->ReadRow() ) {
			$outarr[] = $sqlo->RowData[0];
		} 
		return $outarr;
	}
	
	/**
	 * get all data from worklist of type=$tablename, order by YPOS
	 * max 5000 objects allowed (to prevent MEM-problem)
	 * @param $sqlo
	 * @param $tablename
	 * @return array( list of $objid )
	 */
	function getAllDataOfType(&$sqlo, $tablename) {
		$outarr = array();
		$sqlsel = 'OBJID from WORKLIST_ENTRY where WORKLIST_ID='.$this->objid.
			' and TABLE_NAME='.$sqlo->addQuotes($tablename).' and OBJID is not NULL order by YPOS';
		$sqlo->Quesel($sqlsel);
		$cnt=0;
		while ( $sqlo->ReadRow() ) {
			if ($cnt>5000) {
				throw new Exception('Too many objects (max 5000 allowed).');
			}
			$outarr[] = $sqlo->RowData[0];
			$cnt++;
		}
		return $outarr;
	}
	
	/**
	 * get SQL string for: get data of selected worklists of type=$tablename 
	 * NO order
	 * @param $sqlo
	 * @param $tablename
	 * @return array( list of $objid )
	 */
	static function SQL_getAllDataOfType(&$sqlo, $tablename, $wl_ids) {
		
		$wl_id_str = implode(', ',$wl_ids);
		$sqlsel = 'OBJID from WORKLIST_ENTRY where WORKLIST_ID in ('.$wl_id_str.')'.
			' and TABLE_NAME='.$sqlo->addQuotes($tablename).' and OBJID is not NULL';
		return $sqlsel;
	}
	
	/**
	 * get NUMBER of cells from worklist of type=$tablename
	 * @param $sqlo
	 * @param $tablename
	 * @return int $number
	 */
	function getNumCellsOfType(&$sqlo, $tablename) {
		
		$sqlsel = 'count(OBJID) from WORKLIST_ENTRY where WORKLIST_ID='.$this->objid.
		' and TABLE_NAME='.$sqlo->addQuotes($tablename).' and OBJID is not NULL';
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$cnt = $sqlo->RowData[0];
		
		return $cnt;
	}
}
