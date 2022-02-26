<?php
require_once ('o.CCT_ACCLOG.subs.inc');
require_once 'f.prefvals.inc';
require_once 'gui/f.pageEntriesOrg.inc';
require_once 'visufuncs.inc';

/**
 * - generate a HTML table for a list of objects, dynamic columns
 * - is an ABSTRACT class; see examples for usage
 * - uses REQUEST-avriables:
 *   - $_REQUEST['page']
 *   - $_REQUEST['showList']['action'] = ['html'], 'xlsx', 'csv'
 *   - $_REQUEST['showList']['entryPerPage'] = [20]
 *   - $_REQUEST['showList']['sort'] = sort condition
 * - save session vars in: $_SESSION['s_formState'][$options['URL'].'_showList']
 *    ['entryPerPage'] = [20] : max number : 1000
 *    ['sort'] = keep sort info
 * @package glob.objtab.showList.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @link  file://CCT_QM_doc/89_1002_SDS_code.pdf#gShowObjList
 * @example other scripts: www/lab/plugin/o.CYTORD.sover.inc <pre>
 * 
 * class testHelp extends gShowObjList {
	
	 function initExtend(&$sqlo) {
		 $this->spritzLib = new oDEV_spritz_sub();
	 }
	
	 function oneRowExtend(&$sqlo, $rawData) {
	      return array();
	 }
   }
  
       $sqlAfter = 'from CONCRETE_SUBST x where x.ABSTRACT_SUBST_ID='.$objid. 
       		' and x.SOME_DATE>'.$mindateSql;
	   $showkeys = array(
			'x.CONCRETE_SUBST_ID'=>array('nice'=>'ID'), 
			'x.NAME'=>1, 
			'a.H_ALOG_ACT_ID'=>1, 
			'x.SOME_DATE'=>array('nice'=>'ProdDate'), 
			'x.QUANTITY'=>1
		);
		$options = array(
				'entryPerPage' => 20,
				'URL'  => "p.php?mod=".$this->_mod,
				'title'=> 'Spritzgusswerkzeuge',
				'sql.select'=> 'x.* ',
				'sql.order' => 'x.CHIP_READER_ID DESC',
				'modname'   => $this->_mod
		);
		
		$showLib = new testHelp();
		
		$showLib->init($sqlo, $tablename, $showkeys, $sqlAfter, $option );
		
		$showLib->showList($sqlo, $sqlo2);
		
		
 </pre>
 * @version $Header: trunk/src/phplib/gui/glob.objtab.showList.inc 59 2018-11-21 09:04:09Z $
 */
class gShowObjList {
	
	/**
	 * column description: array[COL_CODE] => array('nice'=>nice_column )
	 *      COL_CODE: 
	 *    		x.COL : table column name
	 *      	a.COL : CCT_ACCESS column name
	 *      	y.COL : extended column name, not related to a table
	 *      'nice' column nice name
	 *      'sort' => [0], 1 is sortable ?
	 * @var array coldesc_STRUCT $coldesc
	 */
	var $coldesc; // column description
	
	/**
	 * array POS => NICE_NAME
	 * @var $colNiceArr
	 */
	var $colNiceArr;
	
	var $colPos2Code; // POS to CODE
	
	/**
	 * @var $options :
	 *   'entryPerPage' : number: shown entries per page
	 *   'URL'          : URL of the plugin, where it is used; e.g. "p.php?mod=LAB/hallo"
	 *   'title' : title of table
	 *   'modname' : Module-identifier; e.g. LAB/spritz/home
	 *   'sql.select' : SQL-string between SELECT and FROM; example: 'x.* '
	 *   'sql.order'  : SQL-string after ORDDER; example: 'x.CHIP_READER_ID DESC'
	 */
	var $options;
	
	var $CERT_FLAG_defs2 = array(
		1=>array('color'=>'#00FF00'),
	);
	
	private $format='html';  // 'excel', 'html', 'cvs'
	
	function __construct() {
		$this->CERT_FLAG_defs = NULL;
	}
	
