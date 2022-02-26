<?php
require_once ('date_funcs.inc');
require_once ('lev1/f.exportDataFile.inc');
require_once ('o.PROJ.subs.inc');
require_once ('o.PROJ.paths.inc');
require_once ('f.prefvals.inc');
require_once ("f.objview.inc");
require_once ("o.REA_LOG.subs.inc");
require_once ("o.CHIP_READER.logs.inc");
require_once 'object.info.inc';

/**
 * - search in name and notes of service logs
 *
 * - to search in project
 *   - o.PROJ.subs.inc : cProjSubs::getTableSQL()
 *   - o.PROJ.paths.inc : oPROJpathC::showPathSlim()
 *
 * @package o.CHIP_READER.logSearch.inc
 * @author  Steffen Kube
 * @swreq UREQ:0000855, FS-LIM08-04
 * @param $_REQUEST['go']
 * @param $_REQUEST['parx'] array of
 *    'searchtext' : search text
 *    'key'		   : key
 *
 * @param $_REQUEST['projid'] project ID, where analysed CHIP_READERs are stored
 * @param $_REQUEST['action']
 *   ['project']  - select devices by project
 *    'select' - select devices by list selection
 * @version $Header: trunk/src/plugin/o.CHIP_READER.logSearch.inc 59 2018-11-21 09:04:09Z $
 */
class o_CHIP_READER_logsearch_XPL extends gPlugin {
	
	var $_projid; // search project ID
	
	
	function register() {
		$tablename	= 'REA_LOG';
		$title		= 'search in service logs';
		
		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title']    = $title.' (FS-LIM08-04)';
		$this->infoarr['title_sh'] = 'service log search';
		$this->infoarr['form_type']= 'tool'; 
		$this->infoarr["obj_name"] = $tablename;
		$this->infoarr['design']   = 'norm';
		$this->infoarr['help_url'] = 'o.CHIP_READER.logSearch.html';
		$this->infoarr["locrow"]   = array( array('home.php', 'home') );
		
		$this->infoarr2 = array( 
			'accCheck' => array('tab'=>array('read'), 'obj'=>array() ),
			'progressBar' =>1
			);
				
	}
	/**
	 * 
	 * 
	 * @param int $viewmode 1 - view, 0 - edit
	 * @return unknown_type
	 */
	
	function form0(&$sqlo) {
		
		$parx = $this->parx;
		
		$prefValLib = new fPrefValsGet();	
		$prefs = $prefValLib->getValInitList( $sqlo, "REA_LOG", "KEY" );
		$prefarr = NULL;
		foreach( $prefs as $dummy=>$valarr) {
			$prefarr[$valarr[0]] = $valarr[1];
		}
		reset ($prefs);
		
		$initarr["title"]       = '&nbsp;';
		$initarr["submittitle"] = "search";
		$initarr["tabwidth"]    = "AUTO";
	
		$hiddenarr = NULL;
		$projid = $this->_projid;
		$hiddenarr["projid"] = $projid;
		$hiddenarr["action"] = $this->action;
	
		$formobj = $this->_html->formInit($initarr, $hiddenarr);
		
		$fieldy=array();
		
		$fieldy[] = array ( 
			"title" => "Text", 
			"name"  => "searchtext",
			"object"=> "text",
			"val"   => $parx["searchtext"],
			"notes" => 'search in "short description" and "notes" of service logs'
			 );
		
		$fieldy[] = array (
				"title" => "Key",
				"name"  => "key",
				"object"=> "select",
				"inits" => $prefarr,
				"val"   => $parx["key"],
				"notes" => 'optional select key'
		);
			 
			 
		$formobj->fieldArrOut( $fieldy );
		
		
		$formobj->close( TRUE, $closeArr );
		
		
	}
	
	function help() {
		htmlInfoBox( "Short help", "", "open", "HELP" );
		?>
		<ul>
		<li>This modul searches for text in the service log of selected devices. </li>
		<li>Devices can be selected by "list view" or all devices in a project</li>
		<li>Searches for name or notes in the device log. </li>
		<li>Just give parts of a name or notes. </li>
		<li>The search is not case-sensitive, so you can choose to insert small or capital letters. </li>
	
		</ul>
		<?
		htmlInfoBox( "", "", "close" );
		
		echo "<br />\n";
	}
	
