<?php
/**
 * add one log entry to a selection of CHIP_READER
 * @package obj.chip_reader.lognew_li.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq UREQ:0001498: o.CHIP_READER > add service logs for selection of devices 
 * @param array parx
 * @param int $go
 * @param array $tabArgu
 */
session_start(); 

require_once ('reqnormal.inc');
require_once ('subs/obj.rea_log.edform.inc');
require_once ("o.REA_LOG.new.inc");
require_once ('glob.objtab.page.inc');
require_once ("f.visuTables.inc");
require_once ("func_form.inc");
require_once ('f.msgboxes.inc'); 
/**
 * add log entry to list of devices
 * @author steffen
 */
class oCHIP_READER_logAdd_li {
	
function __construct($parx, $go, $sqlAfter) {
	$this->go = $go;
	$this->parx=$parx;
	$this->sqlAfter=$sqlAfter;
}

function GoInfo($go, $coltxt=NULL) {
	$goArray   = array( "0"=>"Give parameters", 1=>"Prepare", 2=>"Update" );
	$extratext = '[<a href="'.$_SERVER['PHP_SELF'].'">Start again</a>]';
	
	$formPageLib = new FormPageC();
	$formPageLib->init( $goArray, $extratext );
	$formPageLib->goInfo( $go ); 

} 

function _getInfo(&$sqlo, $objid) {
	
	$sqlsel="POS, NAME from REA_LOG where CHIP_READER_ID=".$objid. " order by POS desc";
	$sqlo->Quesel($sqlsel);
	$sqlo->ReadArray();
	$this->lastLog = $sqlo->RowData;
}


function newLoop( &$sqlo, &$sqlo2 ) {
	global $error;
	$FUNCNAME= 'newLoop';
	
	$parx = $this->parx;
	$go = $this->go;
	$ReaLogNew = new oREA_LOG_new();
	
	$this->sum = array();
	
	if ($parx['KEY']==NULL) {
		$error->set( $FUNCNAME, 2, 'no parameter KEY set!');
		return;
		
	}
	
	$tabobj = new visufuncs();
	$headOpt = array( "title" => "Device-list log list" );
	$headx  = array ( "device", 'Status', "Last Pos", 'Last Log entry' );
	$tabobj->table_head($headx,   $headOpt);

	
	
	$sqlAfter = $this->sqlAfter;
	$sqlsLoop = "x.CHIP_READER_ID, x.NAME FROM ".$sqlAfter;
	$sqlo2->Quesel($sqlsLoop);
	while ( $sqlo2->ReadRow() ) {
		
		$looperror = 0;
	    $objid   = $sqlo2->RowData[0];
	    $objname = $sqlo2->RowData[1];
	    $statTxt = '';
	    $this->_getInfo($sqlo, $objid);
	    
	    if ($this->lastLog['NAME']==$parx['NAME']) {
	    	$statTxt = '<span style="color:#D06000;"><b>Warning</b></span>: already added? ';
	    }
	    
	    $ReaLogNew->setDevice($objid);
	    
	    do {
	    
		    $info = $ReaLogNew->paramCheck($sqlo, $parx);
		    if (is_array($info)) {
		    	$looperror = 1;
		    	$statTxt = '<span style="color:red;">Error</span>: '.$info[1];
				break;
		    }
		    
			$ReaLogNew->checkAccess($sqlo);
			if ($error->Got(READONLY))  {
				$statTxt = '<span style="color:red;">Error</span>: no access';
				$looperror = 1;
				$error->reset();
				break;
			} 
		
			if ($go==2) {
				$ReaLogNew->addEntry( $sqlo, $parx);
				if ($error->Got(READONLY))  {
					$error->set( $FUNCNAME, 2, 'error occurred on object '.$objid );
					return;
				}
			}
			$this->sum['ok']++;
			$statTxt .= '<span style="color:green;">o.k.</span>';
			
	    } while (0);
	    
	    if ($looperror) {
	    	$this->sum['error']++;
	    }
		
		$infoArr = array( $objname, $statTxt, $this->lastLog['POS'], $this->lastLog['NAME']);
		$tabobj->table_row ($infoArr);
		
		$this->sum['cnt']++;
	}
	
	$tabobj->table_close();
	echo '<br />';
	
	if ($this->sum['error']>0) {
		$visuLib = new fVisuTables();
		$visuLib->showSummary( $this->sum ); 
	}
	
	if ( $go==2 ) {
		echo "<br>";
		cMsgbox::showBox("ok", 'log to '. $this->sum['ok']." devices added."); 
	}
}

function form0(&$sqlo) {
	$EditFormLib = new oChip_readerLogC ();
	$hiddenarr=NULL;
	$formopt = array('allowKEY'=>1);
	$EditFormLib->edform( $sqlo, $_SERVER['PHP_SELF'].'?go=1', $this->parx, $hiddenarr, $formopt);
}

/** 
  - special form for Service-entries
  - only for table-admins
 */
function form1Service(&$sqlo) {

	echo 'Info: Der Nutzer braucht das Rollenrecht: '.tablename_nice2('CHIP_READER').':admin<br><br>';
	$answer = role_admin_check ( $sqlo, 'CHIP_READER' );
	if (!$answer) {
		htmlInfoBox( "Abgelehnt", 'Du musst das Rollenrecht "admin" fuer Tabelle '.tablename_nice2('CHIP_READER') .
			' besitzen, um Service-Logs anzufuegen.', "", "WARN" );
		return;
	}

	// get first device
	$sqlAfter = $this->sqlAfter;
	$sqlsLoop = "x.CHIP_READER_ID, x.NAME FROM ".$sqlAfter;
	$sqlo->Quesel($sqlsLoop);
	$sqlo->ReadRow();
	$objid   = $sqlo->RowData[0];
	
	// get A_CHIP_READER_ID
	$sqlsLoop = "A_CHIP_READER_ID FROM CHIP_READER where CHIP_READER_ID=".$objid ;
	$sqlo->Quesel($sqlsLoop);
	$sqlo->ReadRow();
	$aDevID  = $sqlo->RowData[0];

	$EditFormLib = new oChip_readerLogC ();
	$hiddenarr=NULL;
	$formopt = array('A_DEVID'=>$aDevID);
	$EditFormLib->edform( $sqlo, $_SERVER['PHP_SELF'].'?go=1.5', $this->parx, $hiddenarr, $formopt);
}

function form1() {
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Prepare";
	$initarr["submittitle"] = "Update";
	$initarr["tabwidth"]    = "AUTO";

	$hiddenarr = NULL;
	$formobj = new formc($initarr, $hiddenarr, 1);
	$formobj->addHiddenParx( $this->parx );
	$formobj->close( TRUE );
	echo '<br><br>';
}

function prepare(&$sqlo) {
	
	$LEN_YYYYMMDD = 10;
	// repair some values
	$this->parx['XDATE']  = trim($this->parx['XDATE']);
	$this->parx['XDATE2'] = trim($this->parx['XDATE2']);
	
	if ( strlen($this->parx['XDATE'])>0 and strlen($this->parx['XDATE'])<=$LEN_YYYYMMDD ) {
		$this->parx['XDATE'] .= ' 00:00';
	}
	
	// only if set
	if ( strlen($this->parx['XDATE2'])>0 and strlen($this->parx['XDATE2'])<=$LEN_YYYYMMDD ) {
		$this->parx['XDATE2'] .= ' 23:59';
	}
	
	$visuLib = new fVisuTables();
	$visuLib->showKeyValRaw($this->parx, 'Parameters');
	echo "<br />\n";
}

}