	/**
	 * abstract method
	 * @param object $sqlo
	 */
	function initExtend(&$sqlo, $options=NULL) {
	}
	
	/**
	 * ABSTRACT class
	 * @param object $sqlo
	 * @param array $rawData
	 * @return array $newData
	 */
	public function oneRowExtend(&$sqlo, $rawData) {	
		return array();
	}
	
	/**
	 * @param string $title
	 * @param string $tablename
	 * @param array coldesc_STRUCT $coldesc
	 * @return array of header
	 */
	function getHeaderArray(&$sqlo, $tablename, $coldesc) {
		
		$auditStateLib = new oAccLogC();
		$this->auditColors = oH_ALOG_ACT_subs::$auditColor;
		
		$this->tablename = $tablename;
		$this->coldesc =   $coldesc;
		$this->colFeatures = NULL;
		$this->pkName = PrimNameGet2($tablename) ;
		
		$this->colPos2Code = array();
		$pos=0;
		$colNiceArr=NULL;
		$colNiceArr[$pos] = '#';
		$this->colPos2Code[$pos] = '';
		$pos++;
		
		foreach( $coldesc as $colCode=>$colInfo) {
			$colType = substr($colCode,0,1);
			$colname = substr($colCode,2);
			
			$mother=NULL;
			switch ($colType) {
				case 'x':
					$mother=$tablename;
					break;
				case 'a':
					$mother='CCT_ACCESS';
					break;
			}
			
			if ($colType=='x' or $colType=='a') {
				$colFeatures  = colFeaturesGet( $sqlo, $mother, $colname);
				$this->colFeatures[$colCode]=$colFeatures;
			} else {
				$colFeatures = array();
			}
			
			$columnNice   = $colFeatures['NICE_NAME'];
			
			if (is_array($colInfo)) {
				if ($colInfo['nice']!=NULL) $columnNice   = $colInfo['nice'];
			}
			
			
			// special actions
			switch ($colCode) {
				case 'x.CERT_FLAG':
					$tmpLib = new fPrefValsGet();
					$this->CERT_FLAG_defs = $tmpLib->getListSort( $sqlo, $this->tablename, 'CERT_FLAG' );
					break;
			}
			
			$colNiceArr[$pos] = $columnNice;
			$this->colPos2Code[$pos] = $colCode;
			
			$pos++;
		}
		reset ($coldesc);
		
		return $colNiceArr;
	}
	
	/**
	 * 
	 * @param string $tablename
	 * @param array $coldesc = array(
	 * 	COL_CODE => array('nice'=>nice_column, 'sort'=>1 )
	 *    COL_CODE: 
	 *    	x.COL : table column name
	 *      a.COL : CCT_ACCESS column name
	 *      y.COL : extended column name, not related to a table
	 * @param array $options
	 *   'URL' : URL of the plugin, where it is used; e.g. "p.php?mod=LAB/hallo"
	 *   'title' : title of table
	 *   'obj.go.URL' : alternative URL to edit.tmpl.php
	 *   'sort.allow' 
	 *      -1 : deny sorting input from $_REQUEST
	 * @return NULL
	 */
	function init(&$sqlo, $tablename, $coldesc, $sqlAfter, $options) {
		
		$this->options	  = $options;
		$this->sqlAfter   = $sqlAfter;
		$this->tablename  = $tablename;
		
		$this->obj_go_url= 'edit.tmpl.php?t='.$this->tablename.'&id=';
		if ($this->options['obj.go.URL']) {
		    $this->obj_go_url= $this->options['obj.go.URL'];
		}
		
		$formStateLocator = $this->options['URL'].'_showList';
		if (!is_array($_SESSION['s_formState'][$formStateLocator]))
			$_SESSION['s_formState'][$formStateLocator] = array();
		
		if ($_REQUEST['showList']['entryPerPage']>0) {
			// max number : 1000
			
			$useval = $_REQUEST['showList']['entryPerPage']>1000 ? 1000 : $_REQUEST['showList']['entryPerPage'];
			$_SESSION['s_formState'][$formStateLocator]['entryPerPage'] = $useval;
		}
		
		if ($this->options['sort.allow']>=0) {
    		if ($_REQUEST['showList']['sort']!=NULL) {
    			$_SESSION['s_formState'][$formStateLocator]['sort'] = $_REQUEST['showList']['sort'];
    		}
		}
		
		$optionsCached    = $_SESSION['s_formState'][$formStateLocator];
		if ($optionsCached['entryPerPage']>0) {
			$this->options['entryPerPage'] = $optionsCached['entryPerPage'];
		}
		if ( $optionsCached['sort']!=NULL and $this->options['sort.allow']>=0 ) {
			$this->options['sql.order'] = $optionsCached['sort'];
		}
		
		
		if ($this->options['entryPerPage']<=0) {
			$this->options['entryPerPage'] = 20; // default
		}
		
		// check input values
		$checkData = array('URL', 'title', 'modname', 'sql.select', 'sql.order' );
		foreach($checkData as $key) {
			if ( $this->options[$key] == NULL ) {
				sys_error_my( 'Parameter $options['.$key.'] missing.' );
			}
		}
		$this->initExtend($sqlo);
		
		$this->colNiceArr = $this->getHeaderArray($sqlo, $tablename, $coldesc);
		
	}
	