	/**
	 * search now!
	 * input: $this->sqlDeviceSelect
	 * @param unknown $sqlo
	 * @param unknown $sqlo2
	 */
	function searchNow(&$sqlo, &$sqlo2) {
		global $error;
		
		$FUNCNAME  = 'searchNow';
		$MAX_SHOW  = 1000; // maximum of outputs
		$MAX_NOTES_CHARS  = 150; // maximum of outputs
		$tablename = 'REA_LOG';
		
		$parx   = $this->parx;
		$searchtext = $parx['searchtext'];
		$searchkey  = $parx['key'];
		
		
		$status_def_arr = oREA_LOG_subs::ACCEPT_pref;
		
		
		
		$sqlo = &$this->_sqlo;
		$objInfoLib = new objInfoC();
		
		// onject info, get object data
		require_once ('object.info.inc');
		
		$sqladdKey   = NULL;
		if ($searchkey!=NULL) {
			$sqladdKey = " AND (KEY like ".$sqlo->addQuotes($searchkey).")";
		}
		
		
		//$paramsRaw = $objInfoLib->getParamsRaw ( $sqlo, "REA_LOG", $this->objid, $rawopt);
		$sqlBase = "from REA_LOG ".
			" where (LOWER(NAME) LIKE LOWER('%".$searchtext."%') OR LOWER(NOTES) LIKE LOWER('%".$searchtext."%'))".
			" AND CHIP_READER_ID in (".$this->sqlDeviceSelect.") ".$sqladdKey;
		
		$sqlsel = "count(1) ".$sqlBase;
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$co = $sqlo->RowData[0];
		
		echo "<b>" . $co . "</b> matches found:<br><br>";
		
	
		// show a table
		$columns  = array('CHIP_READER_ID', 'NAME', 'XDATE', 'XDATE2', 'DB_USER_ID', 'KEY', 'NOTES', 'ACCEPT', 'POS');
		$headOpt=array("title"=>'results');
		$headx  = array('#');
		
		foreach( $columns as $dummy=>$col) {
			$ftab_prim_name=NULL;
			$ftab_imp_name =NULL;
			$fk_table = fk_check2($col, $tablename, $ftab_prim_name, $ftab_imp_name );
			if ($fk_table!=NULL) {
				$colNice  = tablename_nice2($fk_table); 
			} else {
				$colNice  = columnname_nice2('REA_LOG', $col);
			}
			if ($col=='XDATE') $colNice .= ' *';
			$headx[] = $colNice;
		}
		reset ($columns); 
		
		
		$this->_html->_table3_init($headx, $headOpt);
			
		
		$sqlsel = "* ".$sqlBase." ORDER BY XDATE DESC";
		$sqlo->Quesel($sqlsel);
		
		$entryCnt=0;
		while ( $sqlo->ReadArray() ) {
			
			
			$readerid = $sqlo->RowData['CHIP_READER_ID'];
			$POS      = $sqlo->RowData['POS'];
			$XDATE    = $sqlo->RowData['XDATE'];
			
			$readerNice = obj_nice_name ( $sqlo2, 'CHIP_READER', $readerid );
			$hyperlink  = '<a href="edit.tmpl.php?t=CHIP_READER&id=' . $readerid . '">' . $readerNice . '</a>';
			$pos_entry_link = '<a href="edit.tmpl.php?t=REA_LOG&id='     . $readerid . '&&primasid[1]='.$POS.'">' . ($entryCnt+1) . ' ==></a>';
			$ACCEPT_nice = $status_def_arr[$sqlo->RowData['ACCEPT']];
			$user_nice = obj_nice_name ( $sqlo2, 'DB_USER', $sqlo->RowData['DB_USER_ID'] );
			
			if (strlen($sqlo->RowData['NOTES'])>$MAX_NOTES_CHARS) {
				$notes = substr($sqlo->RowData['NOTES'], 0, $MAX_NOTES_CHARS) . "...";
			} else {
				$notes = $sqlo->RowData['NOTES'];
			}
			$notes = htmlspecialchars($notes);
    		$dataArr=array( 
    				$pos_entry_link ,
    				$hyperlink, 
    				$sqlo->RowData['NAME'], 
    				$XDATE, 
    				$sqlo->RowData['XDATE2'],
    		        $user_nice, 
    				$sqlo->RowData['KEY'], 
    				$notes, 
    				$ACCEPT_nice,
    				$POS
    				);
			$this->_html->_table3_row($dataArr);
			if ($entryCnt>$MAX_SHOW)  {
				$error->set( $FUNCNAME, 4, 'More then '.$MAX_SHOW.' results!' );
				return;
			}	
			$entryCnt++;
		}
		
		
		$this->_html->_table3_close();
		
		if (!$entryCnt)  {
			$error->set( $FUNCNAME, 3, 'No entries found!' );
			return;
		}
		
	}
	
