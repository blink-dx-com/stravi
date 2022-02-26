<?php
/**
 * analyze application log
 * - read applog-file
 * - show entries in a table
 * 
 * @package f.appLogAna.php
 * @swreq SREQ:0001004 g > f.appLogAna.php application log analysis 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param $parx['shownum'] number of lines
 * @param $parx['mode'] : 
 * 	'apperr', application log
 *  'phplog'  PHP error log
 *  'MODUL_LOG' : Module-log : SREQ:0003368
 * @param $parx['ignore'] : komma separated list of keys: 'USERERROR', 'INFO'
 * @param $parx['ignoretxt']: ignore lines, containing  given text
 * @param $parx['white']  : komma separated list of WHITELIST keys: 'USERERROR', 'INFO'
 * @param $parx['seaText']  : search for seaText in the wild text
 * @param $parx['prio']   : number, show all events >= prio
 * @global $_SESSION['userGlob']['f.appLogAna.php'] serialized array
 * @version $Header: trunk/src/www/pionir/rootsubs/f.appLogAna.php 59 2018-11-21 09:04:09Z $
 */


extract($_REQUEST); 
session_start(); 

require_once ('reqnormal.inc');
require_once ("visufuncs.inc");
require_once ('func_form.inc');
require_once ('f.appLogAna.inc');



// ------------------------
/**
 * abstract class !!!
 * @author steffen
 *
 */
class fappLogAnaC {
	function __construct($parx) {
		
		if (!$parx['shownum']) $parx['shownum']=20;
		$this->parx=$parx;
		$this->logmode = $parx['mode'];
		// glob_printr( $parx, "parx info" );
	}
	
	public function finishLoop() {}
	public function postInfo() {}
	public function getMoreFields($parx) {}
	
	function initFile() {
		global $error;
		$FUNCNAME = "initFile";
		$this->_countDataLines( );
		if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 1, 'count lines failed.' );
			return;
		}
		$this->open();
	}
	
	function _info($key, $text) {
		echo '<font color=gray>'.$key.':</font> '.$text.'<br>'."\n";
	}
	
	function open() {
		global $error;
		$FUNCNAME= 'open';
		
		if ($this->fileName==NULL) {
			$error->set( $FUNCNAME, 1, "No filename given!");
			return 0;
		}
		
		$this->linecnt = 0;
		$this->FH = @fopen($this->fileName, 'r');
		if ( !$this->FH ) {
			$error->set( $FUNCNAME, 2, "Can't open file '".$this->fileName."'");
			return 0;
		}
		return 1;
	}
	
	function _countDataLines(  ) {
		global $error;
		$FUNCNAME = "countDataLines";
	
		$this->lineCntMax = 0;
		if ( !$this->open() ) return;
		
	    $LINE_LENGTH = 32000;
		$linecount = 0;
		while ( !feof ( $this->FH ) ) {
			 $line  = fgets($this->FH, 32000);
			 $linecount++;
		}
		fclose($this->FH);
		
		$this->lineCntMax=$linecount;
		
	}
	
	
	
	/**
	 * - read one line
	 * @return boolean 0,1
	 * @global $this->line
	 */
	function _readLine() {
		if ( feof ( $this->FH ) ) return 0;
		$this->line  = fgets($this->FH, 32000);
		$this->linecnt++;
		return 1;
	}
	
	
	
	function showTable() {
		$parx=$this->parx;
		
		if ($parx["shownum"]<5) $parx["shownum"]=10;
		$startCnt = $this->lineCntMax - $parx["shownum"];
		if ($startCnt<0) $startCnt=0;
		
		$this->_info('File', $this->fileName);
		$this->_info('lines', $this->lineCntMax);
		
		
		// read till start
		while ( $this->_readLine() ) {
			$line = $this->line;
			if ( ($this->linecnt+1) >= $startCnt ) {
				break;
			}
		}
		$lineArr = explode( "\t", $line, 2); 
		$tmpDateStart = $lineArr[0];
		$this->_info('Start at line', $startCnt. ' Date: '.$tmpDateStart);
		
		
		$this->tabobj = new visufuncs();
		$headOpt = array( "title" => $this->tableTitle);
		$headx   = $this->headx;
		$this->tabobj->table_head($headx,   $headOpt);
		
		$lineCnt=0;
		$ignoredCnt=0;
		while ( $this->_readLine() ) {
			$line = $this->line;
			
			if ( trim($line)  == NULL ) continue;
			
			// echo "-DEBUGG: $line <br>\n";
			
			$ignored = $this->_rowout($this->linecnt, $line);
			if (!$ignored) $lineCnt++;
			else $ignoredCnt++;
		}
		fclose($this->FH);
		
		$this->finishLoop();
		
		$this->tabobj->table_close();
		
		echo 'Lines shown: '.$lineCnt.'; ignored: '.$ignoredCnt."<br />\n";
		$this->postInfo();

	}
	
	function form() {
		
		
		$parx=$this->parx;
		
		$initarr   = NULL;
		$initarr["action"]      = $_SERVER['PHP_SELF'];
		$initarr["title"]       = "Show Parameters";
		$initarr["submittitle"] = "Submit";
		$initarr["tabwidth"]    = "AUTO";
	
		$hiddenarr = NULL;
		$hiddenarr['parx[mode]']=$parx["mode"];
		$formobj = new formc($initarr, $hiddenarr, 0);
	
		$fieldx = array ( 
			"title" => "Lines", 
			"name"  => "shownum",
			"object"=> "text",
			"val"   => $parx["shownum"], 
			"notes" => "number of lines"
			 );
		$formobj->fieldOut( $fieldx );
		
		$fieldxArr = $this->getMoreFields($parx);
		
		if ( is_array($fieldxArr) ) {
			$formobj->fieldArrOut($fieldxArr);
		}
		
		
	
		$formobj->close( TRUE );
	
	}
}



