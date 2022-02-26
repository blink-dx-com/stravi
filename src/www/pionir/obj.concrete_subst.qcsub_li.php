<?php
/**
 * [QC_checker:substance_concrete] Check QC-things for a list of CONCRETE_SUBST
 * TBD: $this->actCnt++; -- changed substances	   		
 * @package obj.concrete_subst.qcsub_li.php
 * @version0 2008-04-01
 * @swreq UREQ:0002055 o.CONCRETE_SUBST > Tool QC_checker, mehrere Substanzen
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param  $go
  		   $parx["action"] 
  		   		= ["view"], 
		   		"autofini" -- autofinish
			 
 */
session_start(); 


require_once ("reqnormal.inc"); // includes all normal *.inc files
require_once ("visufuncs.inc");
require_once ("sql_query_dyn.inc");
require_once ("f.visu_list.inc");

require_once ('access_lock.inc');
require_once ('o.CCT_ACCLOG.subs.inc');
require_once ('o.CONCRETE_SUBST.qcInv.inc');
require_once ('f.msgboxes.inc'); 

class oConc_substQcsub_li {
var $statsArr = NULL; // "error"

function __construct(
		&$sqlo,
		$go,
		$parx,
		$option=NULL // "output" => 0 - silent
					 // 			1 - details
		) {
	$this->go = $go;
	$this->parx = $parx;
	$this->opt =$option;
	$this->statsArr = NULL; 
	$this->statsArr["error"] = 0;
	
	$this->substQcInvlib= new oConcSubstQcInv($sqlo, $go); 
	
	$tablename="CONCRETE_SUBST";
	$sqlopt=array();
	$sqlopt["order"] = 1;
	$this->sqlAfter  = get_selection_as_sql( $tablename, $sqlopt);
	$this->table_flag=0;
	
	$this->statusColor = array(
		"released" =>"#008000",
		"finished"=>"#808080",
		);
}	

function init( &$sqlo ) {
	global $error;
	$FUNCNAME= "init";
	
	if ( $this->parx["action"]=="autofini" ) {
		$this->actCnt = 0;
		echo "Action: <b>Set status from 'Released' to 'Finished' for expired substances</b><br><br>\n";
		
		$this->substQcInvlib->initAutoFini($sqlo);
		
		if ( !$this->go ) {
			$this->_formAutoFini();
		} else {
			echo "<b>Do Auto-finish NOW!</b><br>";
		}
	}
	
}

function _formAutoFini() {
	require_once ('func_form.inc');
	
	
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Prepare Auto-Finish";
	$initarr["submittitle"] = "Do Auto-Finish";
	$initarr["tabwidth"]    = "AUTO";

	$hiddenarr = NULL;
	$hiddenarr["parx[action]"]     = "autofini";
	$formobj = new formc($initarr, $hiddenarr, 0);
	$formobj->close( TRUE );
	echo "<br>";
}

function _getUseIcon($qcproblem, $statName) {

	$usableIcon = "i13_nouse.gif";
	
	if (!$qcproblem AND $statName=="" ) {
		$usableIcon = "i13_ok2.gif";
		return ($usableIcon);
	}
	
	if ($qcproblem>0 AND $statName=="released" ) {
		$usableIcon = "i13_err.gif";
		return ($usableIcon);
	}
	if (!$qcproblem AND $statName=="released" ) {
		$usableIcon = "i13_ok.gif";
	}
	return ($usableIcon);
	
}

function _singleCheck( &$sqlo, $substid, $acc_id ) {
	// check one substance
	//
	global $error;
	$FUNCNAME= "_singleCheck";
	
	$sqls = "select c.NAME, a.NAME from CONCRETE_SUBST c".
			" JOIN ABSTRACT_SUBST a ON c.ABSTRACT_SUBST_ID=a.ABSTRACT_SUBST_ID ".
			" where c.CONCRETE_SUBST_ID=".$substid;
	$sqlo->query($sqls);
	$sqlo->ReadRow();
	$feat1   = $sqlo->RowData;
	
	$this->substQcInvlib->setSubst( $sqlo, $substid );		
	$retarr = $this->substQcInvlib->singleCheck( $sqlo );
	if ($error->Got(READONLY))  {
		$errLast   = $error->getLast();
		$error_txt = $errLast->text;
		$error->reset();
		$this->statsArr["error"] = $this->statsArr["error"]+1;
	}
	if ($retarr["qcproblem"]>0) $this->statsArr["qcproblem"] = $this->statsArr["qcproblem"] +1;
	
	$manirights = 	$retarr["manirights"];
	$CCT_ACCLOG_stat = $retarr["ACCLOG_stat"];
	$qcinfo 	= $retarr["qcinfo"];
	$actAnswer 	= $retarr["actAnswer"];
	
	if ( $actAnswer>0 ) $this->actCnt++;
	
	if ( $manirights>0 ) {
		$manirightsOut = "<img src=\"images/but.lock.un.gif\">";
	} else {
		$manirightsOut = "<img src=\"images/but.lock.in.gif\">";
	}
	$CCT_ACCLOG_statN = "";
	if ($CCT_ACCLOG_stat) {
		$statName = obj_nice_name( $sqlo, "H_ALOG_ACT", $CCT_ACCLOG_stat );
		$colorx = $this->statusColor[$statName];
		if ( $colorx!="" ) {
			$CCT_ACCLOG_statN = "<font color=".$colorx.">".$statName."</font>";
		} else $CCT_ACCLOG_statN = $statName;
	}
	
	$usableIcon = "<img src=\"images/".
		$this->_getUseIcon($retarr["qcproblem"], $statName)."\" hspace=3>"; 
	$featAcc = access_data_getai( $sqlo, $acc_id, 3 );
	
	$infoarr = array( 
		"<a href=\"edit.tmpl.php?t=CONCRETE_SUBST&id=".$substid."\">".$substid."</a>", 
	  	$feat1[0], 
		$feat1[1], 
		$featAcc["crea_date"], 
		$usableIcon,
		$manirightsOut, 
		$CCT_ACCLOG_statN, 
		$qcinfo 
		);
		
	
	if ($actAnswer!="") $infoarr[] = $actAnswer;
	if ($error_txt!="") $infoarr[] = "<font color=red>Error</font> ".$error_txt;
	
	$rowshow = 1;
	if ( $this->parx["action"]=="autofini" ) {
		if ( $actAnswer==0 AND $error_txt=="" ) {
			$rowshow = 0; // only show interesting ROWs
		}
	}
	
	if ( $rowshow ) {
		
		if ( !$this->headshown ) $this->_table_head();
		$this->tabobj->table_row ($infoarr);
		$this->headshown = 1;
	}
	$this->sumarr[$CCT_ACCLOG_stat] =  $this->sumarr[$CCT_ACCLOG_stat]+1;
}

function _table_head() {

	//$sqlAcc = $this->accLogState["released"]
	//$searchcond = " AND ".$sqlAcc;
	// echo "Filter: [<a href=\"view.tmpl.php?tableSCond=".$searchcond."&searchOp_i=AND\">Only released</a>]";
	$this->tabobj = new visufuncs();
	$headOpt = array( "title" => "QC list");
	$headx  = array (
			"ID", 
			"Name", 
			"Abstract", 
			"Crea_date", 
			"Use",
			"<img src=\"images/but.lock.in.gif\" title=\"Lock status\">", 
			"Status", "QC_info");
	if ( $this->tableInfoCol ) {
		$headx[] = "Info";
	}
	$this->tabobj->table_head($headx,   $headOpt);
}

function doCheck( &$sqlo, &$sqlo2 ) {
	global $error;
	$FUNCNAME= "doCheck";
	
	$this->tableInfoCol=0;
	if ( $this->parx["action"]=="autofini" ) {
		$this->tableInfoCol=1;
	}
	
	$this->headshown = 0;
	$loopError = 0;
	$this->sumarr=NULL;
	
	$sqlsLoop = "SELECT x.CONCRETE_SUBST_ID, x.CCT_ACCESS_ID FROM ".$this->sqlAfter;
	$sqlo2->query($sqlsLoop);
	$loopcnt=0;
	while ( $sqlo2->ReadRow() ) {
		
		$substid = $sqlo2->RowData[0];
		$acc_id  = $sqlo2->RowData[1];
		
		$this->_singleCheck( $sqlo, $substid, $acc_id );
		
		if ($error->Got(READONLY))  {
     		$errLast   = $error->getLast();
     		$error_txt = $errLast->text;
			$error->reset();
			$errflag = 1;
			$loopError=1;
		}
		$loopcnt++;
	}
	
	
	if ( $this->headshown ) $this->tabobj->table_close();
	echo "<br>";
	
	if ($loopError) {
		 $error->set( $FUNCNAME, 1, "Errors occured." );
	}
	
	if ( $this->parx["action"]=="autofini" ) {
		echo "Analysed objects: <b>".$loopcnt."</b><br>\n";
		if (!$this->go) echo "objects to be touched:  <b>".$this->actCnt."</b><br>\n";
		else  echo "Touched objects:  <b>".$this->actCnt."</b><br>\n";
		if ($this->statsArr["error"]) echo "<font color=red>Errors:</font> <b>". 
			$this->statsArr["error"]."</b><br>";
	}
	if ($this->statsArr["qcproblem"]) echo "<b><font color=red>QC-Problems:</font></b> <b>". 
			$this->statsArr["qcproblem"]."</b><br>";
}

function summary(&$sqlo) {
	
	$tabobj = new visufuncs();
	$headOpt = array( "title" => "Status Summary" );
	$headx  = array ("Status", "number of objects");
	$tabobj->table_head($headx,   $headOpt);
	
	foreach( $this->sumarr as $key=>$val) {
		$niceer = "";
		if ($key) $niceer = obj_nice_name ($sqlo, "H_ALOG_ACT", $key );
		$dataArr=array( $niceer, $val);
		$tabobj->table_row ($dataArr);
	}
	reset ($this->sumarr); 
	
	$tabobj->table_close();
}
	
}