	/**
	 * show TABLE-header
	 * @param string $title
	 */
	function showHeader() {
		
		?>
		<script type="text/javascript">
		function sort_pref(tmp_col_ext, sortcritDirNew) {
			location.href="<?php echo $this->options['URL'] .'&showList[sort]';?>=" +tmp_col_ext+ "%20" + sortcritDirNew;
		}
		</script>
		<?php
		
		
		$titleExtend = '&nbsp;&nbsp;<a href="'.$this->options['URL'] .'&showList[action]=xlsx" title="Export as Excel"><img src="images/ic.EXCEL.gif"></a>';
		
		$this->tabobj = new visufuncs();
		$headOpt = array( "title" => $this->options['title'] . ' ' . $titleExtend);
		$headx   = $this->colNiceArr;
		
		// do html-formatting
		
		$sortTmpArr = explode(" ",$this->options['sql.order']);
		$sortColumn = $sortTmpArr[0];
		$sortOrder  = $sortTmpArr[1];
		
		$htmlHeader=array();
		foreach($headx as $pos => $text) {
			$colCode = $this->colPos2Code[$pos];
			$newText = $text;
			
			if ($this->coldesc[$colCode]['sort']>0) {
				$direction='DESC';
				$direct_char = 'v';
				if ($sortColumn==$colCode) {
					if ($sortOrder=='DESC') {
						$direction='ASC';
						$direct_char = '^';
					}
				}
				$newText = "<a href=\"javascript:sort_pref('".$colCode."', '".$direction."')\" style='color:#7070AA'>".$text."</a>";
			}
			if ($sortColumn==$colCode) $newText = $newText.' '.$direct_char;
			
			$htmlHeader[$pos] = $newText;
		}
		
		
		$this->tabobj->table_head($htmlHeader,   $headOpt);
	}
	
