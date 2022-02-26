<?php
/**
 * export data from list as: CSV; XLS, ...
 * a) streaming
 * b) give download-file
 * - manage the DB-encoding (UTF-8 or ISO-8859-1)
 * - since 2014-09-08: SUBREQ:010: support ARCHDB-export (UREQ:4856)
 * - GLOBAL-params:
 *    $_SESSION['s_tabSearchCond'][$tablename]['arch'] : 0,1
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @package glob.objtab.exp.php
 * @swreq UREQ:0000776: g > many objects: i.Data_export > ODS or XLS or CSV
 * @swreq UREQ:4856: g > Partial-Data-Archiving (PDA)
 * @param string $t : tablename 
 * @param string $format : "csv", "xls", "xlsxml", "xlsx"
 * @param $view_opt
 * 	["colCoNa"]:       0,1 show CODE name of column, e.g. EXP_ID instead of "exp id"
 *  ["add_nice_head"]: 0,1 add header line with table nice column names
 *  ["pureids"]:       0,1 shore pure database-IDs instead of object-names
 *  ["stop_cnt"] :     INTEGER:  if set stop after "stop_cnt" lines (for export of pure data headers ...
 *  @version $Header: trunk/src/www/pionir/glob.objtab.exp.php 59 2018-11-21 09:04:09Z $
 */

session_start(); 


require_once ('reqnormal.inc');

require_once("sql_query_dyn.inc");
require_once("f.sql_query.inc");
require_once("db_x_obj.inc");
require_once("view.sub.csv.inc");
require_once("date_funcs.inc");

require_once ('f.workdir.inc');
require_once ('f.progressBar.inc');
require_once ("javascript.inc" );
require_once ('lev1/f.exportDataFile.inc');

class gObjExp2 {
    
    private $view_opt;
    