// --------------------------------------------------- 
global $error, $varcol;

$error = & ErrorHandler::get();
$sqlo  = logon2( $_SERVER['PHP_SELF'] ); // URL-link for the first db-login; e.g. for object ($_SERVER['PHP_SELF']."?id=".$id)
$sqlo2 = logon2(  ); 

if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();

$tablename			 = "CONCRETE_SUBST";

$title       		 = "[QC_checker:substance_concrete]";
$infoarr = NULL; 
$infoarr["title"] = $title;
$infoarr["scriptID"] = "obj.exp.l_qccheck";
$infoarr["form_type"]= "list";

$infoarr["obj_name"] = $tablename;
$infoarr["obj_cnt"]  = 1;
$infoarr['help_url'] = "o.CONCRETE_SUBST.qccheck.html";

$pagelib = new gHtmlHead();
$headarr = $pagelib->startPage($sqlo, $infoarr);
echo " [<a href=\"glob.objtab.acclock.php?t=".$tablename."\">LockObjects</a>]";
if ( $_REQUEST["parx"]["action"]== "autofini" ) 
	echo " [<a href=\"".$_SERVER['PHP_SELF']."\">QC_checker</a>] <b>Auto-Finish</b>";
else
	echo " <b>QC_checker</b> [<a href=\"".$_SERVER['PHP_SELF']."?parx[action]=autofini\">Auto-Finish</a>]";
	
echo "<br>";
echo "<ul>\n";

gHtmlMisc::func_hist( $infoarr["scriptID"], $title, $_SERVER['PHP_SELF'] );

// check TABLE selection
$listVisuObj = new visu_listC();		
$copt = array ("elemNum" => $headarr["obj_cnt"] ); // prevent double SQL counting
list ($stopFlag, $stopReason)= $listVisuObj->checkSelection( $sqlo, $tablename, $copt );
if ( $stopFlag<0 ) {
    htmlFoot("Attention", $stopReason);
}

$mainLib = new oConc_substQcsub_li( $sqlo, $_REQUEST["go"], $_REQUEST["parx"] );
$mainLib->init( $sqlo );
if ($error->Got(READONLY))  {
     $error->printAll();
	 htmlFoot();
}
$mainLib->doCheck( $sqlo, $sqlo2 );
echo "<br>";
if ($error->Got(READONLY))  {
     $error->printAll();
}

if ( $_REQUEST["go"]==1 ) {
	cMsgbox::showBox("ok", "Ready"); 
	echo "<br>";
    
}

$mainLib->summary($sqlo);

htmlFoot("<hr>");