	private function project_Select(&$sqlo, $projid) {

		$this->sqlDeviceSelect = cProjSubs::getTableSQL ($projid, 'CHIP_READER'); // sqlStatement
		
		$this->numDevices = cProjSubs::getTableCnt ( $sqlo, $projid, 'CHIP_READER' );

		$projPathLib = new oPROJpathC();
		$htmlText = $projPathLib->showPathSlim($sqlo, $projid, 'edit.tmpl.php?t=PROJ&id=');
		echo $htmlText. '<br />'."\n";
	}
	
	/**
	 * check, if a selection is active ...
	 * @param $sql
	 * @param $option : "checkNumMax"
	 */
	private function list_select( &$sqlo, $option=NULL ) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
		require_once ("f.visu_list.inc");
	
		$this->obj_cnt = 0;
	
		$tablename   = 'CHIP_READER';
		$listVisuObj = new visu_listC();
	
		$copt=NULL;
		$copt["doCount"] = 1;
		if ($option["checkNumMax"]>0) $copt["checkNumMax"] = $option["checkNumMax"];
	
		$checkResult = $listVisuObj->checkSelection( $sqlo, $tablename, $copt );
		$stopFlag   = $checkResult[0];
		$stopReason = $checkResult[1];
		$this->numDevices = $checkResult[2];
	
		if ( $stopFlag<0 ) {
			$error->set( $FUNCNAME, 1, $stopReason.
					' Please select elements from table "'.tablename_nice2($tablename).'"' );
			return;
		}
		
		require_once("f.sql_query.inc");
		
		$querylib = new fSqlQueryC($tablename);
		$sqlopt["order"] = 1;

		$sqlAfterNoOrder  = $querylib->full_query_get($tablename);
		$sqlDeviceAfter   = $sqlAfterNoOrder;
		$this->sqlDeviceSelect = 'select x.CHIP_READER_ID from '.$sqlDeviceAfter;
		
		
		
		$objLinkLib = new fObjViewC();
		$html = $objLinkLib->tableViewLink($tablename);
		echo 'List Selection: '.$html.'<br />'. "\n";
	}
	
	private function showNumberselected(&$sqlo) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;

		
		$num = $this->numDevices;
		
		if (!$num) {
			$error->set( $FUNCNAME, 1, 'No devices selected!' );
			return;
		}
		echo 'Search in <b>'.$num.'</b> devices.<br>'."\n";
	}
	
	function startMain() {
		global $error;
		
		$FUNCNAME='startMain';
		
		$this->infox = NULL;
		$sqlo  = &$this->_sqlo;
		$sqlo2 = &$this->_plug_regSql2();
		
		$this->parx = $_REQUEST['parx'];
		$this->hiddenarr = $_REQUEST['hiddenarr'];
		$projid = $_REQUEST['projid'];
		
		$action = $_REQUEST['action'];
		$this->action = $action;
		
		switch ($action) {
			case 'select':
				// analyse selection
				$this->list_select($sqlo);
				break;
			default:
				if (!$projid) {
					$error->set( $FUNCNAME, 1, 'Please choose a project!');
					return;
				}
				$this->_projid = $projid;
				$this->project_Select($sqlo, $projid);
		}
		
		$this->showNumberselected($sqlo);
		if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 1, 'Object selection problem.' );
			return;
		}
		
		
		
		if (!$this->_go) {
			$this->form0($sqlo);
			echo "<br>";
			$this->help();
			return;
		}
		
		$this->parx["searchtext"]   = trim($this->parx["searchtext"]);
		$this->parx["key"]   = trim($this->parx["key"]);
		
		$this->form0($sqlo);
		
		if (!$this->parx["searchtext"] and !$this->parx["key"]) {
			$error->set( $FUNCNAME, 2, 'Please give a search text!');
			return;
		}
		
		
		$this->searchNow($sqlo, $sqlo2);	
	}
	
}