	/**
	 * get PREPARED data of one row
	 * @param object $sqlo
	 * @param array $rawData
	 */
	function oneRowGet(&$sqlo, $rawData) {
		
	    $accessFeat=array();
	    if ($rawData['CCT_ACCESS_ID']) {
    		$sqlsel = '* from CCT_ACCESS where CCT_ACCESS_ID='.$rawData['CCT_ACCESS_ID'];
    		$sqlo->Quesel($sqlsel);
    		$sqlo->ReadArray();
    		$accessFeat = $sqlo->RowData;
	    }
	    
		$colopt=NULL;
		$bgcolor = NULL;
		
		$dataArr = NULL;
		$id = $rawData[$this->pkName];
		
		if ($this->format=='html') {
		  $dataArr[] = '<a href="' . $this->obj_go_url . $id.'">'.
			'<img src="images/arrow.but.gif" title="browse to object"></a>';
		} else {
		    $dataArr[] = ''; // TBD: may be no column here?
		}
		
		
		reset ($this->coldesc);
		foreach( $this->coldesc as $colCode=>$val) {
			
			$colType = substr($colCode,0,1);
			$colname = substr($colCode,2);
			$outvalue=='???';
			$convHtml=0;
			
			if ($colCode=='a.H_ALOG_ACT_ID') {
				$tmpval = $accessFeat[$colname];
				if ($tmpval>0) {
					$valname = obj_nice_name ( $sqlo, 'H_ALOG_ACT', $tmpval );
					$bgcolor = $this->auditColors[$valname];
				}
			}
			
			switch ($colType) {
				case 'x':
					$outvalue = $rawData[$colname];
					$convHtml=1;
					break;
				case 'a':
					$outvalue = $accessFeat[$colname];
					$convHtml=1;
					break;
				case 'y':
					$outvalue = $rawData[$colCode];	
					break;
			}
			
			
			if ($this->colFeatures[$colCode]['CCT_TABLE_NAME']!=NULL) {
				$fkTable = $this->colFeatures[$colCode]['CCT_TABLE_NAME'];
				if ($outvalue>0) {
					$outvalue = obj_nice_name ( $sqlo, $fkTable, $outvalue );
				}
			}
			
			if ($colCode=='x.CERT_FLAG' and $outvalue!=NULL) {
				$tmpValColor = '#E0E0E0';
				if ($this->CERT_FLAG_defs2[$outvalue]['color']!=NULL) {
					$tmpValColor = $this->CERT_FLAG_defs2[$outvalue]['color'];
				}
				$extraText = $this->CERT_FLAG_defs[$outvalue];
				$outvalue='<span style="background-color:'.$tmpValColor.';">'.$outvalue.':'.$extraText.'</span>';
				$convHtml=0;
			}
			
			
			if ($convHtml)
				$colOut = htmlSpecialchars( $outvalue );
			else 
				$colOut = $outvalue;
				
			$dataArr[] = $colOut;
		}
		reset ($this->coldesc); 
		
		$colopt=NULL;
		if ($bgcolor!=NULL) {
			$colopt=array('bgcolor'=>$bgcolor);
		}
		
		return array($dataArr, $colopt);
		
	}
	
	function close() {
		$this->tabobj->table_close();
	}
	
	
	/**
	 * get COUNT of selected entries from $this->sqlAfter
	 * @param  $sqlo
	 * @return int $count
	 */
	public function getQueryCnt(&$sqlo) {
		
		if ($this->sqlAfter==NULL) {
			sys_error_my('parameter "sqlAfter" missing');
		}
		
		$sumSQL = $this->sqlAfter;
		$sqlsel = 'count(1) '.$sumSQL;
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$maxcnt = $sqlo->RowData[0];
		
		return $maxcnt;
	}
	
