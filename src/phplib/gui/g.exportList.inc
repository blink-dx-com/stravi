<?php
/**
 * manage export to a temp-file for a SQL-selection statement, 
 * - the single export is managed by an interface class,
 *  methods: singleRow()
 * @namespace core::gui::objtab
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
require_once('f.workdir.inc');
require_once('f.progressBar.inc');
require_once("sql_query_dyn.inc");

class g_exportList_C {
	var $filename;
	var $selectCnt;

/**
 * 
 * @return 
 * @param object $sqlo
 * @param object $format 'xls', 'csv'
 * @param object $scriptID
 * @param $sqls_main e.g. "EXP_ID, NAME from exp where exp_id < 3000"
 * @param object $options[optional]
 *   'selectCnt' : number of objects
 */	
function __construct(&$sqlo, $format, $scriptID, $sqls_main, $options=NULL) {

	$this->infox   = NULL;
	$this->scriptID= $scriptID;
	$this->format  = $format;
	$this->view_opt  = $options;
	$this->sqls_main = $sqls_main;
	$this->selectCnt= $options['selectCnt'];
	
	if ( $_SESSION["userGlob"]["g.debugLevel"]>0 ) {
	    $this->_debug = $_SESSION["userGlob"]["g.debugLevel"];
	}
 
}



function streamStart() {
	global $error;
	$FUNCNAME= 'streamStart';
	
	$format = $this->format;
	
	$workdirObj = new workDir();
	$tmpdir   = $workdirObj->getWorkDir ( $this->scriptID );
	if ($error->Got(READONLY))  {
		$error->set( $FUNCNAME, 1, 'error on getWorkDir' );
		return;
	}
	$this->workSubDir = $workdirObj->subDirName;
	
	$this->filename = 'tempfile.dat';
	$this->filenameFull = $tmpdir . DIRECTORY_SEPARATOR . $this->filename;
	if (  $this->_debug > 0 ) {
		echo "INFO: temp-file-name: ".$this->filenameFull.'<br />'."\n";
	}
	
	switch ($format) {
		case 'xls':
			require_once("export/f.xls.simple.inc");
			$this->filenameNice = 'data.xls';
			$this->streamLib = new f_export_xls_simple($this->filenameFull);
			$this->toolInfo = $this->streamLib->toolInfo;
			break;
		case 'csv':
			require_once("export/f.csv.inc");
			$this->filenameNice = 'data.txt';
			$this->streamLib = new f_export_csv($this->filenameFull);
			break;
	}
	
    $this->streamLib->openFile() ;
    
}

/**
 * show link
 * @return 
 */
function streamStop() {
	$this->streamLib->closeFile();
	
	$filename = $this->filename;
	$sizex    = filesize($this->filenameFull);
	$tmpsizeMb= $sizex* 0.000001;
	$sizeOut  = $sizex.' bytes';
	if ($sizeOut>1000000) $sizeOut  = sprintf("%.2f MBytes", $tmpsizeMb);
	
	$this->down_url_path = "f_workfile_down.php?name=".$filename."&file=".
          $this->workSubDir."/".$filename.'&name='.$this->filenameNice;
          
	$this->infox['filesize'] = $sizeOut;
	/* 
	 echo "<a href=\"". $this->down_url_path . "\" onmouseover=\"status='".$this->filenameNice.
			"';return true;\"><img src=\"images/ic.docdown.big.gif\" border=0 TITLE=\"Download\"></a>".
			"&nbsp;<b>Download</b>".
			' ('.$sizeOut.')';
	*/
}

/**
 * build the output
 * @return 
 * @param object $sqlo
 * @param object $sqlo2
 * @param object $interClass - the interface class
 * @param array $headerarr array of array(header column names)
 */
function outputStart(&$sqlo, &$sqlo2, &$interClass, &$headerarr) {
	global $error;
	$FUNCNAME= 'outputStart';

	// $viSubObj = &$this->viSubObj;
	
	echo '... Format: '.$this->format.'...<br />' ."\n";
	echo '... Start collecting data ...<br />' ."\n";
	
	$MIN_CNT = 100;
	$flushLib = new fProgressBar( );
	$prgopt['objname']= 'rows';
  	$prgopt['maxnum'] = $this->selectCnt;
	
	if ($this->selectCnt>$MIN_CNT) {
		$showProgress=1;
  		$flushLib->shoPgroBar($prgopt);
		echo '<br />';
	}
	$retVal    = $sqlo->Quesel($this->sqls_main);
	if ($error->Got(READONLY))  {
		$error->set( $FUNCNAME, 1, 'on streamStart' );
		return;
	}
	$hasResult = $sqlo->ReadRow();
	
	$queryResults = $selectedBaseCnt;
	if ( $sqlo->RowData==NULL ) $queryResults=0;
	
	if (!$hasResult) {
		$error->set( $FUNCNAME, 1, "No data: This query contains no data." );
		return 0;
	}
	
	
	/***************************/
	/*** Start of data table ***/
	/***************************/
	
	// get extented column information
	
	$this->streamStart();
	if ($error->Got(READONLY))  {
		$error->set( $FUNCNAME, 1, 'on streamStart' );
		return;
	}
	
	
	reset ($headerarr);
	foreach( $headerarr as $dummy=>$oneRow) {
		$this->streamLib->addRow($oneRow);
	}
	reset ($headerarr); 
	
	$cnt=0;
	while( $hasResult )  {
		$oneRowArr = $interClass->singleRow($sqlo2, $sqlo->RowData, $cnt);
		$this->streamLib->addRow($oneRowArr);
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
