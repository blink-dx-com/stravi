<?php
/**
 * Statistical summary of many worklists
 * @package o.WORKLIST.ana1.inc
 * @swreq: UREQ:7536 o.WORKLIST > Analyse: Allgemein, Statistische Zahlen
 * @version $Header: trunk/src/plugin/o.WORKLIST.ana1.inc 59 2018-11-21 09:04:09Z $
 * @global $_SESSION['userGlob']['o.WORKLIST.ana1'] = array(
 *   'cols'=>array( XPOS=>array( statscol1, statscol3, ...)
 * )
 * @param $go
 * @param $in
 * @param $showform : 0,1
 */

require_once ("o.WORKLIST.subs.inc");
require_once ('f.sql_query.inc');
require_once ("f.objview.inc");
require_once 'o.WORKLIST.content.inc';
require_once 'f.math.inc';
require_once ('gui/f.htmlGraph2.inc');

class o_WORKLIST_PlugAna1_oneCol {
	
	function __construct($absEntries) {
		$this->absEntries = $absEntries;
	}
	
	public function initWorklist(&$sqlo, $wlid ) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$this->wlid = $wlid;
		$this->ABS_WORKLIST_ID= glob_elementDataGet( $sqlo, 'WORKLIST', 'WORKLIST_ID', $wlid, 'ABS_WORKLIST_ID');
		
		$this->oneRowLib = new oWORKLIST_oneRow($sqlo, $this->absEntries, $wlid, $this->ABS_WORKLIST_ID);
		if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 1, 'Error on init worklist:'.$wlid );
			return;
		}
		
		$this->allRowArr = array();
		
		$sqlsel = "distinct(YPOS) from WORKLIST_ENTRY where WORKLIST_ID=".$wlid." order by YPOS";
		$sqlo->Quesel($sqlsel);
		
		$this->y_index=array();
		
		while ( $sqlo->ReadArray() ) {
				
			$ypos = $sqlo->RowData['YPOS'];
			$this->y_index[] = $ypos;
		}
		
		foreach ($this->y_index as $ypos) {
		
			$sqlsel = "* from WORKLIST_ENTRY where WORKLIST_ID=".$wlid." and YPOS=".$ypos." order by XPOS";
			$sqlo->Quesel($sqlsel);
		
			$rowArr = NULL; // data of row
			while ( $sqlo->ReadArray() ) {
				$xpos  = $sqlo->RowData['XPOS'];
				$table = $sqlo->RowData['TABLE_NAME'];
				$objid = $sqlo->RowData['OBJID'];
				$val   = $sqlo->RowData['SH_NOTES'];
				$this->allRowArr[$ypos][$xpos] = array( $table, $objid, $val );
			}
			
		}
	}
	
	/**
	 * 
	 * @param unknown $wlid
	 * @param unknown $XPOS
	 * @return array('cnt', 'mean', 'stddev')
	 */
	public function analyseColumn(&$sqlo, $XPOS) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$wlid = $this->wlid;


		$dataArray = array(); 
		
		
		foreach($this->y_index as $YPOS) {
			
			$oneCellData = array();
			$rowArr = $this->allRowArr[$YPOS];
			
			$this->oneRowLib->setRowData($rowArr);
			$objHtmlText = $this->oneRowLib->getColView($sqlo, $XPOS);
			$dataArray[$YPOS]=$objHtmlText;
			
			
			
		}
		
		// analyse data array!
		$meanval = 0;
		$cnt     = 0;
		$sum     = 0;
		$onlyNumbers = array();
		$stddev  = 0;
		foreach($dataArray as $YPOS=>$value) {
			
			if (is_numeric($value)) {
				$sum   = $sum + $value;
				$onlyNumbers[]=$value;
				$cnt++;
			}
			
		}
		if ($cnt) {
			$meanval = $sum/$cnt;
		}
		if ($cnt>1) {
			$stddev = fMathC::standard_deviation($onlyNumbers);
		}
			
		return array( 'cnt'=> $cnt, 'mean'=>$meanval, 'stddev'=>$stddev );
		
		
	}
}