	/**
	 * show list as HTML table
	
	 * @param $sqlo
	 * @param $sqlo2
	 * @param $sqlsel
	 */
	function showList_html(&$sqlo, &$sqlo2) {
	
		$showkeys = $this->coldesc;
		$sumSQL   = $this->sqlAfter;
		$this->format = 'html';
	
		$maxcnt = $this->getQueryCnt($sqlo);
		
		if (!$maxcnt) {
			echo '<br />No data available for this query.'."<br>\n";
			return;
		}
		
		$this->showHeader();
	
		$parx = array('entryPerPage' => $this->options['entryPerPage'] );
		$pageOrgLib = new f_pageEntriesGui();
		$showinf    = array( 'startShow'=> $_REQUEST['page'], 'foundEntries'=>$maxcnt );
		$info       = $pageOrgLib->setParams( $showinf, $parx );
	
		// options['sql.select'] . $this->sqlAfter .  options['sql.order']
		// 'x.* '                . $this->sqlAfter . 'x.CHIP_READER_ID DESC'
		$sqlsel = $this->options['sql.select'] .' '. $sumSQL .' order by ' .  $this->options['sql.order'];
	
		
	
		$moreExists = 0;
		$startShow = $info['start'];
		$endShow   = $info['end'];
	

		$sqlo2->Quesel($sqlsel);
		$cnt   =0;
		$showit=0; // show the entry, if selected page is reached in loop
		
		while ( $sqlo2->ReadArray() ) {
	
			if ( $cnt >= $startShow ) $showit=1;
			if ( $cnt > $endShow )   {
				$moreExists = 1;
				break;
			}
				
			if ($showit) {
				$rawData  = $sqlo2->RowData;
		
				$moredata = $this->oneRowExtend($sqlo, $rawData);
				if (!is_array($moredata)) $moredata=array();
				$sumdata  = array_merge($rawData, $moredata);
				
				
				/*
				if ( $_SESSION["userGlob"]["g.debugLevel"]>0 ) {
					glob_printr( $sumdata, "sumdata info" );
				}
				*/
				
					
				list($dataArr, $colopt) = $this->oneRowGet($sqlo, $sumdata);
				$this->tabobj->table_row ($dataArr, $colopt);
			}
			
			$cnt++;
	
		}
		$this->close();
	
		$pageOrgLib->setLastShownEntry($cnt-1);
		$desturl = $this->options['URL'] . '&page=';
		
		// add page size selector
		$addText = "\n".'&nbsp;&nbsp;&nbsp;<span style="color:#808080">EntriesPerPage:</span> ';
		$pageSizeArr = array(20, 50, 100);
		if( !in_array($this->options['entryPerPage'], $pageSizeArr) ) {
			$pageSizeArr[] = $this->options['entryPerPage'];
			sort($pageSizeArr); // sort values
		}
		
		foreach($pageSizeArr as $onesize) {
			$pageOneHtml = '<a href="'.$this->options['URL'].
				'&showList[entryPerPage]='.$onesize.'">'.$onesize.'</a>';
			if ($onesize==$this->options['entryPerPage']) $pageOneHtml = '<b>'.$pageOneHtml.'</b>';
			$addText .= '['.$pageOneHtml.'] ';
		}
		
		$pageOrgLib->showEntryNav($desturl, array('addtext'=>$addText) );
	}
	
	/**
	 * export data in file
	 * @param object $sqlo
	 * @param object $sqlo2
	 */
	function exportList(&$sqlo, &$sqlo2, $format='xlsx') {
		global $error;
		
		require_once 'lev1/f.exportDataFile.inc';
	
		$this->format = $format;
		$showkeys = $this->coldesc;
		$sumSQL   = $this->sqlAfter;
	
		$sqlsel   = $this->options['sql.select'] .' '. $sumSQL .' order by ' .  $this->options['sql.order'];
	
		
		$headerArray = $this->colNiceArr;
	
		$filename_base    = str_replace("/", "_", $this->options['modname']);
		$workDirName = $filename_base;
		
		
		
		$exportObj   = new f_exportDataFile_C( $this->format, $workDirName, $filename_base);
		if ($error->Got(READONLY))  {
			return;
		}
		// FUTURE: array('# '.$this->options['title']), 
		$headerx   = array( $headerArray );
		$exportObj->outputStart( $headerx );
	
		$sqlo2->Quesel($sqlsel);
		$cnt=0;
		while ( $sqlo2->ReadArray() ) {
	
			$rawData  = $sqlo2->RowData;
	
			$moredata = $this->oneRowExtend($sqlo, $rawData);
			$sumdata  = array_merge($rawData, $moredata);
			
			list($dataArr, $colopt) = $this->oneRowGet($sqlo, $sumdata);
	
			$exportObj->oneRow($dataArr);
	
			$cnt++;
	
		}
	
		$exportObj->close();
		echo $exportObj->getDownloadText();
	}
	
	/**
	 * show/export list HTML or XLSX or CSV
	 * INPUT: $_REQUEST['showList']['action'] : ['html'], 'xlsx'
	 * @param $sqlo
	 * @param $sqlo2
	 */
	function showList(&$sqlo, &$sqlo2) {
	    
	    $format = $_REQUEST['showList']['action'];
	    if ($format==NULL) $format='html';
	    
	    if ($format=='html') {
	        $this->showList_html($sqlo, $sqlo2);
	    } else {
	        $this->exportList($sqlo, $sqlo2, $format);
	    }
	}
	
}