global $error, $varcol;

$error = & ErrorHandler::get();
$sqlo  = logon2( $_SERVER['PHP_SELF'] );
$sqlo2  = logon2(  );
if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();

$go 		= $_REQUEST['go'];
$parx 		= $_REQUEST['parx'];
$tabArgu    = $_REQUEST['tabArgu'];

if ($tabArgu['KEY']!=NULL) $parx['KEY'] = $tabArgu['KEY']; // transport parameter

$tablename='CHIP_READER';
$title       = "add log entry to a selection of devices";
$infoarr=array();
$infoarr["obj_cnt"] = 1;          // show number of objects
$infoarr["title"]   = $title;  
$infoarr["title_sh"]   = 'add log'; 
$mainObj = new gObjTabPage($sqlo, $tablename );
$headopt=array();
$mainObj->showHead($sqlo, $infoarr, $headopt);
$mainObj->initCheck($sqlo);
echo '<ul>';

$i_tableNiceName = tablename_nice2($tablename);

$t_rights = tableAccessCheck( $sqlo, $tablename );
if ( $t_rights["write"] != 1 ) {
	tableAccessMsg( $i_tableNiceName, "insert" );  // $answer = getTableAccessMsg( $name, $right );
	htmlFoot();
}

$sqlAfter = $mainObj->getSqlAfter();

$MainLib = new oCHIP_READER_logAdd_li($parx, $go, $sqlAfter);
$MainLib->GoInfo($go);
if ( !$go ) {
	$MainLib->form0($sqlo);
	htmlFoot('<hr>');
}

$MainLib->prepare($sqlo);

if ( $go==1 ) {

	
	if ($MainLib->parx['KEY']=='Service') {
		$MainLib->form1Service($sqlo);
	} else {
		$ReaLogNew = new oREA_LOG_new();
		$info = $ReaLogNew->paramCheck($sqlo, $MainLib->parx);
		if (is_array($info)) {
			
			htmlErrorBox("Error", "Parameter", $info[1] );
			echo "<br>";
			
			$MainLib->form0($sqlo);
			return;
		}
		$MainLib->form1();
		$MainLib->newLoop($sqlo, $sqlo2);
	}
	$error->printAll();
}

if ($go==1.5) {
	// need only for KEY='Service'
	$ReaLogNew = new oREA_LOG_new();
	$info = $ReaLogNew->paramCheck($sqlo, $MainLib->parx);
	if (is_array($info)) {
		htmlErrorBox("Error", "Parameter", $info[1] );
		$MainLib->form1Service($sqlo);
		return;
	}
	$MainLib->form1();
	$MainLib->newLoop($sqlo, $sqlo2);
	$error->printAll();
}

if ( $go==2 ) {
	$MainLib->newLoop($sqlo, $sqlo2);
	$error->printAll();
	 
}

htmlFoot('<hr>');