class o_WORKLIST_PlugAna1_anaOneWL {
	
	function __construct($absEntries, $absViewCols) {
		$this->absEntries  = $absEntries;
		$this->absViewCols = $absViewCols;
		$this->columnAnaLib = new o_WORKLIST_PlugAna1_oneCol( $this->absEntries );
	}
	
	/**
	 * 
	 * @param unknown $sqlo
	 * @param unknown $sqlo2
	 * @param unknown $wlid
	 * @return Array(XPOS => $stats_array)
	 */
	function getData(&$sqlo, $wlid) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$retarr=array();
		
		$this->columnAnaLib->initWorklist($sqlo, $wlid );
	
		foreach($this->absViewCols as $XPOS) {

			$statsOneCol = $this->columnAnaLib->analyseColumn($sqlo, $XPOS);
			$retarr[$XPOS] = $statsOneCol;
		}

		return $retarr;
	}
}


class o_WORKLIST_ana1_XPL extends gPlugin {
	
	private $max_values_array;

	
	function __construct(&$sqlo) {
		parent::__construct($sqlo);
	}
	
	function register() {
		$tablename	= 'WORKLIST';
		$title		= 'Statistical summary';
		
		$this->infoarr			 = NULL;
		$this->infoarr['title_sh'] = $title;
		$this->infoarr['title']    = 'Statistical summary of many worklists';
		$this->infoarr['help_base'] = 'wiki';
		$this->infoarr['help_url'] = 'plugin/o.worklist.ana1';
		$this->infoarr['form_type']= 'list'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['obj_cnt']  = 1; // count current selection
		
		
		$this->infoarr2 = array( 
				'accCheck' => array('tab'=>array('read'), 'obj'=>array() ),
				'listChkSelect' => 1
				);
		
	}
	
	function plug_init() {
		$this->statscols = array(
				'cnt'   =>array('t'=>'cnt'),
				'mean'  =>array('t'=>'mean'),
				'stddev'=>array('t'=>'stddev'),
				'gh_mean' =>array('t'=>'graph mean')
		);
	}
	
	function preCheck( &$sqlo, $sqlAfter ) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$this->colx_ID    = 5; // column of NOTES
		$colNice ='Notes';
		
		$sqlsel = "x.WORKLIST_ID from " . $sqlAfter ;
		$sqlo->Quesel ( $sqlsel );
		$sqlo->ReadRow ();
		$first_worklistid = $sqlo->RowData[0];
		
		
		$this->workAnaLib = new oWORKLIST_subs($first_worklistid);
		$initAnswer       = $this->workAnaLib->init($sqlo);
		
		$this->absWorkId  = $initAnswer['absWorklistId'];
		$this->maxy = $initAnswer['maxy'];
		
		$this->absEntries = $this->workAnaLib->getPlannedAttribs($sqlo);
		$this->absViewCols = array();
		foreach($this->absEntries as $oneColumn=> $columnSettings) {
			
			
			// check "VIEW" columns
			if ( $columnSettings['typ']=='VIEW'  ) {
				$this->absViewCols[]=$oneColumn;
			}
		}
		
		
		
		// check "VIEW" columns
		if ( !sizeof($this->absViewCols) ) {
			$error->set( $FUNCNAME, 1, 'No column defined as VIEW-Type!' );
			return;
		}
		
		$sqlsel = "count(distinct(x.ABS_WORKLIST_ID)) from " . $sqlAfter ;
		$sqlo->Quesel ( $sqlsel );
		$sqlo->ReadRow ();
		$num_abs_worklists = $sqlo->RowData[0];
		
