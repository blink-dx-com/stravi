<?php
/**
 * reopen selected objects
 * - user must be table admin ?
 * @package glob.objtab.accreopen.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq UREQ:0001142: g > reopen selected objects 
 * @param 
 * 	$t -- tablename	
	$do		
 	$parx ['notes']
 	
 */
session_start(); 


require_once ("reqnormal.inc"); // includes all normal *.inc files
require_once ('access_lock.inc');
require_once ('o.CCT_ACCLOG.gui.inc');
require_once ("f.objview.inc");	
require_once ("f.visu_list.inc"); 
require_once ('lev1/o.CCT_ACCESS.reopen.inc');


class gObjaccReopenLiGui {
	
	var $hasManiRole; // user is table admin? 0:no, 1:yes

function __construct( &$sqlo, $tablename, $go, $parx) {
	$this->tablename=$tablename;
	$this->go = $go; // "lock"
	$this->parx = $parx;
	$this->table_flag=0;
	
	$this->lockObj      = new gObjaccLock();
	$this->accLogLib    = new oAccLogC();
	
	$sqlopt["order"] = 1;
	$this->sqlAfter  = get_selection_as_sql( $tablename, $sqlopt);
	
	$this->pkname = PrimNameGet2($tablename);
	$this->CCT_ACCLOG_nice = tablename_nice2("CCT_ACCLOG"); 
	
	$this->hasManiRole = 0;
	$hasManiRole = oAccLogStaticC::checkManiRole($sqlo, $tablename);
	if ( $hasManiRole[0]=="execute" ) $this->hasManiRole = 1;
	
	$this->reopenLib = new gObjAccReopen($sqlo);
	
	$goArray   = array( 
		"0"=>"Prepare reopen", 
		1  =>"Reopen"
		);
	$extratext = "[<a href=\"".$_SERVER['PHP_SELF']."?t=".$this->tablename."\">Start again</a>]";
	
	$formPageLib = new FormPageC();
	$formPageLib->init( $goArray, $extratext );
	$formPageLib->goInfo( $go );
	
}

function setObj($objid, $cct_access_id) {
	$this->objid = $objid;
	$this->cct_access_id = $cct_access_id;
	$this->info = NULL;
}

/**
 * check access permission on object
 * @param $sql
 */
function accessInfo( &$sql ) {
	global $error;
	$FUNCNAME= "accessInfo";
	
	$id = $this->objid;
	$tablename = $this->tablename;
	$cct_access_id = $this->cct_access_id;
	
	$hasMani = access_hasManiRights( $sql, $cct_access_id );
	$this->info["hasMani"] = $hasMani;
	
	
	$o_rights = access_check($sql, $this->tablename, $id);
	if ( !$o_rights["entail"] ) {
		$this->info["noentail"] = 1;
	}
	
	if ( $this->info["hasMani"]>0 AND $this->info["noentail"] AND !$this->hasManiRole) {
		// can not manipulate
		$error->set( $FUNCNAME, 1, "no 'entail'-right on object." );
		return;
	} 
	
	$this->accLogLib->setObject( $sql, $tablename,  $id, $cct_access_id );
	$lastAccKey = $this->accLogLib->getLastLog($sql);
	if ($lastAccKey) $this->info["lastLog"] = $lastAccKey;
	
	
	if ( !$this->info["hasMani"] ) {
		// want add action
		// need ROLE
		if ( !$this->hasManiRole ){
			$this->info["addLogErr"] = 1;
			return;
		}	
		// get last CCT_ACCLOG
			
		if ( $lastAccKey!="" AND $this->parx["action"]!=NULL AND $lastAccKey==$this->parx["action"] ) {
			$error->set( $FUNCNAME, 2, $this->CCT_ACCLOG_nice." entry exists" );
			return;
		}
	}
	
	
}

/**
 * reopen one object
 * - check, if user has WRITE-access
 * - add access, if needed
 * - add "reopen" audit
 * @param $sql
 * @return -
 */
function accessReopenOne( &$sqlo ) {
	// OUTPUT: $this->info
			
	global $error;
	$FUNCNAME= "accessLockOne";
	
	$parx = $this->parx;
	$this->reopenLib->doReopen($sqlo, $this->cct_access_id, $this->tablename, $this->objid, $parx);
	
	$tmpbr = $this->info["inf"]!="" ? ", " : "";
	$this->info["inf"] .= $tmpbr."added log";
	
	
	
}

function formAsk(&$sql) {
	
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Reopen and add reason";
	$initarr["submittitle"] = "Reopen and add reason";
	$initarr["tabwidth"]    = "AUTO";

	$hiddenarr = NULL;
	$hiddenarr["t"] = $this->tablename;

	
	$formobj = new formc($initarr, $hiddenarr, 0);
	
	$fieldx = array ( 
		"title" => "Reopen reason:",
		"name"  => "notes",
		"object"=> "textarea",
		"inits" => array("cols"=>60),
		"val"   => $parx["notes"]
		 );

	$formobj->fieldOut( $fieldx );		
	$formobj->close( TRUE );
}

function form2(&$sql) {
	
	
	
	$parx = $this->parx;
	
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Lock objects and add log";
	$initarr["submittitle"] = "Do it!";
	$initarr["tabwidth"]    = "AUTO";

	$hiddenarr = NULL;
	$hiddenarr["t"] = $this->tablename;
	$formobj = new formc($initarr, $hiddenarr, 1);
	$formobj->addHiddenParx( $parx );
	
	$formobj->close( TRUE );
}

function _lineinfo( 
	&$sqlo, 
    $objid, 
	$statusflag, // 0 : ok, 1:error, 2:warn
  	$manirights, 
  	$errtxt, 
    $info_txt,
	$lastAccLog  // ID
	 ) {
	
	if ( !$this->table_flag) {
		$this->table_flag=1;
		$this->tabobj = new visufuncs();
		$headOpt = array( "title" => "Object info");
		$headx  = array ("Object", "Current AccLog", "Error", "Info");
		$this->tabobj->table_head($headx,  $headOpt);
	}
	
	$exptxt = fObjViewC::bo_display( $sqlo, $this->tablename, $objid );
	$statusimg="";
	if ($statusflag==1) $statusimg="<img src=\"images/i13_err.gif\"> ";
	if ($statusflag==2) $statusimg="<img src=\"images/i13_warning.gif\"> ";
	$manirightsOut="";
	if ( $manirights>0 ) {
		$manirightsOut = "<img src=\"images/but.lock.un.gif\">";
	} else {
		$manirightsOut = "<img src=\"images/but.lock.in.gif\">";
	}
	$lastAccLogTxt="";
	if ($lastAccLog) {
		$lastAccLogTxt = obj_nice_name( $sqlo, "H_ALOG_ACT", $lastAccLog );
	}
	$dataArr = array( $exptxt, $lastAccLogTxt,  $statusimg.$errtxt, $info_txt );
	$this->tabobj->table_row ($dataArr);
	
}

/**
 * reopen objects
 * @param $sqlo
 * @param $sqlo2
 * @return -
 */
function doAll( &$sqlo, &$sqlo2 ) {
	// do all
	global $error;
	$FUNCNAME= "doAll";		
	$loopError = 0;

	
	$sqlsLoop = "SELECT x.".$this->pkname.", x.CCT_ACCESS_ID FROM ".$this->sqlAfter;
	$sqlo2->query($sqlsLoop);
	while ( $sqlo2->ReadRow() ) {
		
		$loopError=0;
		$error_txt="";
		$objid   = $sqlo2->RowData[0];
		$cct_access_id   = $sqlo2->RowData[1];
		
		$this->setObj($objid, $cct_access_id); 
		$this->accessInfo( $sqlo );
		
		if ( !$error->Got(READONLY) AND ($this->go>1) ) $this->accessReopenOne( $sqlo );
		
		if ($error->Got(READONLY))  {
     		$errLast   = $error->getLast();
     		$error_txt = $errLast->text;
			$error->reset();
			$errflag = 1;
			$loopError=1;
		} else {
			if ($this->info["addLogErr"]>0) {
				$loopError = 2;
				$error_txt = "need special ROLE to add an ".$this->CCT_ACCLOG_nice.".";
			}
		}
		$this->_lineinfo( $sqlo, 
			$objid, $loopError, $this->info["hasMani"], $error_txt, 
   			$this->info["inf"], $this->info["lastLog"]);
	}
	
	if ( $this->table_flag) $this->tabobj->table_close();
	
	if ($loopError) {
		 $error->set( $FUNCNAME, 1, "Errors occured." );
	}
}

function paramInfo(&$sqlo) {
	$parx = $this->parx;
	
}

}