	var $filename;
	var $selectCnt;
	var $infox; /* info array
		'filesize'
		'Db.encoding'
	*/
	var $tablename_SQLuse; // if ARCHDB active, the name of an ARCHIVE-table
	
function __construct(&$sqlo, $tablename, $format, $scriptID, $view_opt=NULL) {
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	$this->tablename = $tablename;
	$this->tablename_SQLuse = $tablename;
	$this->infox   = array();
	$this->scriptID= $scriptID;
	$this->format  = $format;
	$this->view_opt = $view_opt;
	
	if ( $_SESSION["userGlob"]["g.debugLevel"]>0 ) {
	    $this->_debug = $_SESSION["userGlob"]["g.debugLevel"];
	}
	
	$this->viSubObj  = new viewSubCsv($tablename);
	$viSubObj = &$this->viSubObj;
	
	$tabSearch = &$_SESSION['s_tabSearchCond'][$tablename];
	$view_opt  = $this->view_opt;
	
	
	
	$rparams = array(
		"modebase"=>NULL,
		"view_opt"=>&$view_opt
	);
	$viSubObj->prViewModeSet($rparams);
	
	$colNames  = columns_get_pos($tablename);
	$classname = $tabSearch['c'];
	
	
	
	$exp_raw_desc_id = $viSubObj->getDynaColMother($sqlo, $mother_idM);
	$access_id_has   = $viSubObj->access_id_has;
	$class_tab_has   = $viSubObj->class_tab_has;
	list($selectCols, $useJoin) = $viSubObj->colinfoget($sqlo, $tablename, $colNames, 
		$access_id_has, $class_tab_has, $classname, $exp_raw_desc_id, 1);
		
	if ( $classname ) {
		$viSubObj->getClassParams($sqlo, $classname);
	}
	
	$selectstr = $viSubObj->sqlColString($selectCols);
	
	
	$sqlDynLib = new fSqlQueryC($tablename);
	
	$this->sqlAfter = $sqlDynLib->full_query_get( $this->tablename_SQLuse );
	//$this->sqlAfter= get_selection_as_sql( $tablename );
	
	
	
	$sortcritXText = $sqlDynLib->_query_sort_org(); //query_sort_org( $tablename );
	$this->infox['sql_info_str']  = $sqlDynLib->get_sql_info();
	$sortcritX 	   = $viSubObj->getSortMatrix( $sortcritXText );
    $tmpSort       = $sqlDynLib->_query_sort_get($sortcritXText); // query_sort_get( $tablename,  );
    
	$this->sqlAfterSort = $this->sqlAfter . $tmpSort;
	$this->sqls_main = "select ".$selectstr." from ". $this->sqlAfterSort;
	
	$this->_doCnt($sqlo);
	
}

function _doCnt(&$sqlo) {
	global $error;
	$FUNCNAME= 'doCnt';
	
	$sqls = "select count(*) from ". $this->sqlAfter; // get number of rows
	//$sqlo->setErrLogging(0); // switch OFF error log (not interesting)
	$retval = $sqlo->query($sqls);
	//$sqlo->setErrLogging(1);

	if ( $retval <= 0) {
		$error->set( $FUNCNAME, 1, 'Error on Query' );
		return;
	}

	if ( $sqlo->ReadRow() )  $this->selectCnt = $sqlo->RowData[0]; // number of selected sets DB
}

function streamStart(&$sqlo) {
	global $error;
	$FUNCNAME= 'streamStart';
	
	$format = $this->format;
	
	$encoding = glob_elementDataGet( $sqlo, 'GLOBALS', 'NAME', 'Db.encoding', 'VALUE');
	$this->infox['Db.encoding'] = $encoding; // save the db-encoding
	
	$expOpt = array();
    $expOpt['encoding']='UTF-8'; // destination encoding
// 	if ($encoding=='UTF-8') {
// 		$expOpt['conv.UTF8_8859'] = 1; // convert UTF to ISO-8859-1
// 	}
	
	$this->streamLib = new f_exportDataFile_C( $format, $this->scriptID, 'data', $expOpt);
	$header_arr = NULL;
	if ($this->view_opt["add_nice_head"]>0) {
	    
	    // add a comment line with the nice names of columns
	    $key='colCoNa';
	    $old_val = $this->viSubObj->get_view_opt($key);
	    
	    $this->viSubObj->set_view_opt($key, 0); // temporary switch to NICE-NAMES
	    $header_row = $this->viSubObj->prHeadRow( $sqlo );
	    
	    $header_row[0] = '#'. $header_row[0]; // add a comment character to the first column
	    $header_arr = array($header_row);
	    
	    $this->viSubObj->set_view_opt($key, $old_val); // reset value
	}
	
	$additional_info_arr = array(
	    'CreationDate: '. date_unix2datestr( time(), 1),
	    'Creator: '. $_SESSION['sec']['appuser'],
	    'Data table: '. tablename_nice2($this->tablename),
	    'Data filter: '. $this->infox['sql_info_str']
	    
	);
	
	$this->streamLib->set_dataset_infos( $additional_info_arr );
	$this->streamLib->outputStart( $header_arr );
    
}

/**
 * show link
 * @return 
 */
function streamStop() {
	$this->streamLib->close();
	
	
	$this->down_url_path     = $this->streamLib->getInfoVal('down_url_path');
	$this->infox['filesize'] = $this->streamLib->getInfoVal('filesize');
}

/**
 * build the output
 * @return 
 * @param object $sqlo
 * @param object $sqlo2
 */
function outputx(&$sqlo, &$sqlo2) {
	global $error;
	$FUNCNAME= 'outputx';

	$viSubObj = &$this->viSubObj;
	
	echo '... Format: '.$this->format.'...<br />' ."\n";
	echo '... Start collecting data ...<br />' ."\n";
	
	$MIN_CNT = 100;
	$flushLib = new fProgressBar( );
	$prgopt = array();
	$prgopt['objname']= 'rows';
  	$prgopt['maxnum'] = $this->selectCnt;
	
	if ($this->selectCnt>$MIN_CNT) {
		$showProgress=1;
  		$flushLib->shoPgroBar($prgopt);
		echo '<br />';
	}
	$sqlo->query($this->sqls_main);
	$hasResult = $sqlo->ReadRow();

	if (!$hasResult) {
		 htmlInfoBox( "No data", "This query contains no data. Please check your conditions.", "", "WARN" );
		 return 0;
	}
	
	list($colNames_show, $foreign_keys) = $viSubObj->headRowPrep();
	
	
	/***************************/
	/*** Start of data table ***/
	/***************************/
	
	// get extented column information
	
	// analyse Db.encoding
	
	$primast=NULL; // not used
	$viSubObj->xColsManage( $sqlo2 );
	$viSubObj->prSetPar2( $foreign_keys, $colNames_show);
	
	$this->streamStart($sqlo2);
	if ($error->Got(READONLY))  {
		$error->set( $FUNCNAME, 1, 'on streamStart' );
		return;
	}
	
	$oneRowArr = $viSubObj->prHeadRow( $sqlo2 );
	$this->streamLib->addHeader($oneRowArr);
	$stop_cnt = $this->view_opt['stop_cnt'];
	
	$cnt=0;
	while( $hasResult )  {
	    
	    if ($stop_cnt>0 and $cnt>=$stop_cnt) {
	        break;
	    }
	    
		$oneRowArr = $viSubObj->print_row($sqlo2, $sqlo->RowData, $cnt);
		$this->streamLib->oneRow($oneRowArr);
		if ($showProgress) $flushLib->alivePoint($cnt);
		
		$cnt++;
		if ( !$sqlo->ReadRow() ) break;
		
	}
	
	if ($showProgress) $flushLib->alivePoint($cnt,1); 
	$this->streamStop();
	
	return 1;
}

function forwardPage() {
	$pureTxt = 'Download file to your computer';
	if ($this->_debug>0) {
		$textOut = '<b><a href="'.$this->down_url_path.'">'.$pureTxt.'</a></b>';
	} else $textOut = $pureTxt;
	echo  '... '.$textOut.' ('.$this->infox['filesize'].
	'). <font color=gray>Starts automatically. Please wait ...</font><br />'."\n";
	echo '<br /><hr>';
	
	js__location_href($this->down_url_path);
	
}

}