// --------------------------------------------------- 
global $error, $varcol;

$error = & ErrorHandler::get();
$sqlo  = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();

$parx	= $_REQUEST['parx'];
$go		= $_REQUEST['go'];
$title	= 'analyze log: '.$parx['mode'];
$role_right_name = "f.appLogAna.php";

$infoarr			 = NULL;
$infoarr['scriptID'] = '';
$infoarr['title_sh'] = $title;
$infoarr['title']    = $title . ' (access with role-right: '.$role_right_name.')';
$infoarr['form_type']= 'tool';
$infoarr['design']   = 'norm';
$infoarr['locrow']   = array( array('rootFuncs.php', 'home') );

$pagelib = new gHtmlHead();
$headarr = $pagelib->startPage($sqlo, $infoarr);


$role_right = role_check_f ($sqlo , $role_right_name );
if ( $role_right !='execute' ) {
	$pagelib->htmlFoot( 'ERROR_USER', 'Sorry , you must have role right "'. $role_right_name .
		'" to use this tool.');
}

/*
if ( !glob_isAdmin() ) {
     $pagelib->htmlFoot( 'ERROR_USER',   
     "Only root can execute this!".
     " For security reason it is not allowed for common users" );
     
}
*/

if ($parx['mode']==NULL) {
	$pagelib->htmlFoot( 'ERROR_USER', "Please give a mode!");
}

if (!$go) {
	$tmpParx = $_SESSION['userGlob']['f.appLogAna.php'];
	if ($tmpParx!=NULL){
		$tmpParx2 = unserialize($tmpParx);
		$parx['ignore']=$tmpParx2['ignore'];
		$parx['ignoretxt']=$tmpParx2['ignoretxt'];
		$parx['prio']  =$tmpParx2['prio'];
	}
} else {
	$_SESSION['userGlob']['f.appLogAna.php'] = serialize($parx);
}

switch ($parx['mode']) {

	case 'apperr':
		$MainLib = new fappErrLog($parx);
		break;
		
	case 'phplog':
		$MainLib = new fphpErrLog($parx);
		break;
	case 'MODUL_LOG':
		require_once ('f.appLogAna_MODUL.inc');
		$MainLib = new fModulLog($sqlo, $parx);
		break;
}
$pagelib->chkErrStop();

$MainLib->initFile();
$pagelib->chkErrStop();
$MainLib->form();
$MainLib->showTable();

$pagelib->chkErrStop();

$pagelib->htmlFoot();
