<?php
/**
 * - plugin table WORKLIST
 *    
 * @package o.WORKLIST.guimod.inc
 * @swreq UREQ:0000932 db.WORKLIST: Messauftrag organisieren
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param $_REQUEST['id'] id of CYTCART
 * @param $_REQUEST['act'] 
 *   'new' : create empty rows
 *   'autocollect'    - autocollect
 *   'update.plan'    - update all planned cells: cell[y][x] = ID of object or other values
 *   'status.assignMe'- assign worklist to me
 *   'status.finish'  - finish worklist
 *   'status.measured'- worklist measured
 *   'newrow'		  - new row
 * @param $parx['rows']  number of rows for $act='new',
 * @param $parx['input'] ['ID'], 'NAME' => then cell contains object-names
 * @param $_REQUEST['cell'] used by 'update.plan' array(y,x)=objid
 * @version $Header: trunk/src/plugin/o.WORKLIST.guimod.inc 59 2018-11-21 09:04:09Z $
 */
require_once ("o.WORKLIST.mod.inc");
require_once ("o.WORKLIST.subs.inc");
require_once ("o.ABS_WORKLIST.subs.inc");
require_once ("o.ABS_WORKLIST.subs2.inc");
require_once ('access_lock.inc');
require_once ('o.CCT_ACCLOG.gui.inc');
require_once ("class.filex.inc");

/**
 * autocollect one column
 * @author steffen
 *
 */
class o_WORKLIST_autoColOneCol {
	var $xpos;
	
	function __construct(&$workModLib, $workObjId, $mod) {
		$this->objid = $workObjId;
		$this->_mod = $mod;
		$this->workModLib=$workModLib;
		$this->absWorkLib2 = new oABS_WORKLIST_subs2();
		$this->concWorkLib = new oWORKLIST_subs($workObjId);
		
		$this->updateWasActive=0;
	}
	
	function initCol(&$sqlo, $tmpInfoArr, $xpos) {
		global $error;
		$FUNCNAME= __CLASS__.':initCol';
		
		$this->xpos=$xpos;
		$this->formInit=0;
		
		// analyse all rows of column $xpos
		$collectStr = $tmpInfoArr['auto_collect'];
		$tablename  = $tmpInfoArr['table'];
		

		$collectInfoArr = $this->absWorkLib2->anaAutoCollectStr($collectStr);
		if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 1, 'AUTO_COLLECT error on column '.$xpos );
			return;
		}
		if (!is_array($collectInfoArr)) {
			return; // no valid collect string
		}
		
		echo '... Update column '.$xpos.'; table '.tablename_nice2($tablename).'<br><br>';
		
		$this->tablename=$tablename;
		$pkname    = PrimNameGet2($tablename);
		$sqlselAfterCol = ' from '.$tablename.' where ';
		$this->sqlselPre = $pkname.$sqlselAfterCol;
		$this->sqlselCnt = 'count(1)'.$sqlselAfterCol;
		if ( $_SESSION["userGlob"]["g.debugLevel"]>0 ) {
			echo 'SQL-command: '.$this->sqlselPre.' where '.$collectInfoArr['sqlCond'].'<br />';
		}
		return $collectInfoArr;
	}
	
	function closeUpdate(&$sqlo) {
		if ($this->updateWasActive) $this->workModLib->close($sqlo);
	}
	
	function closeForm() {
		if (!$this->formInit) return;

		
		echo "<input type=submit value=\"Update your selection\">\n"; // SUBMIT
		echo '</form>'."\n";
		echo "<br>\n";
		return 1;
	}
	
	/**
	 * aut collect one cell
	 * @param $sqlo
	 * @param $ypos
	 * @param $sqlAfter
	 * @return -
	 */
	function collectCell(&$sqlo, $ypos, $sqlAfter) {
		global $error;
		$FUNCNAME= __CLASS__.':_updateCell';
		
		$tablename = $this->tablename;
		$xpos	   = $this->xpos;
		$sqlselPre = $this->sqlselPre;
		$sqlselCnt = $this->sqlselCnt;
		
		
		
		$oldEntry= $this->concWorkLib->getCellVals($sqlo, array($xpos,$ypos) );
		if ($oldEntry['TABLE_NAME']!=NULL and  $oldEntry['OBJID']!=NULL) {
			echo '<span style="color:#808000; font-weight:bold;">No update:</span> cell already has a value.';
			return;
		}
		
		$sqlsel  = $sqlselCnt . $sqlAfter;
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$objcnt = $sqlo->RowData[0];
		if (!$objcnt) {
			return;
		}
		
		if ($objcnt>1) {
			echo $objcnt.' objects found! <b>no auto-update!</b> ';
			if ($objcnt>40) return; // impossible to manually select
			
			if ( !$this->formInit ) {
				echo "<form style=\"display:inline;\" method=\"post\" ".
		 			" name=\"editform\"  action=\"".$_SERVER['PHP_SELF']."\" >\n";
				echo "<input type=hidden name='mod' value='".$this->_mod."'>\n";
				echo "<input type=hidden name='act' value='update.plan'>\n";
				echo "<input type=hidden name='go' value='1'>\n";
				echo "<input type=hidden name='id' value='".$this->objid."'>\n";
				$this->formInit=1;
			}
			$sqlsel  = $sqlselPre . $sqlAfter;
			$sqlo->Quesel($sqlsel);
			$objsel=NULL;
			while ( $sqlo->ReadRow() ) {
				$objid = $sqlo->RowData[0];
				$objsel[]=$objid;
			}
			echo ' <b>You must select: </b><br>';
			foreach( $objsel as $dummy=>$objid) {
				$nice = obj_nice_name ( $sqlo, $tablename, $objid );
				echo '&nbsp; <input type=radio name="cell['.$ypos.']['.$xpos.']" value='.$objid.' >';
				echo ' '.$nice.'<br>'."\n";
			}
			reset ($objsel); 
			
			return;
		}
			
		$sqlsel  = $sqlselPre . $sqlAfter;
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$objid = $sqlo->RowData[0];
		if ($objid)  {
			// connect to row
			echo ' <span style="color:green; font-weight:bold;">update:</span> '. obj_nice_name ( $sqlo, $tablename, $objid );
			$xy    = array($xpos,$ypos);
			$params= array('TABLE_NAME'=>$tablename, 'OBJID'=>$objid);
			$this->workModLib->updateCell($sqlo, $xy, $params);
			$this->updateWasActive=1;
			if ($error->Got(READONLY))  {
				$error->set( $FUNCNAME, 3, 'error on update of cell: '.$xpos.','.$ypos );
				return;
			}
		}
	}
}



