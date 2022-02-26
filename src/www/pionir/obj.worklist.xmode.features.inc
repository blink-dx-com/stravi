<?php
/**
 * worklist extension 
 * used by edit.tmpl.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq UREQ:0001159: o.WORKLIST > eine Worklist anzeigen/bearbeiten 
 */
require_once ("f.objview.inc");	
require_once ('func_form.inc');
require_once ('o.WORKLIST.subs.inc');
require_once ("o.ABS_WORKLIST.subs2.inc");
require_once ("visufuncs.inc");
require_once 'o.WORKLIST.content.inc';



/**
 * GUI sub class for one worklist
 *
 */
class oWORKLIST_guiSub {
	
	var $assignArr; /**
		@var array(
			'isme'    => 0,1, 
			'user'    => $userName, 
			'statusID'=>    $lastStatusID
			'statusName' => name of $lastStatusID
			);
	*/
	var $viewmode; // 'edit', 'view'
	var $absEntries; /** abstract worklist entries; array of [$xpos]=array(key=>val)
	  array('t'=>$table, 'typ'=>['OBJ','VIEW',..], 'note'=>, 'collect'=> )
	  
	*/
	
	var $_debug;
	var $ABS_WORKLIST_ID;

	
function __construct($id, $viewmode, $o_rights) {
	$this->id=$id;
	$this->editmode=$viewmode;
	$this->o_rights=$o_rights;
	$this->_debug = $_SESSION["userGlob"]["g.debugLevel"];
	$this->colOrder = NULL;
	$this->ABS_WORKLIST_ID = 0;
}
function init(&$sqlo) {
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	$this->absEntries = NULL;

	$this->objLinkLib = new fObjViewC();
	$this->workAnaLib = new oWORKLIST_subs($this->id);
	
	$this->absWorkLib = new oABS_WORKLIST_subs2();
	$initAnswer = $this->workAnaLib->init($sqlo);
	$this->ABS_WORKLIST_ID =  $initAnswer['absWorklistId'];
	$this->absEntries = $this->workAnaLib->getPlannedAttribs($sqlo);
	
	$this->maxy   = $initAnswer['maxy'];
	$this->maxx   = $initAnswer['maxx'];
	$this->colOrder =  $initAnswer['colOrder'];
	
	if (!sizeof($this->colOrder)) {
		$error->set( $FUNCNAME, 1, 'no columns defined in abs_worklist.' );
		return;
	}
	
	// analyse $this->absEntries
	if (is_array($this->absEntries)) {
		foreach( $this->absEntries as $xpos=>$valarr) {
			if ($valarr['opt']!=NULL) {
				$this->absEntries[$xpos]['optarr']= $this->absWorkLib->option2arr($valarr['opt']);
			}
		}
		reset ($this->absEntries); 
	}
	if ( $_SESSION["userGlob"]["g.debugLevel"]>0 ) glob_printr( $this->absEntries, "absEntries info" );
	
	$this->assignArr=$this->_AssignInfo($sqlo);
	
	

	
}



/**
 * analyse: $this->editmode, $this->assignArr['isme']
 * @return int 0,1
 */
function getEditPossible() {
	if ($this->editmode=='view')    return 0;
	if (!$this->o_rights['entail']) return 0;
	if ($this->assignArr['isme']) $editPossible=1;
	else $editPossible=0;
	
	if (glob_isAdmin()) $editPossible=1;
	return $editPossible;
}

function _AssignInfo(&$sqlo) {
	$objid=$this->id;
	$tablename='WORKLIST';
	
	$isAssignedToMe=0;
	$userName=NULL;
	$accLogLib    = new oAccLogC();
	$accLogLib->setObject( $sqlo, $tablename, $objid );
	$accLogState = oH_ALOG_ACT_subs::getH_ALOG_ACT_ID( $sqlo, $tablename, 'assigned' );
	if (!$accLogState) {
		return;
	}
	
	$lastStatusID   = $accLogLib->getLastLog($sqlo);
	if ($lastStatusID) {
	    $lastStatusName = oH_ALOG_ACT_subs::statusID2Name( $sqlo, $lastStatusID );
	} else $lastStatusName = NULL;
	$accInfoArr   = $accLogLib->getLastEntryOfAction($sqlo, $accLogState);
	if (is_array($accInfoArr)) {
		if ($accInfoArr['DB_USER_ID'] == $_SESSION['sec']['db_user_id']) {
			$isAssignedToMe=1;
		}
		if ($accInfoArr['DB_USER_ID']) $userName = obj_nice_name ( $sqlo, 'DB_USER', $accInfoArr['DB_USER_ID'] ) ;
	}
	return array('isme'=>$isAssignedToMe, 'user'=>$userName, 'statusID'=>$lastStatusID, 'statusName'=>$lastStatusName);
}

function showNaviButtons(&$sqlo) {

	$current_status = $this->assignArr['statusName'];
	$entailOk  = $this->o_rights['entail'];
	$canReject = 0;
	if ( role_admin_check ( $sqlo, 'WORKLIST') ) $entailOk = 1; // allow, if set
	if ( role_check_f_pure ( $sqlo, 'o.WORKLIST.acclog', $_SESSION['sec']['db_user_id'] )=='execute') {
		$canReject = 1;
	}
	$finalStatusReached = 0;
	if ( $current_status == 'rejected' or $current_status == 'finished') {
		$finalStatusReached = 1;
	}
	
	if ($this->editmode=='edit' and $entailOk and !$finalStatusReached) {
		$this->_formStart('editform3', '');
		echo "<input type=button name='dummy' value='Ich &uuml;bernehme' ".
				"onclick=\"document.editform3.act.value='status.assignMe'; document.editform3.submit();\"> \n";
		
		// @swreq UREQ:0001134: o.WORKLIST > "gemessen" Status 
		if ( $current_status != 'gemessen' ) {
			echo "<input type=button name='dummy' value='Gemessen' ".
				"onclick=\"document.editform3.act.value='status.measured'; document.editform3.submit();\"> \n";
		} else {
			echo '[Gemessen] ';
		}
		if ( $current_status != 'finished' ) {
			echo "<input type=button name='dummy' value='Auftrag fertig' ".
				"onclick=\"document.editform3.act.value='status.finish'; document.editform3.submit();\"> \n";
		} else {
			echo '[Auftrag fertig] ';
		}
		
		
		
		echo '</form>'."\n";
	}
	
	if ($this->editmode=='edit' and $current_status != 'rejected') {
		if ($canReject) {
			echo "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<input type=button name='dummy' value='Reject' ".
					"onclick=\"location.href='glob.obj.acclock.php?t=WORKLIST&id=".$this->id."&parx[statName]=rejected'\"> \n";
		} else {
			echo '&nbsp;&nbsp;<input type=button value="Reject" disabled title="only QC group can reject"> '."\n";
		}
	}
	
	
	echo '<b>Bearbeiter:</b> '.$this->assignArr['user'];
	
	$motherobj = $this->workAnaLib->getMotherObj($sqlo);
	if (is_array($motherobj)) {
		$objLinkLib = new fObjViewC();
		$motherhtml = $objLinkLib->bo_display( $sqlo, $motherobj['t'], $motherobj['id'] );
		echo '&nbsp;&nbsp; <b>Mother object:</b> '.$motherhtml."<br />\n";
	}
	
	echo "<br />\n";
	if ( $this->editmode=='edit' and !$this->assignArr['isme'] and !$finalStatusReached) {
		htmlInfoBox( "Bearbeitung gesperrt", "Du musst die worklist &uuml;bernehmen, um sie zu bearbeiten.", "", "WARN" );
		echo "<br />\n";
	}
	
}




/**
 * start HTML-form for p.php?mod=DEF/o.WORKLIST.guimod
 * @param string $formname
 * @param string $act
 * @param string $formInputOpt [ID], NAME
 * @return -
 */
function _formStart($formname, $act, $formInputOpt=NULL) {
	
	
	echo '<form style="display:inline;" method="post" '.
		 ' name="'.$formname.'"  action="p.php?mod=DEF/o.WORKLIST.guimod" >'."\n";
	echo '<input type=hidden name="id" value="'.$this->id.'">'."\n";
	echo '<input type=hidden name="act" value="'.$act.'">'."\n";
	if ($formInputOpt!=NULL) echo '<input type=hidden name="parx[input]" value="'.$formInputOpt.'">'."\n";
}

/**
 * table header
 * @param  string $mode edit,view
 * @param string $currentEditCol id of edit-column
 */
function _tableHeader($mode=NULL, $currentEditCol=NULL) {
	
	$objViewLib = new fObjViewC();
	$editPossible = $this->getEditPossible();
	if ($currentEditCol) $editIsActive=1;
	else $editIsActive=0;
	
	// echo "<table border=0 cellspacing=1 cellpadding=1 bgcolor=#EFEFEF ><tr><td>"; // light blue bgcolor
	echo "<table border=0 cellspacing=1 cellpadding=2 bgcolor=#CFCFFF>\n";
	
	
	echo "<tr>";
	echo "<td>#</td>";
	
	reset ($this->colOrder);
	foreach( $this->colOrder as $dummy=>$xpos) {
		$thisEntry = $this->absEntries[$xpos];
		$entryType = $thisEntry['typ'];
		$iconSrc   = NULL;
		$title     = NULL;
		$tablename = $thisEntry['t'];
		$title     = $thisEntry['optarr']['title'];
		
		if ($tablename!=NULL) {
			$iconSrc = '<img src="'.$objViewLib->_getIcon($tablename).'">';
		}
		if ($title==NULL) {
			if ($tablename!=NULL) {
				$title   = tablename_nice2($tablename);
			}
		}
		
		echo "<td>";
		echo $iconSrc;
		if($editPossible) { 
			if (!$editIsActive) {
				if ( $entryType!='VIEW' and $entryType!='PLUGIN' ) { // not a view ???
					echo '&nbsp;[<a href="edit.tmpl.php?t=WORKLIST&id='.
						$this->id.'&tabArgu[act]=edit&tabArgu[col]='.$xpos.'">Edit</a>]'.
						'&nbsp;[<a href="p.php?mod=DEF/o.WORKLIST.guimod2&id='.
						$this->id.'&col='.$xpos.'" title="Import">Imp</a>]';
				}
			} else {
				if ($currentEditCol==$xpos) {
					echo '&nbsp;[<a href="javascript:fillDown( '.$currentEditCol.' )">Fill down</a>]';
					echo '&nbsp;[<a href="javascript:clearCol( '.$currentEditCol.' )">Clear</a>]';
				}
			}
		}
		if ($thisEntry['typ']=='OBJ' and $tablename!=NULL and !$editIsActive) {
			echo '&nbsp;[<a href="p.php?mod=DEF/o.WORKLIST.guiana&id='.
					$this->id.'&act=select&parx[colx]='.$xpos.'">List</a>]';
		}
		echo "<br /><b>";
		echo $title;
		echo "</b>";
		
		echo "</td>";
		$xpos++;
	}
	reset ($this->colOrder);
	echo '</tr>'."\n";
	
}

function _tableClose()  {
	echo "</table>\n";
	// echo "</td></tr></table>\n";
	
}

/**
 * 
 * @param  $dataArray
 * @param  $ypos
 * @param  $opt 
 * 		'submitRow' : 0,1
 */
function _tableRow($dataArray, $ypos, $opt=NULL) {
	if ( $opt['submitRow'] )  $bgcolor='#D0D0FF';
	else $bgcolor='#EFEFEF';
	
	echo '<tr bgcolor='.$bgcolor.'>';
	echo "<td>";
	if ( !$opt['submitRow'] ) echo "<b>".$ypos.".</b>";
	echo "</td>";
	
	reset ($this->colOrder);
	foreach( $this->colOrder as $dummy=>$xpos) {
		echo "<td>";
		echo $dataArray[$xpos];
		echo "</td>";
	}
	reset ($this->colOrder);
	echo "</tr>\n";
}
	
function showTable(&$sqlo, $id) {
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	
	$this->_tableHeader($this->editmode);
	
	$ypos=1;
	
	$oneRowLib = new oWORKLIST_oneRow($sqlo, $this->absEntries, $id, $this->ABS_WORKLIST_ID);
	if ($error->Got(READONLY))  {
		$error->set( $FUNCNAME, 1, 'Error on Init Table.' );
		return;
	}
	
	while ($ypos<=$this->maxy) {
		
		$sqlsel = "* from WORKLIST_ENTRY where WORKLIST_ID=".$id." and YPOS=".$ypos." order by XPOS";
		$sqlo->Quesel($sqlsel);
		
		$rowArr = NULL; // data of row
		while ( $sqlo->ReadArray() ) {
		    $xpos  = $sqlo->RowData['XPOS'];
		    $table = $sqlo->RowData['TABLE_NAME'];
		    $objid = $sqlo->RowData['OBJID'];
		    $val   = $sqlo->RowData['SH_NOTES'];
		    $rowArr[$xpos] = array( $table, $objid, $val );
		}
		
		$oneRowLib->setRowData($rowArr);
		
		
		$dataArray=NULL;
		reset ($this->colOrder);
		foreach( $this->colOrder as $dummy=>$xpos) {
			$objHtmlText = $oneRowLib->getColView($sqlo, $xpos);
			$dataArray[$xpos]=$objHtmlText;
			$xpos++;
		}
		reset ($this->colOrder);
		$this->_tableRow($dataArray, $ypos);
		
		$ypos++;
	}
	$this->_tableClose();
	
	$collected_errors = $oneRowLib->getErrors();
	
	if (sizeof($collected_errors)) {
		echo "<br>\n";
		$tabobj  = new visufuncs();
		$headx   = array ("Errors");
		$headOpt = array('title'=>'Collected ERRORs');
		$tabobj->table_head($headx,   $headOpt);
		foreach($collected_errors as $rowdata) {
			$tabobj->table_row ( array($rowdata) );
		}
		$tabobj->table_close();
	}
	
}

/**
 * get objects from clipboard
 * @global $this->clipArr [OUTPUT]
 */
function _getClipBoardInfo(&$sqlo) {

	require_once ("f.clipboard.inc");
	$this->clipArr = NULL; // array[table] = array(ID=>NAME), object list from clipboard => for select box
	$clipobj  = new clipboardC();
	
	$colOrder=$this->colOrder;
	reset ($colOrder);
	foreach( $colOrder as $dummy=>$xpos) {
		$useTable    = $this->absEntries[$xpos]['t'];
		if ($useTable!=NULL) {
			$substArrTmp = $clipobj->getAllObjects( $useTable );
			$objectArr   = NULL; 
			
			if (sizeof($substArrTmp)) {
				$this->clipArr[$useTable]=array();
				foreach( $substArrTmp as $index=>$objid) {
					if ($objid>0) { // only good IDs
						$objname = obj_nice_name ( $sqlo, $useTable, $objid );
						$this->clipArr[$useTable][$objid]=$objname;
					}
				}
				reset ($substArrTmp); 
			}
		}
		
	}
	reset ($colOrder);
}

/**
 * get existing object-IDs of one column
 * @param  $sqlo
 * @param  $xpos
 * @return array
 */
function _getExistingObj(&$sqlo, $xpos) {
	$sqlsel = "OBJID from WORKLIST_ENTRY where WORKLIST_ID=".$this->id." and XPOS=".$xpos." and OBJID>0 order by YPOS";
	$sqlo->Quesel($sqlsel);
	
	$objArrT = NULL;
	while ( $sqlo->ReadArray() ) {
	    $objid = $sqlo->RowData['OBJID'];
	    $objArrT[] = $objid;
	}
	
	$useTable = $this->absEntries[$xpos]['t'];
	$objArr   = array();
	if (sizeof($objArrT)) {
		foreach( $objArrT as $dummy=>$objid) {
			if ($objid>0) {
				$objname = obj_nice_name ( $sqlo, $useTable, $objid );
				$objArr[$objid]=$objname;
			}
		}
		reset ($objArrT); 
	}
	return $objArr;
}

function _newEntries() {
	require_once ('func_form.inc');
	
	$initarr   = NULL;
	$initarr["action"]      = 'p.php?mod=DEF/o.WORKLIST.guimod';
	$initarr["title"]       = "Create new entries";
	$initarr["submittitle"] = "Create";
	$initarr["tabwidth"]    = "AUTO";

	$hiddenarr = NULL;
	$hiddenarr["act"]     = 'new';
	$hiddenarr["id"]     = $this->id;

	$formobj = new formc($initarr, $hiddenarr, 0);

	$fieldx = array ( 
		"title" => "Number of rows", 
		"name"  => "rows",
		"object"=> "text",
		"val"   => $parx["rows"], 
		"notes" => "e.g. 10"
		 );
	$formobj->fieldOut( $fieldx );

	$formobj->close( TRUE );
}

function _arrayMergeKeys(&$dest, $src) {
	reset ($src);
	
	$firstAppend=1;
	$initlen=sizeof($src);
	if (!$initlen) $firstAppend=0; // no data before 
	
	foreach( $src as $key=>$val) {
		if (!isset($dest[$key])) {
			if ($firstAppend) $dest[0]=' --- ';
			$dest[$key]=$val;
			$firstAppend=0;
		}
	}
	reset ($src); 
}

/**
 * get SQL-command for SELECT from ABS_WORKLIST_ENTRY
 * @param  $sqlo
 * @param  $absEntriyCol
 * @return -
 * @global $this->clipArr
 */
function _editManagePreselect(&$sqlo, $absEntriyCol, $useTable) {
	global $error;
	$FUNCNAME= __CLASS__.':_editManagePreselect';
	
	$answer = $this->absWorkLib->anaObjPreselectSql($absEntriyCol['collect']);
	$tablename = $absEntriyCol['t'];
	if ( $this->_debug>0 ) {
		debugOut('Collect-Str fo '.$tablename.': '.$answer['sqlCond'], '_editManagePreselect');
	}
	if ($tablename==NULL) return;
	if ($answer['sqlCond']==NULL) return;
	
	$sqlsel=$answer['sqlCond'];
	$sqlo->Quesel($sqlsel);
	if ($error->Got(READONLY))  {
		$error->set( $FUNCNAME, 1, 'SQL command not correct' );
		return;
	}
	
	$MAXCOLLECT=50; // maximum !
	$selectObjArr=NULL;
	$cnt=0;
	while ( $sqlo->ReadRow() ) {
		if ($cnt>$MAXCOLLECT) break;
	    $selectObjArr[]  = $sqlo->RowData[0];
	    $cnt++;
	}
	
	if (sizeof($selectObjArr)) {
		$objNamesNice=NULL;
		foreach( $selectObjArr as $dummy=>$objid) {
			if ($objid>0) {
				$objname =  obj_nice_name ( $sqlo, $tablename, $objid);
				$objNamesNice[$objid]=$objname;
			}
		}
		reset ($selectObjArr);
		$this->_arrayMergeKeys( $this->clipArr[$useTable], $objNamesNice );
	} else {
		htmlInfoBox( "Warning", "No objects found during preseletion. (See abstract worklist definition of column)", "", "WARN" );
		echo '<br>'."\n";
	}
		
	
}

/**
 * prepare one column for edit, get all possible objects
 * @param  $sqlo
 * @param int $colid
 * @return -
 */
function _editPrepCol(&$sqlo, $colid) {
	global $error;
	$FUNCNAME= __CLASS__.':_editPrepCol';
	
	$absEntriyCol = $this->absEntries[$colid];
	$useTable     = $this->absEntries[$colid]['t'];

	if ( $colid<=0 ) return;
		
	if ( $absEntriyCol['typ']=='OBJ') {
		$existObjs = $this->_getExistingObj($sqlo, $colid);
		if (is_array($this->clipArr[$useTable]))
			$this->_arrayMergeKeys( $this->clipArr[$useTable], $existObjs );
		else $this->clipArr[$useTable] = $existObjs;
	}
	
	if ( $absEntriyCol['typ']=='OBJ' and $this->absWorkLib->has_preselect_cond($absEntriyCol['collect']) ) {
		$this->_editManagePreselect($sqlo, $absEntriyCol, $useTable);
	}
	
}

/**
 * add javascript to HTML-page
 */
function _addJavascript() {
	echo '<script language="JavaScript">'."\n";
	js_replaceLF("\t");
	?>
	function fillDown( colx ) {
		  
		iniVal = document.editform2.elements["cell[1]["+colx+"]"].value;
		if (iniVal=="") {
			alert("Du musst in der ertsen Zeile einen Initial-Wert eingeben.");
			return;
		}
		
		row=2;
		while (1) {
			elemName = "cell["+row+"]["+colx+"]";
			
			if ( document.getElementsByName(elemName)[0] ) {
				answer=1;
			} else answer=0;
			if ( !answer ) break;
			document.editform2.elements[elemName].value = iniVal; 
			row=row+1;
		}
	}
	
	<?php 
	// clear column
	?>
	function clearCol( colx ) { 
		iniVal = "";	
		row=1;
		while (1) {
			elemName = "cell["+row+"]["+colx+"]";
			
			if ( document.getElementsByName(elemName)[0] ) {
				answer=1;
			} else answer=0;
			if ( !answer ) break;
			document.editform2.elements[elemName].value = iniVal; 
			row=row+1;
		}
	}
	
	<?php // give an alternative value for a <select>  ?>
	
	function ed2_valueInSel( fieldID ) { 
		valid = document.editform2.elements[fieldID].options.selectedIndex ;
		valu  = document.editform2.elements[fieldID].options[valid].text;
		newval= prompt ( 'Value or [NULL]', valu );
		nullval = 0;
	
		if ( newval == "[NULL]" )  {
			nullval = 1;
			newval  = "";
		}
		if ( (newval != null) || nullval ) {
			document.editform2.elements[fieldID].options[0].text    = newval;
			document.editform2.elements[fieldID].options[0].value   = newval;
			document.editform2.elements[fieldID].options[0].selected= true;
		}
	}
	<?php
	
	
	
	echo '</script>'."\n";
}




/**
 * show edit form
 * @param object $sqlo
 * @param int $id
 * @param array $pageArgu
 * 	'act' = 'edit'
 *  'col' = ID of column
 * @return NULL
 */
function editTable($sqlo, $id, $pageArgu) {
	echo "<ul>\n";
	
	$currentEditCol = $pageArgu['col']; // currently edited column
	
	if (!$this->maxy) { // no entries
		$this->_newEntries();
		return;
	} else {
		echo '[<a href="p.php?mod=DEF/o.WORKLIST.guimod&id='.$this->id.'&act=autocollect">Auto-Collect results</a>]'."<br>\n";
	}
	
	$this->_getClipBoardInfo($sqlo);
	
	$useAbstractArr  = $this->absEntries[$pageArgu['col']];
	$useTable        = $useAbstractArr['t'];
	
	$this->_editPrepCol($sqlo, $pageArgu['col']);
	
	if ($useTable!=NULL and $useAbstractArr['optarr']['input']=='NAME') {
		$formInputOpt='NAME';
	} else $formInputOpt=NULL;
	
	
	$oneRowLib = new oWORKLIST_oneRow($sqlo, $this->absEntries, $id, $this->ABS_WORKLIST_ID);
	$oneRowLib->initEdit($this->clipArr, $pageArgu['col']);
	
	$this->_addJavascript();
	
	$this->_formStart('editform2', 'update.plan', $formInputOpt);
	
	
	$this->_tableHeader($this->editmode, $currentEditCol);
	
	$ypos=1;
	while ($ypos<=$this->maxy) {
		
		$sqlsel = "* from WORKLIST_ENTRY where WORKLIST_ID=".$id." and YPOS=".$ypos." order by XPOS";
		$sqlo->Quesel($sqlsel);
		
		$rowArr = NULL;
		while ( $sqlo->ReadArray() ) {
		    $xpos  = $sqlo->RowData['XPOS'];
		    $table = $sqlo->RowData['TABLE_NAME'];
		    $objid = $sqlo->RowData['OBJID'];
		    $val   = $sqlo->RowData['SH_NOTES'];
		    $rowArr[$xpos] = array( $table, $objid, $val );
		}
		
		$oneRowLib->setRowData($rowArr);

		
		$dataArray=NULL;
		reset ($this->colOrder);
		foreach( $this->colOrder as $dummy=>$xpos) {
			$htmlText = $oneRowLib->getColEdit($sqlo, $xpos, $ypos);
			$dataArray[$xpos]=$htmlText;
			$xpos++;
		}
		reset ($this->colOrder);
		$this->_tableRow($dataArray, $ypos);
		$ypos++;
	}
	
	if ( $pageArgu['act']=='edit') {
		
		$dataArray=NULL;
		reset ($this->colOrder);
		foreach( $this->colOrder as $dummy=>$xpos) {
			if ($pageArgu['col']==$xpos) $tmptext='<input type=submit value="Update">';
			else $tmptext='';
			$dataArray[$xpos]=$tmptext;
		}
		reset ($this->colOrder);
		$rowopt=array('submitRow'=>1);
		$this->_tableRow($dataArray, $ypos, $rowopt);
	}
	echo '</form>'."\n";
	$this->_tableClose();
	
	if ( $pageArgu['act']!='edit') {
		echo '<br />'."\n";
		$this->_formStart('editform4', 'newrow');
		echo ' <input type=submit value="Neue Zeile">'."\n";
		echo '</form>'."\n";
	}
	
	
}

}


class obj_edit_ext extends gObj_edit_feat_ABS {
    
    function rightWindow( &$sqlo, &$sqlo2) {
        $id = $this->id;
        $o_rights = &$this->o_rights;
        $viewmode = $this->viewmode;
       
    	global $error;
    	echo "<ul>\n";
    	$tabArgu =  $_REQUEST['tabArgu'];
    	
    	$workLib = new oWORKLIST_guiSub($id, $viewmode, $o_rights);
    	$workLib->init($sqlo, $id);
    	if ($error->printAll()) {
    		return;
    	}
    	$workLib->showNaviButtons($sqlo);
    	
    	$editPossible = $workLib->getEditPossible();
    	
    	if ($editPossible) {
    		$workLib->editTable($sqlo, $id, $tabArgu );
    	} else {
    		$workLib->showTable($sqlo, $id);
    	}
    	
    	$error->printAll();
    }

}