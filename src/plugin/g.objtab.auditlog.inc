<?php

require_once ("visufuncs.inc");
require_once ("o.H_ALOG_ACT.subs.inc");
require_once ('func_form.inc');
require_once ("gui/f.pageEntriesOrg.inc");
require_once ('lev1/f.exportDataFile.inc');

/**
 * - list for "audit log" of a selection of objects 
 *    
 * @package g.objtab.auditlog.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq   UREQ:0000931: o.CCT_ACCLOG > Anzeige/Suche im Audit log ermöglichen
 * @param   $_REQUEST['t'] tablename
 * @param   $_REQUEST['page'] page number, start with 1
 * @param   $_REQUEST['go'] start search
 * @param   $_REQUEST['sea'] serach params
 * @global  $_SESSION['s_formState']['g.objtab.auditlog] = array of search vars
 * @global  $_REQUEST['parx'] 
 *             'format' : ['html'], 'xls'
 *   
 */
class g_objtab_auditlog_XPL extends gPlugin {
	
	function __construct(&$sqlo) {
		parent::__construct($sqlo);
	}
	
	function register() {
		$tablename	= $_REQUEST['t'];
		$title		= 'Analyse audit log';
		
		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'list'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['obj_cnt']  = 1;
		$this->infoarr['inforow']    = 'This tool shows the audit log entries of selected objects.';
		
		
		$this->infoarr2 = array( 
			'accCheck' => array('tab'=>array('read'), 'obj'=>array() ),
			'listChkSelect' =>1,
			'progressBar'   =>1,
			);
				
		
	}
	
	/**
	 * 
	 * @param $sqlo
	 * @param $sqlAfterNoOrd
	 * @return array _STRUCT_paramana
	 *    'cntobj'=>, 
	 *    'cntAuditNoCond'=>, 
	 *    'cntAudit'=>, 
	 *    'sqlAuditAfter'=>
	 */
	function _searchPrep(&$sqlo, $sqlAfterNoOrd, $parx) {
		
		if ($parx['format']==NULL) $parx['format']='html';
		$this->parx = $parx;
		
		if ( $_SESSION['s_formState']['g.objtab.auditlog']['sea']!=NULL ) {
			$searchWhere = $this->_creaCondbyParam($sqlo, $_SESSION['s_formState']['g.objtab.auditlog']['sea']);
		} else $searchWhere = NULL;
		
		$sqlAfterNoOrd = $this->listLib->getSqlAfterNoOrder();
		$cntall = $this->listLib->getObjCnt();
		
		$sqlAuditAfter = "CCT_ACCLOG where CCT_ACCESS_ID in (select a.CCT_ACCESS_ID from ".$sqlAfterNoOrd.")";
		
		$sqlsel = "count(1) from ".$sqlAuditAfter; // without any special condition
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$auditCntNoCond = $sqlo->RowData[0];
		
		if ($searchWhere!=NULL) {
			$sqlAuditAfter .= ' and ('.$searchWhere.')';
		}
		$sqlsel = "count(1) from ".$sqlAuditAfter;
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$auditCnt = $sqlo->RowData[0];
		
		return array('cntobj'=>$cntall, 'cntAuditNoCond'=>$auditCntNoCond, 'cntAudit'=>$auditCnt, 'sqlAuditAfter'=>$sqlAuditAfter);
	}
	
	function _table_head($header, $numrows) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$this->loopcnt=0;
	