class o_WORKLIST_guimod_XPL extends gPlugin {
	
	var $absEntries; // tablenames
	
	function __construct(&$sqlo) {
		parent::__construct($sqlo);
	}
	
	function register() {
		$tablename	= 'WORKLIST';
		$title		= 'Modify';
		
		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title']    = $title;
		$this->infoarr['title_sh'] = 'Modify';
		$this->infoarr['form_type']= 'obj'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['obj_id']   = $_REQUEST['id'];
		$this->infoarr['checkid']  = 1;
		
		
		$this->infoarr2 = array( 
				'accCheck' => array('tab'=>array('read', 'write'), 'obj'=>array('write') )
				);
					
	}
	
	function _act_NEW(&$sqlo, $rows) {
		global $error;
		$FUNCNAME= __CLASS__.':_act_NEW';
		
		$this->_html-> line_out('... insert '.$rows.' rows.');
		$i = 1;
		$params=array();
		while ($i<=$rows) {
			$xy=array(1,$i);
			$this->workModLib->updateCell($sqlo, $xy, $params);
			if ($error->Got(READONLY))  {
				$error->set( $FUNCNAME, 1, 'error on insert of row: '.$i );
				return;
			}
			$i++;
		}
		$this->workModLib->close($sqlo);
		$this->_html-> line_out('o.k.');
	}
	
