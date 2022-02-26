<?php
require_once('f.workdir.inc');


/**
 * manage export to a temp-file for set of data on server-work-path
 * - expects the data as ISO-8859-1 format, but UTF-8 data can be converted automatically
 *   with option $options['conv.UTF8_8859']
 * @package f.exportDataFile.inc
 * @author Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq UREQ:0001292: g > provide an export-library of data for CSV, XLS, ... 
 * @version $Header: trunk/src/phplib/lev1/f.exportDataFile.inc 59 2018-11-21 09:04:09Z $
 * @example
 *   $exportObj = new f_exportDataFile_C('xls', 'test', '/tmp/sess_3737374/temp.xls');
 *   
 
 *   
 *   $headerx = array( array('col1', 'col2') );
 *   $exportObj->outputStart( $headerx );
 *   $exportObj->oneRow(array('val1',566.45));
 *   $exportObj->close();
 *   $exportObj->getDownloadText();
 *   
 */
class f_exportDataFile_C {
	var $filename;
	var $selectCnt;
	var $fileExt;    // extension of nice filename 

/**
 * 
 * init class 
 * @param object $format 'xls', 'csv', 'xlsx'
 * @param string $scriptID
 * @param string $filename short filename
 * @param object $options[optional]
 *   'selectCnt' : number of objects
 *   'workDir'   : give full path of workdir instead of using class workDir()
 *   'separator' : separator for CSV: e.g. ','
 *   'typeArr'   : array( COLID => ('string', 'numeric') )
 *   'autoFileExt'   : use auto file-extension ?
 *      -1  : no, 
 *      [1] : yes [default]
 *   'encoding'  : [ISO-8859-1] PHP-defined Charset; see htmlentities()
 *   'conv.UTF8_8859' : do a DATA convert from UTF-8 to ISO-8859-1
 *   	
 */	
function __construct( $format, $scriptID, $filename, $options=NULL) {

	$this->filename    = $filename;
	$this->filenameNice= NULL;
	$this->infox   = NULL;
	$this->scriptID= $scriptID;
	$this->format  = $format;
	$this->view_opt= $options;
	$this->selectCnt= $options['selectCnt'];
	$this->options= $options;
	$this->expectFile=1;
	$this->_dataset_infos = NULL;
	
	// set encoding
	if ($this->view_opt['encoding']==NULL) {
		$this->view_opt['encoding']='UTF-8';   // 'ISO-8859-1';
	}
	
	if ( $_SESSION["userGlob"]["g.debugLevel"]>0 ) {
	    $this->_debug = $_SESSION["userGlob"]["g.debugLevel"];
	}
 
}

// get relative file-path to session-work-dir; e.g. "r_stats/datafile.dat"
function getSessionRelPath() {
	return $this->workSubDir."/".$this->filename;
}


/**
 * start stream output
 * @return -
 */
function _streamStart($options=NULL) {
	global $error;
	$FUNCNAME= 'streamStart';
	
	$format = $this->format;
	
	if ($this->options['workDir']!=NULL) {
		$tmpdir = $this->options['workDir'];
		$this->workSubDir = basename($tmpdir);
	} else {
		// create work-dir
		$workdirObj = new workDir();
		$tmpdir   = $workdirObj->getWorkDir ( $this->scriptID );
		if ($error->Got(READONLY) or $tmpdir==NULL)  {
			$error->set( $FUNCNAME, 1, 'error on getWorkDir' );
			return;
		}
		$this->workSubDir = $workdirObj->subDirName;
	}
	
	if ($this->filename==NULL) {
		$error->set( $FUNCNAME, 2, 'filename is empty.' );
		return;
	}
	
	$this->filenameFull = $tmpdir . DIRECTORY_SEPARATOR . $this->filename;
	if (  $this->_debug > 0 ) {
		echo "INFO: temp-file-name: ".$this->filenameFull.'<br />'."\n";
	}
	
	if ($this->filenameNice==NULL)  $this->filenameNice = $this->filename; 
	switch ($format) {
		case 'xls':
			require_once("export/f.xls.simple.inc");
			
			$this->view_opt['conv.UTF8_8859'] = 1; // convert UTF to ISO-8859-1
			
			$this->streamLib = new f_export_xls_simple($this->filenameFull, $this->view_opt);
			$this->toolInfo = $this->streamLib->toolInfo;
			$this->fileExt  = 'xls';
			break;
			
		case 'xlsxml':
			require_once("export/f.xlsxml.simple.inc");
			
			// $this->view_opt['conv.UTF8_8859'] = 1; // convert UTF to ISO-8859-1
			
			$this->streamLib = new Excel_XML($this->filenameFull, $this->view_opt);
			if ($this->view_opt['encoding']!=NULL) {
				$this->streamLib->setEncoding( $this->view_opt['encoding'] );
			}
			$this->toolInfo = $this->streamLib->toolInfo;
			$this->fileExt  = 'xls.xml';
			break;
		case 'xlsx':
		    require_once("export/f.xlsx.inc");
		    
		    // EXCEL needs Windows-1252
		    // $this->view_opt['conv.UTF8_8859']=1;
		    
		    $this->streamLib = new f_export_xlsx($this->filenameFull, $this->view_opt);
		    if ($this->view_opt['encoding']!=NULL) {
		        $this->streamLib->setEncoding( $this->view_opt['encoding'] );
		    }
		    $this->toolInfo = $this->streamLib->toolInfo;
		    $this->fileExt  = 'xlsx';
		    break;
		case 'csv':
			require_once("export/f.csv.inc");
			$this->streamLib = new f_export_csv($this->filenameFull, $this->view_opt);
			$this->fileExt  = 'txt';
			break;
		case 'datadict':
			require_once("export/f.datadict.inc");
			$this->expectFile = 0;
			$this->streamLib = new f_export_datadict($this->view_opt);
			break;
		case 'html':
			require_once("export/f.html.inc");
			$this->expectFile = 0;
			$this->streamLib = new f_export_html($this->filenameFull, $this->view_opt);
			$this->fileExt  = 'html';
			break;
	}
	
	if ($this->_debug>0) {
	    debugOut('Format: |'.$format.'| Stream_lib: '. intval($this->streamLib), $FUNCNAME, 1);
	} 
	
	if ($this->filenameNice!=NULL) {
		// correct the file extension
		if ( $this->options['autoFileExt']!=-1) {
			if (substr($this->filenameNice,-strlen($this->fileExt)) != $this->fileExt) {
				$this->filenameNice .= '.'.$this->fileExt;
			}
		}
	}
	
    $this->streamLib->openFile() ;
    
}


/**
 * start the output 
 * @param array $headerarr array of array(header column names)
 */
function outputStart($headerarr=NULL) {
	global $error;
	$FUNCNAME= 'outputStart';	
	
	/***************************/
	/*** Start of data table ***/
	/***************************/
	
	// get extented column information
	
	$this->_streamStart();
	if ($error->Got(READONLY))  {
		$error->set( $FUNCNAME, 1, 'on streamStart' );
		return;
	}
	
	if( is_array($this->_dataset_infos) and $this->format=='xlsx' ) {
	    $this->streamLib->set_dataset_infos($this->_dataset_infos);
	}
	
	if ( is_array($headerarr) ) {
		
		foreach( $headerarr as $oneRow) {
			$this->streamLib->addHeader($oneRow);
		}
	}
}

// add header
function addHeader(&$oneRowArr) {
    $this->streamLib->addHeader($oneRowArr);
}

function set_dataset_infos($dataset_infos) {
    $this->_dataset_infos = $dataset_infos;
}

function oneRow(&$oneRowArr) {
	
	if ($this->view_opt['conv.UTF8_8859']>0) {
		// convert data from UTF-8 => ISO8859-1
		
		foreach( $oneRowArr as $colid=>$val) {
			$oneRowArr[$colid] = utf8_decode($val);
		}
	}
	$this->streamLib->addRow($oneRowArr);
	
}

// must be called before function close() !!!
// only need the base name here
function setFileNameNice($filenameNice) {
	$this->filenameNice = $filenameNice;
}

function close() {
	
	$this->streamLib->closeFile();
	
	if (!$this->expectFile) return;
	
	$filename = $this->filename;
	$sizex    = filesize($this->filenameFull);
	$tmpsizeMb= $sizex* 0.000001;
	$sizeOut  = $sizex.' bytes';
	if ($sizeOut>1000000) $sizeOut  = sprintf("%.2f MBytes", $tmpsizeMb);
	
	$this->down_url_path = $_SESSION['s_sessVars']['loginURL']."/f_workfile_down.php?file=".
          $this->workSubDir."/".$filename.'&name='.urlencode($this->filenameNice);
    $this->infox['filesize']      = $sizeOut;
    $this->infox['down_url_path'] = $this->down_url_path;
	/* 
	 echo "<a href=\"". $this->down_url_path . "\" onmouseover=\"status='".$this->filenameNice.
			"';return true;\"><img src=\"images/ic.docdown.big.gif\" border=0 TITLE=\"Download\"></a>".
			"&nbsp;<b>Download</b>".
			' ('.$sizeOut.')';
	*/
}

/**
 * get value of key
 * @param $key
 */
function getInfoVal($key) {
	return $this->infox[$key];
}

function getFileNameFull() {
	return $this->filenameFull;
}

function &getInternalData($key) {
	if ( method_exists($this->streamLib, 'getData')) {
		return $this->streamLib->getData($key);
	}
}

function getDownloadText() {
	$textOut = '<b><a href="'.$this->down_url_path.'">Download file</a></b> ('.$this->infox['filesize'].')';
	return $textOut;
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