// --------------------------------------------------- 
global $error, $varcol;

$error = & ErrorHandler::get();
$sqlo  = logon2( $_SERVER['PHP_SELF'] );
$sqlo2  = logon2(  );
if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();

$tablename 	= $_REQUEST['t'];
$format = $_REQUEST['format'];
$view_opt= $_REQUEST['view_opt'];


$title		= 'Export list view data';

$flushLib = new fProgressBar( );

$infoarr			 = NULL;
$infoarr['scriptID'] = 'glob.objtab.exp';
$infoarr['title']    = $title;
$infoarr['help_url']    = 'g.Bulk_export.html';
$infoarr['form_type']= 'list';
$infoarr['design']   = 'norm';
$infoarr['obj_name'] = $tablename;
$infoarr['css'] = $flushLib->getCss();
$infoarr['javascript'] = $flushLib->getJS(); 

$pagelib = new gHtmlHead();
$pagelib->startPage($sqlo, $infoarr);


//
// @future : an external database could be connected here ...
//
// $dbconfig = $_SESSION['globals']['ext_database']["krake"];
// $sql_work  = logon_to_db( $dbconfig['user'], $dbconfig['passwd'], $dbconfig['host'],  .... );
// $sql_work  = &$sqlo;
// ->outputx($sql_work, $sqlo2);

// access checks ...
$accCheckArr = array('tab'=>array('read'), 'obj'=>array() );
$pagelib->do_objAccChk($sqlo, $accCheckArr);
$pagelib->chkErrStop();

if ($format==NULL) $pagelib->htmlFoot('ERROR', 'No format given');
if ($tablename==NULL) $pagelib->htmlFoot('ERROR', 'No tablename given');
$selectionActive = query_get_info($tablename);
if ($selectionActive==NULL) {
	$pagelib->htmlFoot('ERROR', 'No selection active. Please activate a condtion for this table.' );
}

$mainLib = new gObjExp2($sqlo, $tablename, $format, $infoarr['scriptID'], $view_opt);
$pagelib->chkErrStop();

$doForward = $mainLib->outputx($sqlo, $sqlo2);
$pagelib->chkErrStop();

if ($doForward) {
	$mainLib->forwardPage();
}

$pagelib->htmlFoot();

