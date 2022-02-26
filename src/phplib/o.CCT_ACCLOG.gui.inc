<?php
require_once ('o.CCT_ACCLOG.subs.inc');
require_once  'o.CCT_ACCLOG.wfl.inc';
require_once ("visufuncs.inc");
require_once ('f.signature.sub.inc');
require_once ('func_form.inc');

/**
 * access log (CCT_ACCLOG) GUI function
 * @package o.CCT_ACCLOG.gui.inc
 * @swreq UREQ:0000898
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/phplib/o.CCT_ACCLOG.gui.inc 59 2018-11-21 09:04:09Z $
 */
class oAccLogGuiC {

var $tablename;
var $withProblems; // show audit problems ?
var $outFormat;    // output format: ['html'], 'text'
private $parx;     // input array; see glob.obj.acclock.php:acclock_parx_STRUCT

function __construct($tablename=NULL) {
	$this->accLogLib = new oAccLogC();
	$this->tablename = $tablename;
	$this->outFormat = 'html';
	
	$this->fSignLib  = new fSignatureSub();
	$this->accLogWfl = new oAccLogWfl($tablename);
	
}

function setObject( &$sql, $tablename, $id ) {
    $this->tablename = $tablename;
    $this->id = $id;
    $this->accLogLib->setObject( $sql, $tablename, $id );
    $this->accLogWfl = new oAccLogWfl($tablename);

}

/**
 * check, if tool is allowed to perform status change
 * @swreq SREQ:0000704:20 sperre bestimmte Kombinationen TABLE,STATUS f�r diese Tools;
 * @param  $sqlo
 * @param  $toolname; e.g. glob.obj.acclock.php, glob.objtab.acclock.php
 * @param  $statusid
 * @return array ('ok'=>1,-1 ,'info'=>text)
 */
function checkToolAllowed(&$sqlo, $toolname, $statusid) {
    
    if ($this->tablename==NULL) {
        throw new Exception('Class not initialized: tablename missing');
    }
    
	if (!$statusid) return;
	
	$retarr = array('ok'=>1 ,'info'=>NULL);
	
	$ACT_name = oH_ALOG_ACT_subs::statusID2Name( $sqlo, $statusid );
	
	switch ($this->tablename) {
		case 'ABSTRACT_SUBST':
			if ($toolname=='glob.obj.acclock.php' or $toolname=='glob.objtab.acclock.php') {
				if ($ACT_name=='released' or $ACT_name=='D_released') {
					$retarr = array('ok'=>-1 ,'info'=>'Bitte nutze fuer diesen Status&uumlbergang das Tool: 
						"MAA_Spec_Release".');
				}
			}
			break;
	}
	
	return $retarr;
}

/**
 * define output format
 * @param string $format
 */
function setOutputFormat($format) {
	if ($format==NULL) {
		sys_error_my( 'input param missing' );
	}
	$this->outFormat = $format;
}



function actionsExist(&$sql) {
    if (!$this->id) {
        sys_error_my( 'Object not inizialized,' );
    }
	return ( $this->accLogLib->actionsExist($sql) );
}

function getAccLogInfo($tablename) {
	$info = "An 'object status log' entry can only be added,".
		" if the object is LOCKED and the user has the function-role-right 'o.".$tablename.".acclog'.";
	return ($info);
}

/**
 * get one row
 * @param $sqlo
 * @param $loopdat
 * @param array $option
 * 	 'dateSlim' => 
 * 		0: normal: YYYY-MM-DD HH:MM:SS
 * 		1: slim format: YYYY-MM-DD
 * @global $this->outFormat
 */
function _getOneAuditRow(&$sqlo, &$loopdat, $option=NULL) {
	
	$userFeats = glob_elemDataGet3( $sqlo, "DB_USER", array("DB_USER_ID"=>$loopdat["DB_USER_ID"]), 
		array('NICK', 'FULL_NAME') ); 
	$loopdat["user"] = $userFeats['FULL_NAME'];
	if ($loopdat["user"]==NULL) $loopdat["user"] ='[ID:'.$loopdat["DB_USER_ID"].']'; // fall back
	$action = oH_ALOG_ACT_subs::statusID2Name( $sqlo, $loopdat["H_ALOG_ACT_ID"] );
	
	$loopdat["action"] = $action;
	if ($loopdat["QM_VAL"]>0 and $loopdat["QM_VAL"]<1.0) {
		// for $this->outFormat=='text': no graphics output!
		if ($this->outFormat=='html') $loopdat["QM_VAL"]=$loopdat["QM_VAL"].' <img src="images/i13_warning.gif">';
	}
	
	$dateStr = $loopdat["ACT_DATE"];
	if ( $option['dateSlim'] ) $dateStr = substr($loopdat["ACT_DATE"],0,10);
	
	$dataArr = array( $loopdat["POS"], $loopdat["user"],$loopdat["action"], $dateStr, 
			$loopdat["NOTES"], $loopdat["QM_VAL"] ); 
	if ($this->withProblems) {
		$dataArr[]=$loopdat["PROBLEM"];
	}
	return $dataArr;
}

/**
 * show full audit log table
 * - show full name of users for fullfilling FDA requirements
 * @swreq UREQ:0001676:11.70: Signature/Record Linking, need full name in audit trails !
 * @param array $options
 *   'shProblems' => 0,1 show audit problems
 */
function showLogTable(&$sqlo, &$sqlo2, $options=NULL) {
    if (!$this->id) {
        sys_error_my( 'Object not inizialized,' );
    }
    
	$this->withProblems = 0;
	$cct_access_id = $this->accLogLib->cct_access_id;
	

	$tabobj    = new visufuncs();
	$tablenice = tablename_nice2('CCT_ACCLOG');
	$headOpt = array( "title" => $tablenice );
	$headx   = array ("#", "User", "Action", "Date", "Notes", 'QM');
	if ($options['shProblems']>0) {
		$headx[]='Problems';
		$this->withProblems = 1;
	} 
	$tabobj->table_head($headx,   $headOpt);
	
	$sqls = "select * from CCT_ACCLOG where cct_access_id=".$cct_access_id." order by POS";
	$sqlo2->query($sqls);
	while ( $sqlo2->ReadArray() ) {
		$loopdat = $sqlo2->RowData;
		$dataArr = $this->_getOneAuditRow($sqlo, $loopdat);
		$tabobj->table_row ($dataArr);
	}
	$tabobj->table_close();
}

/**
 * analyse warning; show warning; return 0,1
 * @return int warning exists
 * 	0: nothing
 *  1: info exists
 *  2: warning, errro
 */
static function show_Warning(&$accLogInfo, $infoIsWarning ) {
    if (empty($accLogInfo)) return 0;
    
    $hasWarning=0;
    
    if ($infoIsWarning==1) {
        htmlInfoBox( "Info", implode('<br />',$accLogInfo), "", "INFO" );
        $hasWarning=2;
        
    } elseif ($infoIsWarning==2) {
        htmlInfoBox( "Warning", implode('<br />',$accLogInfo), "", "WARN" );
        $hasWarning=2;
        
    } else {
        htmlInfoBox( "Info", implode('<br />',$accLogInfo), "", "INFO" );
        $hasWarning=1;
    }
    echo "<br />\n";
    return $hasWarning;
}

/**
 * get full audit log table as ARRAY
 * - show full name of users for fullfilling FDA requirements
 * @swreq UREQ:0001676:11.70: Signature/Record Linking, need full name in audit trails !
 * @param array $option
 * 	 'dateSlim' => 0,1
 */
function getLogTable(&$sqlo, &$sqlo2, $option=NULL) {
    if (!$this->id) {
        sys_error_my( 'Object not inizialized,' );
    }
	$cct_access_id = $this->accLogLib->cct_access_id;
	//$tablenice = tablename_nice2('CCT_ACCLOG');
	// $headOpt = array( "title" => $tablenice );
	
	$dataArr = array();
	$headx   = array ("#", "User", "Action", "Date", "Notes", 'QM');
	
	$sqls = "select * from CCT_ACCLOG where cct_access_id=".$cct_access_id." order by POS";
	$sqlo2->query($sqls);
	while ( $sqlo2->ReadArray() ) {
		$loopdat   = $sqlo2->RowData;
		$dataArr[] = $this->_getOneAuditRow($sqlo, $loopdat, $option);
	}
	return array('h'=>$headx, 'd'=>$dataArr);
}

/**
 * get form row by $type
 * @param string $type
 * 	 'signpw'
 *   'repPolIgn'
 *   'qm_val'
 */
function form_getRowByType($type, $value=NULL) {
	switch ($type) {
		case 'signpw':
			$retarr = array ( 
				"title" => "User Password", "name" => "signpw", "val"=>NULL,  "req"=>1,  
				"backcolor"=>"#FFDDDD", "object"=> "password", 'notes'=>'Password for user "'.$_SESSION['sec']['appuser'].'"; needed for FDA 21 CFR Part 11');
			break;
		case 'repPolIgn':
			$retarr = array ( 
				"title" => "Ignore Policy", "name"  => "repPolIgn", "val"   => 0, 
				"object"=> "checkbox",  "notes" => "Ignore problems of the release policy?" );
			break;
		case 'qm_val':
			$qmsel   = array('0.5'=>'0.5', '1'=>'1');
			$retarr  = array ( 
				"title" => "QM value", 
				"name"  => "qm_val", 
				"val"   => 1, 
				"object"=> "select", 
				'inits' => $qmsel, "notes" => "on special case, give a lower value.",
				"rowid" => 'qm_val'
				);
			break;
	}
	return $retarr;
}

/**
 * handle workflow-policy
 * @param string $tmpNewName new status name
 * @param int $reqFlag 0,1
 * @param array $formOpt ()
 *  'currentFlowState'=> actual status for business object, 
 	'flow'  => array()   for serialized workflow,
 	'ignore'=> 0,1 QM ignored the status dependencies
 * @return array $answer
 *    ('ok'=> -1, 1, 'fieldx'=> array() )
 */
private function _workFlowPol(&$sql, $newStatusName, $newStatusID, $reqFlag, $formOpt) {
    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
    
    $currWflState   = $formOpt["currentFlowState"];
    $wfl_output = $this->accLogWfl->workflow_ana($sql, $currWflState, $newStatusID,  $formOpt['flow'], $formOpt);
    if ($wfl_output['ok']<0) return $wfl_output;
    
    
    if ( $_SESSION["userGlob"]["g.debugLevel"]>0 ) {
        
        debugOut('wfl_output: '.print_r($wfl_output,1), $FUNCNAME, 1);
  
    }
    
    $iniarr         = $wfl_output['ok_states'];
    $preSetStatusID = $wfl_output['status.preset'];
    
	$fieldsx = array ( 
		"title" => "Status", 
	    "name"  => "action", 
	    "val"   => $preSetStatusID, 
		"object"=> "select",  
	    "inits"=>$iniarr,  
	    "notes" => "[optional] the status" 
	);
	
	if ( $this->parx['status.mod']<0 ) {
	    // send as hidden param
	    $fieldsx['object'] = 'info2';
	    $fieldsx['inits']  = $iniarr[$preSetStatusID];  // the html text
	   
	}
	
	if ($reqFlag) {
		 $fieldsx["req"]   = 1;
		 $fieldsx["notes"] = "the status";
	}
	
	return array('ok'=>1, 'fieldx'=>$fieldsx);
}

/**
 * show form parameters
 * @param array $parx
   @param int $reqFlag    0,1
   @param array $formOpt 
 	'noReopen'=>1 : removes reopen option...extendible by more parameters to be defined, 
 	'currentFlowState'=>actual status for business object, 
 	'flow'=>array()   for serialized workflow,
 	'ignore'=>QM ignored the status dependencies
 */
function formParams(&$sql, $parx, $reqFlag, $formOpt = NULL) {
    
    if ($this->tablename==NULL) {
        throw new Exception('Class not initialized: tablename missing');
    }
	
	$fields = array();
	$this->parx = $parx;
	
	$newStatusName  = NULL;
	$newStatusID    = $parx["action"];
	if ($newStatusID) $newStatusName  = oH_ALOG_ACT_subs::statusID2Name($sql, $newStatusID);
	
	$wflAnswer = $this->_workFlowPol($sql, $newStatusName, $newStatusID, $reqFlag, $formOpt);
	if ($wflAnswer['ok']>0) {
		$fieldsx = $wflAnswer['fieldx'];
	} else {
		// early break due to workflow problem
		$fields[] =	$wflAnswer['fieldx'];
		return ($fields);
	}
	
	// check, secondSigning
	$needpw = $this->fSignLib->checkSignNeed($sql, $this->tablename);
		
	$fields[] =	$fieldsx;
	$fields[] = array ( 
		"title"   => "Notes", "name"  => "notes", 
	    "val"     => $parx["notes"], 
	    "colspan" => 2,
		"object"  =>  "textarea",
	    "inits"   => array('cols'=>40, 'rows'=>5 )
	    
	);
	
	if ($needpw) {
		$fields[] = $this->form_getRowByType('signpw');
	}
	
	return ($fields);
	
}

/**
 * show form parameters
 * @param array $parx
 @param int $reqFlag    0,1
 @param array $formOpt
 'ignore'=>QM ignored the status dependencies
 */
function reopen_form(&$sql, $parx, $formOpt = NULL) {
    if ($this->tablename==NULL) {
        throw new Exception('Class not initialized: tablename missing');
    }
    
    $fields = array();
    $this->parx = $parx;

    $needpw = $this->fSignLib->checkSignNeed($sql, $this->tablename);

    $fields[] = array (
        "title"   => "Notes", "name"  => "notes",
        "val"     => $parx["notes"],
        "colspan" => 2,
        "object"  =>  "textarea",
        "inits"   => array('cols'=>40, 'rows'=>5 )
    );
    
    if ($needpw) {
        $fields[] = $this->form_getRowByType('signpw');
    }
    
    return ($fields);
    
}


/**
 * function asks trigger values for BO
 * @param $sqlo
 * @param int $cct_access_id CCT_ACCESS_ID of BO
 * @return $trigger array of trigger values id, name, bo trigger is def for, bo ID
 */
function getActTrigger(&$sqlo, $cct_access_id){
    
    $trigger= array();
	$sqltrig = "WFLOW_ID, TABLE_NAME from CCT_ACCESS where CCT_ACCESS_ID = ". $cct_access_id;
	$sqlo->Quesel($sqltrig);
	$sqlo->ReadRow();
	$trigger ["id"] = $sqlo->RowData[0];
	$trigger ["table"] = $sqlo->RowData[1];
	$pkname   = PrimNameGet2($trigger ["table"] );
	$name_col = importantNameGet2($trigger ["table"] );
	$sqlbo = $pkname . ", ".$name_col." from ". $trigger ["table"] . " where CCT_ACCESS_ID = ". $cct_access_id;
	$sqlo->Quesel($sqlbo);
	$sqlo->ReadRow();
	$trigger["bo_id"]  = $sqlo->RowData[0];
	$trigger["bo"]  = $sqlo->RowData[1];
	if ($trigger ["id"] !=""){
		$sqltrig = "NAME from MODULE where MXID = ". $trigger ["id"];
		$sqlo->Quesel($sqltrig);
		$sqlo->ReadRow();
		$trigger["name"]  = $sqlo->RowData[0];
	}
	return $trigger;
	
}

}