// --------------------------------------------------- 
global $error, $varcol;

$error = & ErrorHandler::get();
$sqlo   = logon2( $_SERVER['PHP_SELF'] );
$sqlo2  = logon2( );
if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();

$t    = $_REQUEST["t"];
$parx = $_REQUEST["parx"];
$go   = $_REQUEST["go"];

$tablename			 = $t;


$title       		 = "[ReopenObjects] Reopen set of objects to me";
$title_sh       	  = "ReopenObjects";
$infoarr = NULL; 
$infoarr["title"] = $title;
$infoarr["title_sh"] = $title_sh;
$infoarr["scriptID"] = "glob.objtab.accreopen";
$infoarr["form_type"]= "list";
$infoarr['design']   = 'norm';

$infoarr["obj_name"] = $tablename;
$infoarr["obj_cnt"]  = 1;
$infoarr["icon"]     = "images/but.lock.reopen.gif";
$infoarr["locrow"]   =  array( array("glob.objtab.access.php?t=".$tablename, "AccessInfo") );

$pagelib = new gHtmlHead();
$headarr = $pagelib->startPage($sqlo, $infoarr);
if (!$go) {
	echo "&nbsp;<i>Info: This tool reopens selected objects to you. You must be table-admin.</i><br>";
}


$isbo = cct_access_has2($tablename);
if ( !$isbo ) {
	 $pagelib->htmlFoot("INFO", "This function only works for business objects!");
}


$copt = array ("elemNum" => $headarr["obj_cnt"] );
$listVisuObj = new visu_listC();
$listVisuObj->exitByTest(  $sqlo, $tablename, $copt );

$mainLib = new gObjaccReopenLiGui( $sqlo, $tablename, $go, $parx);
echo "<br>";

if (!$go) {

	$mainLib->formAsk($sqlo, $sqlo2);
	htmlFoot("<hr>");
}

$mainLib->paramInfo($sqlo);

if ($go==1) {
	$mainLib->form2($sqlo);
}

$mainLib->doAll($sqlo, $sqlo2);
echo "<br>";
$error->printAll();

$pagelib->htmlFoot();