	function _act_NEWROW(&$sqlo) {
		global $error;
		$FUNCNAME= __CLASS__.':_act_NEWROW';
		
		$lasty = $this->maxy;
		if (!$lasty) $lasty=0;
		$newy = $lasty+1;
		$xy=array(1,$newy);
		$params=array();
		$this->workModLib->updateCell($sqlo, $xy, $params);
		if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 1, 'error on insert of row: '.$i );
			return;
		}
		$this->workModLib->close($sqlo);
	}
	
	/**
	 * update cells
	 */
	function _act_UPDATE_PLAN( &$sqlo, $cells, $inputFlag ) {
		global $error;
		$FUNCNAME= __CLASS__.':_act_UPDATE_PLAN';
		
		if (!sizeof($this->absEntries)) {
			$error->set( $FUNCNAME, 1, 'missing planned object-types of abstract worklist' );
			return;
		}
		
		$errorGot=0;
		
		if ($inputFlag=='NAME') {
		
			// convert object NAMEs to IDs; only possible for one column
			
			$firstColx=0;
			$useTable   = NULL;
			$cellNames  = $cells; // copy to special NAME-array
			$cells      = NULL;   // clear ID-array
			
			
			foreach( $cellNames as $ypos=>$rowarr) {
				foreach( $rowarr as $xpos=>$objName) {
					if (!$firstColx) {
						$firstColx=$xpos;
						$plannedArr = $this->absEntries[$xpos];
						if ($plannedArr['typ']!='OBJ') {
							$error->set( $FUNCNAME, 6, 'only supported for TYPE=OBJ');
							return;
						}
						$useTable   = $plannedArr['t'];
						$impNameCol = importantNameGet2($useTable);
					}
					if ($firstColx!=$xpos) {
						$error->set( $FUNCNAME, 7, 'update by object-name only supported for ONE column!' );
						return;
					}
					
					$doUpdate=1;
					$objid=NULL;
					if (trim($objName)==NULL) {
						// is NULL
					} else {
						// search object
						list( $objcnt, $objid, $objname ) = fileC::objGetByName( $sqlo, $useTable, $impNameCol, $objName );
						if ($objcnt!=1) {
							echo '<b>ERROR:</b> object with name: "'.$objName.'" not found ONCE (count:'.$objcnt.').<br>';
							$errorGot=1;
							$doUpdate=0; // keep old data in database, if exists !
						}
					}
					
					if ($doUpdate) $cells[$ypos][$xpos]=$objid;
				}
				reset ($rowarr);
			}
			reset($cellNames);
			
			if (!sizeof($cells)) {
				$error->set( $FUNCNAME, 13, 'No objects were found!' );
				return;
			}
		}
		
		foreach( $cells as $ypos=>$rowarr) {
			foreach( $rowarr as $xpos=>$objid) {
				$plannedArr = $this->absEntries[$xpos];
				if ($plannedArr['typ']==NULL) {
					$error->set( $FUNCNAME, 3, 'missing planned TYPE for column '.$xpos );
					return;
				}
				
				$params = NULL;
				switch ($plannedArr['typ']) {
					case 'OBJ':
						$useTable   = $plannedArr['t'];
						if ($useTable==NULL) {
							$error->set( $FUNCNAME, 2, 'missing planned TABLE for column '.$xpos );
							return;
						}
						if ($objid!=NULL and !is_numeric($objid)) {
							echo '<b>ERROR:</b> object-identifier ('.$objid.') is not a number.<br>';
							$objid=NULL;
						}
						if (!$objid) $useTable=NULL; // unset table, if no object given
						
						$params = array('TABLE_NAME'=>$useTable, 'OBJID'=>$objid);
						break;
					case 'NOTES':
						$params = array('SH_NOTES'=>$objid);
						break;
				}
				
				$xy = array($xpos,$ypos);
				
				$this->workModLib->updateCell($sqlo, $xy, $params);
				if ($error->Got(READONLY))  {
					$error->set( $FUNCNAME, 4, 'error on update of cell: '.$xpos.','.$ypos );
					return;
				}
			}
			reset ($rowarr);
		}
		reset ($cells); 
		$this->workModLib->close($sqlo);
		$this->_html-> line_out('o.k.');
		
		if ($errorGot) {
			$error->set( $FUNCNAME, 11, 'Some objects could not be found!' );
			return;
		}
	}
	
	/**
	 * get all OBJID of one row
	 * @param $sqlo
	 * @param $row
	 * @return array $objIds
	 */
	function _getAllValsFromRow(&$sqlo, $row) {
		$sqlsel = 'XPOS, OBJID from WORKLIST_ENTRY where WORKLIST_ID='.$this->objid.' and YPOS='.$row.' order by XPOS';
		$sqlo->Quesel($sqlsel);
		while($sqlo->ReadRow()) {
			$objIds[$sqlo->RowData[0]] = $sqlo->RowData[1];
		}
		return $objIds;
	}
	
	// assign worklist to me
	function _act_ASSIGN_ToMe(&$sqlo) {
		global $error;
		$FUNCNAME= __CLASS__.':_act_ASSIGN_ToMe';
	
		$objid=$this->objid;
		$tablename='WORKLIST';
		$accLogLib    = new oAccLogC();
		$accLogLib->setObject( $sqlo, $tablename, $objid );
		$accLogState = oH_ALOG_ACT_subs::getH_ALOG_ACT_ID( $sqlo, $tablename, 'assigned' );
		if (!$accLogState) {
			$error->set( $FUNCNAME, 1, 'status "assigned" does not exist.' );
			return;
		}
		
		$lastStatusID = $accLogLib->getLastLog($sqlo);
		if ($lastStatusID == $accLogState) { // already the status, just check last user
			$accInfoArr = $accLogLib->getLastEntryOfAction($sqlo, $accLogState);
			if (is_array($accInfoArr)) {
				if ($accInfoArr['DB_USER_ID'] == $_SESSION['sec']['db_user_id']) {
					$error->set( $FUNCNAME, 2, 'worklist already assigned to you.' );
					return;
				}
			}
		}
		
		
		$argu=array("action"=>$accLogState, 'lastStatIgn'=>1); // ignore laste assigned status
		$accLogLib->addLogObj( $sqlo, 'WORKLIST', $objid, $argu );
		
		
	}
	
	private function _statusChange(&$sqlo, $statusName, $auditNotes) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$tablename='WORKLIST';
		
		$objid=$this->objid;
		$lockObj      = new gObjaccLock();
		$accLogLib    = new oAccLogC();
		$accLogLib->setObject( $sqlo, $tablename, $objid );
		$accLogState = oH_ALOG_ACT_subs::getH_ALOG_ACT_ID( $sqlo, $tablename, $statusName );
		if (!$accLogState) {
			$error->set( $FUNCNAME, 1, 'status "'.$statusName.'" does not exist.' );
			return;
		}
		$lastStatusID = $accLogLib->getLastLog($sqlo);
		if ($lastStatusID==$accLogState) {
			$error->set( $FUNCNAME, 2, 'Status ist schon gesetzt.' );
			return;
		}
		
		$lockObj->accessLockOne( $sqlo, $tablename, $objid);
		
		$argu=array( "action"=>$accLogState,'notes'=>$auditNotes );
		$accLogLib->addLogObj( $sqlo, $tablename, $objid, $argu );
		
		$accLogInfo    = $accLogLib->getInfo();
		if (is_array($accLogInfo)) {
			// show info
			$this->_html->line_out('Info: Status-Post-Action: '. implode('; ',$accLogInfo) );
		}
	}
	
	// finish
	function _act_Finish(&$sqlo) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;

		$auditNotes='NULL';
		$this->_statusChange($sqlo, 'finished', $auditNotes);
		
	}
	
	/**
	 * if experiment row: get last ROW of experiment
	 * @param  $sqlo
	 */
	function _act_Measured(&$sqlo) {
		
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
		$objid=$this->objid;
		$tablename='WORKLIST';
		
		$auditNotes=NULL;
		
		// get ROW of last experiment
		$absWorkLib = new oABS_WORKLIST_subs();
		$absWorkLib->setObj($this->absWorkId);
		$xcol_exp   = $absWorkLib->getColByTablename($sqlo, 'EXP');
		if($xcol_exp>0) {
			// get last exp
			$maxRowExp = $this->workAnaLib->getMaxRowOfCol($sqlo, $xcol_exp);
			$auditNotes= 'last EXP-row: '.$maxRowExp;
		}
		
		$this->_statusChange($sqlo, 'gemessen', $auditNotes);
		
	}
	
	
	
	/**
	 * connect objects to worklist
	 * @param $sqlo
	 * @return unknown_type
	 */
	function _act_CONNECT(&$sqlo) {
		global $error;
		$FUNCNAME= __CLASS__.':_act_CONNECT';
		
		$this->_html-> line_out('Info: Does not update existing values of the Auto-Collect-Cells!');
		
		$absWorkLib2 = new oABS_WORKLIST_subs2();
		$oneColLib   = new o_WORKLIST_autoColOneCol($this->workModLib, $this->objid, $this->_mod);
		
		// get AUTO_COLLECT
		
		$sqlsel = 'XPOS, TABLE_NAME, AUTO_COLLECT from ABS_WORKLIST_ENTRY '.
			'where ABS_WORKLIST_ID='.$this->absWorkId." and AUTO_COLLECT is not NULL and TYPE!='VIEW' order by XPOS";
		$sqlo->Quesel($sqlsel);
		
		$collInfoArr=NULL;
		while ( $sqlo->ReadRow() ) {
		    $xpos  = $sqlo->RowData[0];
		    $table = $sqlo->RowData[1];
		    $collectStr = $sqlo->RowData[2];
		    
		    $collInfoArr[$xpos] = array('table'=>$table, 'auto_collect'=>$collectStr);

		}
		
		if (!sizeof($collInfoArr)) {
			echo 'no auto_collect info found.<br>';
			return;
		}
		
		$loopcnt=0;
		foreach( $collInfoArr as $xpos=>$tmpInfoArr) {
			
			$collectInfoArr = $oneColLib->initCol($sqlo, $tmpInfoArr, $xpos);
			if (!is_array($collectInfoArr)) {
				continue; // no valid collect string
			}
		
			
			$ypos=1;
			while ($ypos<=$this->maxy) {
				echo 'row: ' .$ypos.': ';
				
				$rowVals = $this->_getAllValsFromRow($sqlo, $ypos);
				$answer = $absWorkLib2->createSqlAfter($collectInfoArr['sqlCond'], $collectInfoArr['cols'], $rowVals);
				$sqlAfter=$answer['sql'];
				$tmperror=$answer['error'];
				if ($tmperror!=NULL)  {
					echo '<b>Not possible:</b> '.$tmperror.'<br>';
					$ypos++;
					continue;
				}
				
				if ($sqlAfter!=NULL) {
					$oneColLib->collectCell($sqlo, $ypos, $sqlAfter);
					
				} else {
					echo 'not possible. ';
				}
				echo "<br />\n";
				$ypos++;
			}
			$loopcnt++;
		}
		reset ($collInfoArr); 
		
		$oneColLib->closeUpdate($sqlo);
		
		$stopProcess = $oneColLib->closeForm();
		// echo $loopcnt.' collects performed.<br>';
		return $stopProcess;
	}

	function startMain() {
		global $error;
		$FUNCNAME='startMain';
		
		$this->objid   = $_REQUEST['id'];
		$sqlo = &$this->_sqlo;
		$sqlo2 = &$this->_plug_regSql2();
		
		$parx   = $_REQUEST['parx'];
		$action = $_REQUEST['act'];
		
		if ($action==NULL) {
			$this->htmlFoot('ERROR','Parameter "action" missing');
		}
		
		$this->_html-> line_out('<b>Action:</b> '.$action);
		$this->_html-> line_out('');
		
		$this->workAnaLib = new oWORKLIST_subs($this->objid);
		$initAnswer       = $this->workAnaLib->init($sqlo);
		$this->absWorkId  = $initAnswer['absWorklistId'];
		$this->maxy = $initAnswer['maxy'];
		
		$this->absEntries = $this->workAnaLib->getPlannedAttribs($sqlo);
		
		$this->workModLib = new oWORKLIST_mod();
		$this->workModLib->setObj($sqlo, $this->objid);
		if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 1, 'error on object initialization' );
			return;
		}
		$stopProcess=0;
		
		switch ($action) {
			case 'new':
				if ($parx['rows']<=0) {
					$this->htmlFoot('ERROR','Parameter "rows" missing.');
				}
				$MAX_ROWS=500;
				if ($parx['rows']>$MAX_ROWS) {
					$this->htmlFoot('ERROR','Parameter "rows": max '.$MAX_ROWS.' new rows allowed.');
				}
				$this->_act_NEW($sqlo, $parx['rows']);
				break;
			case  'status.assignMe': 
				// assign worklist to me: no access changde, only status change
				$this->_act_ASSIGN_ToMe($sqlo);
				break;
			case  'status.finish':  // finish worklist
				$this->_act_Finish($sqlo);
				break;
			case  'status.measured':  
				$this->_act_Measured($sqlo);
				break;
			case 'update.plan':
				if (!sizeof($_REQUEST['cell'])) {
					$this->htmlFoot('ERROR','No cells given.');
				}
				$this->_act_UPDATE_PLAN($sqlo, $_REQUEST['cell'], $parx['input'] );
				break;
			case 'autocollect':
				$stopProcess = $this->_act_CONNECT($sqlo);
				break;
			case 'newrow':
				$this->_act_NEWROW($sqlo);
				break;
		}
		
		if ($error->got(READONLY))  {
			return;
		}
		
		if ($stopProcess) return;
		
		$url='edit.tmpl.php?t=WORKLIST&id='.$this->objid;
		js__location_replace($url, "object" );
	}
	
}