		if ($num_abs_worklists!=1) {
			$error->set( $FUNCNAME, 1, 'Alle Worklists mm&uuml;ssen vom selben Typ (WLA) sein.' );
			return;
		}
	}
	
	/**
	 * show a form
	 */
	function gui_settings() {
		
		$old_settings = &$this->table_cols_show;
		
		$initarr   = NULL;
		$initarr["title"]       = "Column settings"; // title of form
		$initarr["submittitle"] = "Submit"; // title of submit button
		$initarr["tabwidth"]    = "600";   // table-width: AUTO
		
		$hiddenarr = NULL; // hidden form variables
		
		// init form, uses class formc
		$formobj = $this->_html->formInit($initarr, $hiddenarr, 0);
		
		
		
		$tmptext=NULL;
		foreach( $this->absViewCols as $XPOS) {
			
			$columnSettings = $this->absEntries[$XPOS];
			$loop_title = $columnSettings['title'];
			
			$tmptext=NULL;
			foreach($this->statscols as $colid=>$colfeatures) {
				
				$check_txt='';
				if ( is_array($old_settings[$XPOS]) and in_array($colid, $old_settings[$XPOS]) ) $check_txt='checked';
				
				$tmptext .= $colfeatures['t'].' <input type=checkbox value=1 name="in['.$XPOS.']['.$colid.']" '.$check_txt.'>'.
					'&nbsp;&nbsp;&nbsp;';
			}

			
			$fieldx = array ( // form-field definition
					"title"   => $loop_title,
					"name"  => "",
					"object"=> "info2",
					"val"    => $tmptext,
					"colspan"=> 2 
					
			);
			$formobj->fieldOut( $fieldx ); // output the form-field
			
		}

		$formobj->close( TRUE ); // close form, sshow submit-button
	}
	
	/**
	 * save column gui settings
	 * @param unknown $col_settings
	 */
	function save_gui_settings($col_settings) {
		
		if (!sizeof($col_settings)) return;
		
		$save_settings=array();
		foreach($col_settings as $XPOS=>$stats_col_array) {
			foreach($stats_col_array as $colid=>$value) {
				$save_settings[$XPOS][]=$colid;
			}
		}
		
		$_SESSION['userGlob']['o.WORKLIST.ana1']['cols'] = serialize($save_settings);
		
	}
	
	private function sign( $number ) {
		return ( $number > 0 ) ? 1 : ( ( $number < 0 ) ? -1 : 0 );
	}
	
	/**
	 * evaluate min, max values of ALL columns, not only the selected ...
	 * OUTPUT: $this->max_values_array
	 * @param unknown $sqlo
	 */
	private function _pre_List_Analysis(&$sqlo, &$sqlo2, $sqlAfter) {
		
		$tmp_anaLib = new o_WORKLIST_PlugAna1_anaOneWL($this->absEntries, $this->absViewCols);
		
		$sqlsel = "x.WORKLIST_ID, x.NAME from " . $sqlAfter;
		$sqlo2->Quesel ( $sqlsel );
		
		$this->max_values_array = array();
		
		while ( $sqlo2->ReadRow () ) {
				
			$worklistid    = $sqlo2->RowData[0];
				
			$allData_WL = $tmp_anaLib->getData($sqlo, $worklistid, $this->absEntries); // $XPOS => $stats_array
			
			foreach( $this->absViewCols as $XPOS) {
			
				$stats_data_per_XPOS = $allData_WL[$XPOS];
				
				foreach($this->statscols as $colid=>$dummy) {
					$value = $stats_data_per_XPOS[$colid];
					
					if ( $value>$this->max_values_array[$XPOS][$colid] ) $this->max_values_array[$XPOS][$colid] = $value;
				}
			}
					
		}
		

	}

	function startMain() {
		global $error;
		
		$FUNCNAME='startMain';
		
		
		//$table  = $_REQUEST['t'];
		$sqlo   = &$this->_sqlo;
		$show_gui_settings = $_REQUEST['showform'];
		
		$this->plug_init();
		
		$sqlo2         = &$this->_plug_regSql2 ();
		$sqlAfter      = $this->listLib->getSqlAfter ();
		$sqlAfterNoOrd = $this->listLib->getSqlAfterNoOrder();
		
		$numlists = $this->listLib->getObjCnt();
		
		if ($numlists>500) {
			$this->htmlFoot('INFO','Aktuell sind nur maximal 500 Worklists hier erlaubt.');
		}
		
		
		$this->preCheck( $sqlo, $sqlAfter );
		if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 1, 'Init error.' );
			return;
		}
		
		if ($_REQUEST['go']>0) {
			$this->save_gui_settings($_REQUEST['in']);
		}
		
		if ( $_SESSION['userGlob']['o.WORKLIST.ana1']['cols']==NULL ) {
			$show_gui_settings = 1;
		} else {
			$this->table_cols_show = unserialize($_SESSION['userGlob']['o.WORKLIST.ana1']['cols']);
		}
			
		if ($show_gui_settings) {
			$this->gui_settings();
			return;
		}
		echo '<a href="p.php?mod='.$this->_mod.'&showform=1">Column settings</a><br>';
		
		$this->_pre_List_Analysis($sqlo, $sqlo2, $sqlAfter);
		
		// show a table
		$headx = array('WLID', 'Name');
		foreach( $this->table_cols_show as $XPOS=>$stats_cols) {
			$columnSettings = $this->absEntries[$XPOS];
			$loop_title = $columnSettings['title'];
			foreach($stats_cols as $stats_col_id) {
				
				$title = $this->statscols[$stats_col_id] ['t'];
				$headx[]=$loop_title.':<br>'.$title;
			}
		}

		$headOpt = array ("title"=>'Statistical Summary');
		$this ->_html->_table3_init ($headx , $headOpt );
		
		$objLinkLib = new fObjViewC();
		
		$tmp_anaLib = new o_WORKLIST_PlugAna1_anaOneWL($this->absEntries, $this->absViewCols);
		
		$sqlsel = "x.WORKLIST_ID, x.NAME from " . $sqlAfter;
		$sqlo2->Quesel ( $sqlsel );
		while ( $sqlo2->ReadRow () ) {
			
			$worklistid    = $sqlo2->RowData[0];
			$worklist_name = $sqlo2->RowData[1];
			
			$url_wl= $objLinkLib->bo_IDLink('WORKLIST', $worklistid, $worklist_name ); // get link to object
			
			$allData_WL = $tmp_anaLib->getData($sqlo, $worklistid, $this->absEntries); // $XPOS => $stats_array
			
			$outarr = array($worklistid, $url_wl);
			
			
			foreach( $this->table_cols_show as $XPOS=>$stats_cols) {
				
				
				$stats_array = $allData_WL[$XPOS];
				$mean = $stats_array['mean'];
				
				$digits = 1; // the number of digits to round ...
				if ($mean===0) {
					$digits = 1;
				} else {
					$digits = (int)(log10($mean));
					$digits = $digits - 3*$this->sign($mean);
				}
				
				$tmparr=array();
				$tmparr['cnt']    = $stats_array['cnt'];
				$tmparr['mean']   = round($stats_array['mean'], -$digits);
				$tmparr['stddev'] = round($stats_array['stddev'], -$digits);
				if ( in_array('gh_mean', $stats_cols) ) {
					$maxval = $this->max_values_array[$XPOS]['mean'];
					$tmpGraphlib = new fHtmlGraph2C($maxval, 150);
					$tmparr['gh_mean'] = $tmpGraphlib->getImgHtml($stats_array['mean']);
				}
				
				foreach($stats_cols as $stats_col_id) {
					$outarr[]=$tmparr[$stats_col_id];
				}
				
				
			}
			
			
			$this ->_html -> _table3_row ( $outarr );
		}
		
		$this ->_html -> _table3_close ();
		
	}
	
}