		if ($this->parx['format']=='xls') {
			$workDirName = str_replace ('/','.', $this ->_mod ); // use the module name
			// creates a work -dir , inits the temporary file
			$this->fileExpLib = new f_exportDataFile_C ('xls', $workDirName , 'auditlog.xls');
			$headerarr=array();
			$headerarr[] = $header;
			
			$this->fileExpLib->outputStart ( $headerarr );
			if ( $error->Got ( READONLY )) {
				$error->set ( $FUNCNAME , 1, 'export start failed.' );
				return;
			}
			$prgopt = array ('maxnum'=> $numrows );
			$this->_html->progressInit ( $prgopt ); // init progress bar
			
		} else {	
			$this->tabobj = new visufuncs();
			$headOpt= array( "title" => "audit log entries");
			$this->tabobj->table_head($header, $headOpt);
		}
	}
	
	function  _table_row ($outarr) {
		if ($this->parx['format']=='xls') {
			$this->fileExpLib->oneRow ( $outarr ); // write one line to file
			$this ->_html->progressAlive ( $this->loopcnt );
		} else {	
			$this->tabobj->table_row ($outarr);
		}
		
		$this->loopcnt++;
	}
	
	function _table_close() {
		if ($this->parx['format']=='xls') {
			$this->_html->progressAlive ( $this->loopcnt ,1) ; // finish progress bar
			$this->fileExpLib -> close ();
			$downtext = $this->fileExpLib->getDownloadText ();
			$this->_html->line_out( '' );
			$this->_html->line_out( $downtext );
		} else {
			$this->tabobj->table_close();
		}
	}
	
	
	/**
	 * show html-list of audit log
	 * @param  $sqlo
	 * @param  $sqlo2
     * @param  array $inParams 
	 * @return -
	 */
	function _showList(&$sqlo, &$sqlo2, $inParams ) {
		// $cntall
		$tablename= $this->tablename;
		$format   = $this->parx['format'];
		
		$auditCnt      = $inParams['cntAudit'];
		$sqlAuditAfter = $inParams['sqlAuditAfter'];
		
		$sortimg = ' <img src="images/but.desc.gif">';
		$header = array( 'object', 'pos', 'user', 'date'.$sortimg,'status','QM value', 'notes' );
		$pkname = PrimNameGet2($tablename);
		$nameCol= importantNameGet2($tablename); 
		
		
		$pageNavLib = new f_pageEntriesGui();
		$showinf= array( 'startShow'=> $_REQUEST['page'], 'foundEntries'=>$auditCnt );
		$parx   = NULL;
	    $info   = $pageNavLib->setParams( $showinf, $parx );
	    $cnt    = 0;
		$moreExists = 0;
		$startShow = $info['start'];
		$endShow   = $info['end'];
		
		if ( $format=='xls' ) {
			// export all
			$startShow = 0;
			$endShow   = $auditCnt;
		}
		
		$this->_table_head($header, $inParams['cntAudit']);
		
			
		$sqlsel = "* from " .$sqlAuditAfter." order by ACT_DATE DESC";
		$sqlo2->Quesel($sqlsel);
		while ( $sqlo2->ReadArray() ) {
			$showit = 0;
			
			if ( $cnt>=$startShow ) $showit=1;
			
			if ( $cnt > $endShow )   {
				$moreExists = 1;
				break;
			}
			
			if ($showit) {
			
			    $accdata    = $sqlo2->RowData;
			    $cct_acc_id = $accdata['CCT_ACCESS_ID'];
			    
			    $sqlsel = $pkname.", ".$nameCol." from ".$tablename." where CCT_ACCESS_ID=".$cct_acc_id;
			    $sqlo->Quesel($sqlsel);
				$sqlo->ReadRow();
				$objid    = $sqlo->RowData[0];
				$objname  = $sqlo->RowData[1];
				
				if ($accdata['DB_USER_ID']) {
					$sqlsel = "nick from DB_USER where DB_USER_ID=".$accdata['DB_USER_ID'];
				    $sqlo->Quesel($sqlsel);
					$sqlo->ReadRow();
					$user_name  = $sqlo->RowData[0];
				} else $user_name=NULL;
				
				if ($accdata['H_ALOG_ACT_ID']) {
					$sqlsel = "NAME from H_ALOG_ACT where H_ALOG_ACT_ID=".$accdata['H_ALOG_ACT_ID'];
				    $sqlo->Quesel($sqlsel);
					$sqlo->ReadRow();
					$H_ALOG_name  = $sqlo->RowData[0];
				} else $H_ALOG_name=NULL;
			    
			    $outarr = array(
			        '<a href="edit.tmpl.php?t='.$tablename.'&id='.$objid.'">'.$objname.' ['.$objid.']</a>', 
			    	$accdata['POS'], $user_name, $accdata['ACT_DATE'], $H_ALOG_name, $accdata['QM_VAL'], $accdata['NOTES'] );
			    $this->_table_row ($outarr);
			    
			}
			$cnt++;
		}
		
		$this->_table_close();
		
		
		if ( $format=='xls' ) {
			//
		} else {
		
			$desturl='p.php?mod='.$this->_mod.'&t='.$tablename.'&page=';
			$pageNavLib->setLastShownEntry($cnt-1);
			$pageNavLib->showEntryNav ( $desturl );
		}
	}
	
	function _addCond($sqlWhere, $condition) {
		$sqlWhere .= $this->_tmpand . $condition;
		$this->_tmpand=' AND ';
		return $sqlWhere;
	}
	
	
	/**
	 * build additional search condition
	 * @param $sqlo
	 * @return unknown_type
	 */
	function _creaCondbyParam(&$sqlo, $searchParams) {
		
		$this->_tmpand=NULL;
		
		$sqlWhere = NULL;
		if ($searchParams['H_ALOG_ACT_ID']) {
			$sqlWhere = $this->_addCond( $sqlWhere, 'H_ALOG_ACT_ID='.$searchParams['H_ALOG_ACT_ID'] );
		}
		if ($searchParams['NOTES']!=NULL) {
			$sqlWhere = $this->_addCond( $sqlWhere, "UPPER(NOTES) like '%".strtoupper($searchParams['NOTES'])."%'" );
		}
		if ($searchParams['QM_VAL']!=NULL) {
			$sqlWhere = $this->_addCond( $sqlWhere, 'QM_VAL='.$searchParams['QM_VAL']  );
		}
		
		return $sqlWhere;
	}
	
	/**
	 * show a search form
	 * @param $sqlo
	 * @return unknown_type
	 */
	function _searchForm(&$sqlo) {
		
		$searchParams = $_SESSION['s_formState']['g.objtab.auditlog']['sea'];
		
		$alog_lib = new oH_ALOG_ACT_subs();
		$h_alog_entries = $alog_lib->getActionEntries($sqlo);
		
		
		echo "<form style=\"display:inline;\" method=\"post\" ".
		 " name=\"editform\"  action=\"p.php\" >\n";
		echo "<input type=hidden name='mod' value='".$this->_mod."'>\n";
		echo "<input type=hidden name='t' value='".$this->tablename."'>\n";
		echo "<input type=hidden name='go' value='1'>\n";
		
		echo "<table cellpadding=1 cellspacing=1 border=0 bgcolor=#EFEFEF>\n<tr><td>";
		echo "<input type=submit value=\"Search\">\n"; // SUBMIT
		
		$option1=NULL;
		$preselected = $searchParams['H_ALOG_ACT_ID'];
		$seltext = formc::selectFget( 'sea[H_ALOG_ACT_ID]', $h_alog_entries, $preselected, $option1); 
		echo 'status: '.$seltext;
		
		$option1=NULL;
		$qmvals = array('0.5'=>'0.5', '1'=>1);
		$preselected = $searchParams['QM_VAL'];
		$seltext = formc::selectFget( 'sea[QM_VAL]', $qmvals, $searchParams['QM_VAL'], $option1); 
		echo 'QM value: '.$seltext;
		
		$tmpval='';
		if ($searchParams['NOTES']!=NULL) $tmpval = htmlspecialchars ($searchParams['NOTES']);
		echo 'notes: <input type=text name="sea[NOTES]" value="'.$tmpval.'" size=30>'."<br />\n";
		
		$selvals = array('html'=>'html', 'xls'=>'xls');
		$seltext = formc::selectFget( 'parx[format]', $selvals, 'html', $option1); 
		echo 'Format: '.$seltext."<br />\n";
		
		echo "</td></tr></table>\n";
		echo "</br>\n";
		
		

		
		echo "</form>";
	}
	

	function startMain() {
		global $error;
		
		$FUNCNAME='startMain';
		
		$this->tablename   = $_REQUEST['t'];
		$parx  = $_REQUEST['parx'];
		$sqlo  = &$this->_sqlo;
		$sqlo2 = &$this->_plug_regSql2();
		
		if ($this->tablename==NULL) {
			$this->htmlFoot('ERROR', 'tablename missing');
		}
		
		$searchWhere=NULL;
		if ($_REQUEST['go']) {
			$searchParams = $_REQUEST['sea'];
			$_SESSION['s_formState']['g.objtab.auditlog']['sea']=$searchParams;
		}
		
		
		$searchParams = $this->_searchPrep($sqlo, $sqlAfterNoOrd, $parx);
		
		if (!$searchParams['cntAuditNoCond']) {
			$this->_html->messageBox('No entries','','WARN', 'No audit entries found.');
			return;
		}
		
		$this->_searchForm($sqlo);
		if (!$searchParams['cntAudit']) {
			$this->_html->line_out('');
			$this->_html->messageBox('No entries','','WARN', 'No audit entries found for condition.');
			return;
		}
		
		$this->_showList($sqlo, $sqlo2, $searchParams );
	}
	
}
