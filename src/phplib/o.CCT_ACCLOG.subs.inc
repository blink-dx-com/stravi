<?php
/**
 * @package o.CCT_ACCLOG.subs.inc
 * @swreq SREQ:0000017: o.CCT_ACCLOG > manage "audit trail" of one object (MOTHER)
 * @swreq UREQ:0000877: g.OBJECT.workflow > concept (Trigger) 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
require_once('insert.inc');
require_once('date_funcs.inc');
require_once ('f.update.inc');
require_once ('o.H_ALOG_ACT.subs.inc');

class oAccLogStaticC {
    
    /**
     * - user can add audit logs to LOCKED object ?
     * - check, if user has role-rights:
     *    - "f.o.".$tablename.".acclog" OR
     *    - 'o.'.$tablename.'.admin'
     * @param $sql
     * @param $tablename
     * @return array($answer, $rightname)
     */
    static function checkManiRole(&$sql, $tablename) {
        $rightname = 'o.'.$tablename.'.acclog';
        $answer  = role_check_f($sql, $rightname);
        if ($answer!='execute') {
            $allowed = role_admin_check ( $sql, $tablename );
            if($allowed>0) $answer='execute';
            $rightname='o.'.$tablename.'.admin';
        }
        return array($answer, $rightname);
    }
}

/**
 * - analysis and manipulation functions for table CCT_ACCLOG
 * - this table represents the AUDIT-TRAIL of an BO
 * - uses audit-state table H_ALOG_ACT
 * - common audit-states: 'released', 'finished', 'reopen'
 * - analyse  table:'GLOBALS', NAME:'pol.objRelease' VALUE:'lab' ???
 * @example get entry of last 'released' action:
 *   $auditLib = new oAccLogC()
 *   $stateid  = oH_ALOG_ACT_subs::getH_ALOG_ACT_ID( $sqlo, $table, 'released'); 
 *   $auditLib->setObject( $sqlo, $tablename, $id);
 *   $entry    = $auditLib->getLastEntryOfAction($sqlo, $stateid); 
 *   
 * @var VARSTRUCT policyAnswerT = array('ok'=>$ok, 'txt'=>$reason, 'qc'=>QC-value)
 * 	 'ok':  
 *   	0 : o.k.  
 * 		<0: error
 *   'qc': float: 0, 0.5, 1
 * 
 */
class oAccLogC {
	var $CLASSNAME  = 'oAccLogC';
	
	var $cct_access_id;
	var $id;
	var $tablename;    // table of object to analyse
	
	var $infox;        // array of  info/warning strings  of the class
	var $infoxIsWarning; // 0, 1, 2 : the $infox array is a warning, else just info-test
	var $stateCache;   // cached array H_ALOG_ACT of ID=>NAME
	var $stateCacheID; // cached array H_ALOG_ACT of NAME=>ID
	
function __construct() {

    
	$this->stateCache   = array();
	$this->stateCacheID = array();
	$this->_hasSpecPolicy = NULL;
	$this->_policyLib     = NULL;
	
	$this->tablename = NULL;
	$this->infoxIsWarning=0;
}


private function _initObject($cct_access_id, $tablename, $id) {
    $this->cct_access_id = $cct_access_id;
    $this->tablename     = $tablename;
    $this->id     	     = $id;
    $this->infox = NULL;
    $this->infoxIsWarning=0;
}

/**
 * set object, use this function instead of setAccessID, if you want to modify the audit-log
 * @param $sql
 * @param $tablename (if empty: get from $cct_access_id)
 * @param $id   (if empty: get from $cct_access_id)
 * @param $cct_access_id  (if not given: $tablename and $id must be given)
 * @return -
 */
function setObject( &$sql, $tablename, $id, $cct_access_id=0 ) {
    
    if (!$cct_access_id and !$id) { 
        throw new Exception('Input error: cct_access_id or id must be set!');
    }
    
    if (!$cct_access_id) { // get access_id, if not given
        if ($tablename==NULL or $id==NULL)  throw new Exception('Input error: id and tablename must be set!');
        $pk = PrimNameGet2($tablename);
        $cct_access_id = glob_elementDataGet( $sql, $tablename, $pk, $id, 'CCT_ACCESS_ID' );
    }
    
    if ($tablename==NULL) {
        $tablename = glob_elementDataGet( $sql, 'CCT_ACCESS', 'CCT_ACCESS_ID', $cct_access_id, 'TABLE_NAME');
    }
    if (!$id) {
        $pk = PrimNameGet2($tablename);
        $id = glob_elementDataGet( $sql, $tablename, 'CCT_ACCESS_ID', $cct_access_id, $pk);
    }
    
    $this->_initObject($cct_access_id, $tablename, $id);
}

/**
 * do reopen
 * @param $sqlo
 * @param $parx : addLogSub() params
 */
function do_reopen( &$sqlo,  $parx) {
    if (!$this->cct_access_id) {
        sys_error_my( 'class '.$this->CLASSNAME.' not initialized' );
    }
    $parx["action"]	= oH_ALOG_ACT_subs::getH_ALOG_ACT_ID($sqlo, $this->tablename, 'reopen');
    $this->addLogSub( $sqlo, $parx );
}

/**
 * add log of an object
 * @param $sql
 * @param $tablename
 * @param $id
 * @param $parx : "action", "notes"
 */
function addLogObj( &$sql, $tablename, $id, $parx) {
    $this->setObject( $sql,  $tablename, $id);
    $this->addLogSub( $sql, $parx);
}



/**
 * get all attributes of CCT_ACCESS
 * @global $this->cct_access_id
 * @return array $retarr [NAME]=VALUE
 */
private function getAccessAttribs(&$sqlo) {
    if (!$this->cct_access_id) {
        sys_error_my( 'class '.$this->CLASSNAME.' not initialized' );
    }
    $sqlsel = "* from CCT_ACCESS where CCT_ACCESS_ID=".$this->cct_access_id;
    $sqlo->Quesel($sqlsel);
    $sqlo->ReadArray();
    $retarr = $sqlo->RowData;
    return ($retarr);
}




/**
 * set info log
 * @param int $isWarning 0,1
 */
private function _add2infox($text, $isWarning=0) {
	$this->infox[]=$text;
	if ($isWarning) $this->infoxIsWarning=$isWarning;
}

/**
 * check release policy
 * @param $sqlo
 * @param $statusName
 * @param array $option
 *   'mo.CERT_FLAG' : [1],2,3,4 planned Life Cycle Flag of mother object
 * @return array VARSTRUCT policyAnswerT
 */
private function _checkPolicy(&$sqlo, $statusName, $option=NULL) {
	if ( $this->_policyLib == NULL ) {
		$labFile = $_SESSION['s_sessVars']['AppLabLibDir'] . "/pol.objRelease.inc";
		require_once($labFile);
		$this->_policyLib = new polObjRelease($sqlo);
	}
	
	$chOpt = $option;
	$this->_policyLib->setObject($sqlo, $this->tablename, $this->id);
	$okarr = $this->_policyLib->checkRelease($sqlo, $statusName, $chOpt);
	return ( $okarr );
}

/**
 * do global post AuditLog actions
 * @param  $sql
 * @param  array $postArgu
 *   'H_ALOG_ACT_ID_last' ID of last $H_ALOG_ACT_ID;
	 'new_status_name' =  name of new audit status
 */
private function postAuditAct($sqlo, $postArgu) {
	if ( $this->_policyLib == NULL ) {
		$labFile = $_SESSION['s_sessVars']['AppLabLibDir'] . "/pol.objRelease.inc";
		require_once($labFile);
		$this->_policyLib = new polObjRelease($sqlo);
	}
	
	$this->_policyLib->setObject($sqlo, $this->tablename, $this->id);
	$logarr = $this->_policyLib->postAuditActX($sqlo, $postArgu['new_status_name']);
	
	if (!empty($logarr)) {
		$this->_add2infox('Action after status change...');
		$this->infox = array_merge($this->infox, $logarr);
	}
	
}

// check special policy
public function specialPolicy(&$sqlo) {
    if ( $this->_hasSpecPolicy !== NULL ) return $this->_hasSpecPolicy;
    
    $this->_hasSpecPolicy = -1;
    $val = glob_elementDataGet( $sqlo, 'GLOBALS', 'NAME', 'pol.objRelease', 'VALUE');
    if ($val=='lab') {
        $this->_hasSpecPolicy = 1;
    }
    return ($this->_hasSpecPolicy);
}

// @return array of strings
function getInfo() {
	return $this->infox;
}
function getInfoxWarnFlag() {
	return $this->infoxIsWarning;
}
function getInfoAll() {
    return array(
        'cct_access_id'=>$this->cct_access_id,
        'id'=>$this->id,
        'tablename'=>$this->tablename
        );
}



function hasManiRights( &$sql ) {
	// check manipulation-rights
	return ( access_hasManiRights( $sql, $this->cct_access_id ) );
}



/**
 * actions exist for cct_access_id ?
 * @param  $sql
 * @return : 0 : no
	   1 : yes
 */
function actionsExist(&$sql) {
	$sqls = "select 1 from CCT_ACCLOG where cct_access_id=".$this->cct_access_id;
	$sql->query($sqls);
	if ( $sql->ReadRow() ) return 1;
	else return 0;
	
}


/**
 * get H_ALOG_ACT_ID of current entry in CCT_ACCESS
 * @global $this->cct_access_id
 */
function getLastLog(&$sql) {
	if (!$this->cct_access_id) {
		sys_error_my( 'class '.$this->CLASSNAME.' not initialized' );
	}
	$sqls = "select H_ALOG_ACT_ID from CCT_ACCESS where CCT_ACCESS_ID=".$this->cct_access_id;
	$sql->query($sqls);
	$sql->ReadRow();
	$retid = $sql->RowData[0];	  
	return ($retid);
}



/**
 * get NAME of current audit status
 */
function getLastLogName(&$sqlo) {
	$statusid = $this->getLastLog($sqlo);
	if (!$statusid) return;
	$name = oH_ALOG_ACT_subs::statusID2Name( $sqlo, $statusid );
	return ($name);
}

/**
 * - is the current status == 'releaased' ?
 * @param $sqlo
 * @return boolean
 */
function currentIsReleased( &$sqlo ) {
	 $statusID = $this->getLastLog($sqlo);
	 if (!$statusID) return 0;
	 $stateRel_ID = oH_ALOG_ACT_subs::getH_ALOG_ACT_ID( $sqlo, $this->tablename, 'released' );
	 if ($statusID==$stateRel_ID) { // is released ?
	 	return 1;
	 } else {
	 	return 0;
	 }
}

/**
 * - get all infos of the LAST entry CCT_ACCLOG
 * @param $sqlo
 * @return array $infoArr (KEY=>VAL) KEY: original column names
 */
function getLastEntry(&$sqlo) {
	if (!$this->cct_access_id) {
		sys_error_my( 'class '.$this->CLASSNAME.' not initialized' );
	}

	$sqlsel = "* from CCT_ACCLOG where CCT_ACCESS_ID=".$this->cct_access_id.
		" order by POS DESC";
	$sqlo->Quesel($sqlsel);
	$sqlo->ReadArray();
	$infoArr = $sqlo->RowData;
	return ($infoArr);
}

/**
 * - get all infos of the last released-state
 * - search for the LST released-state (does not have to be the current one)
 * @param $sqlo
 * @return array $infoArr
 */
function getLastReleased(&$sqlo) {
	if (!$this->cct_access_id) {
		sys_error_my( 'class '.$this->CLASSNAME.' not initialized' );
	}
	
	$stateid = oH_ALOG_ACT_subs::getH_ALOG_ACT_ID( $sqlo, $this->tablename, 'released' );
	 
	$sqlsel = "* from CCT_ACCLOG where CCT_ACCESS_ID=".$this->cct_access_id. 
			" and H_ALOG_ACT_ID= ".$stateid." order by POS DESC";
	$sqlo->Quesel($sqlsel);
	$sqlo->ReadArray();
	$infoArr = $sqlo->RowData;	
	return ($infoArr);
}

/**
 * - get all infos of the LAST entry with $stateid
 * @param $sqlo
 * @return array $infoArr
 */
function getLastEntryOfAction(&$sqlo, $stateid) {
	if (!$this->cct_access_id) {
		sys_error_my( 'class '.$this->CLASSNAME.' not initialized' );
	}
	
	if (!$stateid) return;
	 
	$sqlsel = "* from CCT_ACCLOG where CCT_ACCESS_ID=".$this->cct_access_id. 
			" and H_ALOG_ACT_ID= ".$stateid." order by POS DESC";
	$sqlo->Quesel($sqlsel);
	$sqlo->ReadArray();
	$infoArr = $sqlo->RowData;	
	return ($infoArr);
}

/**
 * - get all entries (POS, H_ALOG_ACT_ID) of one CCT_ACCESS_ID, order by POS
 * @param $sqlo
 * @return array $infoArr[POS] = H_ALOG_ACT_ID
 */
function getAllEntries(&$sqlo) {
	if (!$this->cct_access_id) {
		sys_error_my( 'class '.$this->CLASSNAME.' not initialized' );
	}
	
	$infoArr = array();
	
	$sqlsel = "POS, H_ALOG_ACT_ID from CCT_ACCLOG where CCT_ACCESS_ID=".$this->cct_access_id. 
			" order by POS";
	$sqlo->Quesel($sqlsel);
	while ($sqlo->ReadRow() ) {
		$infoArr[$sqlo->RowData[0]] =  $sqlo->RowData[1];	
	}
	return ($infoArr);
}

/**
 * - get all entries POS=>array( H_ALOG_ACT_ID, DB_USER_ID, DATE ) of one CCT_ACCESS_ID, order by POS
 * @param $sqlo
 * @return array $infoArr[POS] = array(KEY=>VAL)
 */
function getAllEntriesLong(&$sqlo) {
	if (!$this->cct_access_id) {
		sys_error_my( 'class '.$this->CLASSNAME.' not initialized' );
	}
	
	$infoArr = array();
	
	$sqlsel = "* from CCT_ACCLOG where CCT_ACCESS_ID=".$this->cct_access_id. 
			" order by POS";
	$sqlo->Quesel($sqlsel);
	while ($sqlo->ReadArray() ) {
		$infoArr[$sqlo->RowData['POS']] =  $sqlo->RowData;	
	}
	return ($infoArr);
}

/**
 * check release policy
 * @param $sqlo
 * @param array $option
 *   'mo.CERT_FLAG' : [1],2,3,4 planned Life Cycle Flag of mother object
 * @return array
 */
function checkReleasePolicy(&$sqlo, $statusname, $option=NULL) {
	if ( $this->specialPolicy($sqlo)>0 ) {
		$retarr    = $this->_checkPolicy($sqlo, $statusname, $option);
		return $retarr;
	}
	return;
}

// 
/**
 * RAW CCT_ACCLOG insert, no checks, no triggers
 * @param object $sqlo
 * @param array $argu -- argu of CCT_ACCLOG
 * @return number
 */
function addLog_ACCESS(object $sqlo, array $argu) {
    $argu['CCT_ACCESS_ID'] = $this->cct_access_id; # set the right CCT_ACCESS !
    $insopt   = array("types"=>array('ACT_DATE'=>'TIME'));
    insert_row_s( $sqlo, "CCT_ACCLOG", $argu, $insopt );
}

function addLog_RAW($sqlo, $argu) {

    if ( !$this->cct_access_id or $this->tablename==NULL ) {
        sys_error_my( $FUNCNAME.' not initialized.' );
    }
    $xcctuser_id  = $_SESSION['sec']['db_user_id'];
    
    if (!$argu['H_ALOG_ACT_ID']) {
        throw new Exception('Input missing: H_ALOG_ACT_ID.');
    }
   
    $argu=NULL; // argus for INSERT
    $argu['CCT_ACCESS_ID']=$this->cct_access_id;
    $argu['DB_USER_ID']	  =$xcctuser_id;
    $argu['ACT_DATE']	  =time();
    addLog_ACCESS($sqlo, $argu);
}


/**
 * - if trigger exists: pre-check
 * - add log
 * - no access-check !
 * - do _checkPolicy, if a policy exists
 * - also user root must fulfill the policy
 * - if trigger exists: post-check
 * @param $sql
 * @param $parx 
 * 	"action" : action-id
	"notes"  : can be NULL
 *	'repPolIgn'  : 0,1 ignore policy, the calling module must test the right to to this!
 *	'lastStatIgn': 0,1 ignore, if last status was the same like the new status
 *				useful for status 'assigned'
 *	'qm_val' : if 'repPolIgn' set, give QM_VAL
 *	'mo.CERT_FLAG' : [1],2,3,4 planned Life Cycle Flag of mother object
 *	'DB_USER_ID_set' [OPTIONAL] : alternativ $argu['DB_USER_ID']
 * @swreq SREQ:0000017:004: manage adding an audit status to the object
 * @throws exception, ERROR-ID: 
 * 	 22 - set state "released" not possible
 *   30 - status-check before setting the new status: failed
 */
function addLogSub( &$sql, $parx ) {
	$FUNCNAME="addLogSub"; // please do NOT rename this, because it is asked in error-handling
	global  $error;
	
	$xcctuser_id  = isset($parx['DB_USER_ID_set']) ? $parx['DB_USER_ID_set'] : $_SESSION['sec']['db_user_id'];
	
	
	if ( !$this->cct_access_id or $this->tablename==NULL ) {
		sys_error_my( $FUNCNAME.' not initialized.' );
	}
	
	$ignoreLastStatus = $parx['lastStatIgn']>0 ? 1 : 0;
	$accessAttribs    = $this->getAccessAttribs($sql);
	$H_ALOG_ACT_ID_last = $accessAttribs['H_ALOG_ACT_ID'];
	$wflow_id         = $accessAttribs['WFLOW_ID'];
	$newStatusName    = oH_ALOG_ACT_subs::statusID2Name( $sql, $parx["action"] );
	
	
	$argu=NULL; // argus for INSERT
	$argu['CCT_ACCESS_ID']=$this->cct_access_id;
	$argu['DB_USER_ID']	  =$xcctuser_id;
	$argu['ACT_DATE']	  =time();
	$argu['H_ALOG_ACT_ID']=$parx["action"];
	$argu['NOTES']		  =$parx["notes"];
	
	$workclass=NULL; // workflow class
	
	if ($wflow_id) {
		// @swreq SREQ:0000017:001: manage workflows
		// init trigger
		$wflArgu = $argu; // extend arguments for trigger
		$wflArgu['H_ALOG_ACT_ID_last'] = $H_ALOG_ACT_ID_last;
		$wflArgu['tablename'] = $this->tablename;
		$wflArgu['wflow_id']	    = $wflow_id;
		$wflArgu['new_status_name'] = $newStatusName;
		if ($H_ALOG_ACT_ID_last) {
		    $wflArgu['old_status_name'] = oH_ALOG_ACT_subs::statusID2Name($sql, $wflArgu['H_ALOG_ACT_ID_last'] );
		}
	
		require_once('gStatusTriggerAbs.inc');
		$wfstart = new gStatusTriggerManage();
		$classname = $wfstart->dynamicinclude( $sql, $wflArgu, $wflow_id );
		if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 2, 'Error got for init of trigger, calling module '.$wflow_id );
			return;
		}
		
		$workclass = new $classname;
		$workclass->init($wflArgu);
		
		// pre trigger (before status change)
		$workclass->beforeStatus( $sql );
		if ($error->Got(READONLY))  {
			// @swreq:0001112: g.OBJECT.workflow > beforeStatus() check 
			$errLast   = $error->getLast(NULL,0, READONLY);
			$error_txt = $errLast->text;
			$error_id  = $errLast->id;
			if ($error_id<100 and glob_isAdmin() and 
				strstr($_SESSION["userGlob"]["g.debugKey"], 'trigger.err.ignore')!=NULL 
				) {
				$error->reset();
				$this->_add2infox('workflow.beforeStatus-error has been ignored by user! Error was:'.$error_txt,1 );
			} else {
				$error->set( $FUNCNAME, 30, 'Error got on trigger:beforeStatus (module '.$wflow_id.
					'); root can ignore this. see online help.' );
				return;
			}
		}
	}
	
	
	$QM_VAL=NULL;
	$policyAnswer=NULL;
	
	if ( $parx['action'] ) {
		if ( $this->specialPolicy($sql)>0) {
			$ignorePol = $parx['repPolIgn'];
			
			$cheOpt=NULL;
			if ($parx['mo.CERT_FLAG']) $cheOpt = array( 'mo.CERT_FLAG'=>$parx['mo.CERT_FLAG'] );
			$policyAnswer    = $this->_checkPolicy($sql, $newStatusName, $cheOpt);
			$QM_VAL    = $policyAnswer['qc']; // get QC-value from policy
			
			if ( $policyAnswer['ok']<0  ) {
				if ( !$ignorePol ) { // ignore policy problem ?
					$error->set( $FUNCNAME, 22, 'set state "'.$newStatusName.'" not possible due to the QM-policy.<br />'.
				 		'Info: val:'.$policyAnswer['ok'].':'.$policyAnswer['txt'] );
					return;
				} else {
					// only warning
					$this->_add2infox('QC-policy warning has been ignored by user!',1);
					$QM_VAL = $parx['qm_val'];
				}
			} else {
				if ($QM_VAL==NULL) $QM_VAL = $parx['qm_val'];
			}
			
		}
	}
	
	$releaseId = oH_ALOG_ACT_subs::getH_ALOG_ACT_ID( $sql, $this->tablename, 'released' );
	if ( $releaseId == $parx['action']  ) {
		if ($QM_VAL>0.45 and $QM_VAL<0.55 and $parx["notes"]==NULL) {
			$error->set( $FUNCNAME, 101, 'you must give "notes", because of "special release": QC_test-value='.$QM_VAL );
			return;
				
		}
	}
	
	
	if ( $H_ALOG_ACT_ID_last AND $parx["action"]==$H_ALOG_ACT_ID_last and !$ignoreLastStatus) {
		  $error->set( $FUNCNAME, 1, "last entry in CCT_ACCLOG has already the status H_ALOG_ACT_ID=".$parx["action"] );
		  return;
	}

	
	if ($QM_VAL>0)       $argu['QM_VAL']  = $QM_VAL; // QM was calculated at the end of function; save only values > 0
	
	// @swreq SREQ:0000017:005: save the policy-problems
	if ($policyAnswer['ok']<0) {
		// test, if string is too long
		$PROB_MAX_LEN = 250; // max len of problem to save
		$tmptxt = $policyAnswer['txt'];
		if (strlen($tmptxt)>$PROB_MAX_LEN) $tmptxt = substr($tmptxt,0,$PROB_MAX_LEN).'...';
		$argu['PROBLEM'] = $tmptxt; 
	}
	
	$this->addLog_ACCESS($sql, $argu);

	// post global action for; may be used in the FUTURE!
	if ( $parx['action'] and  $this->specialPolicy($sql)>0 ) {
		$postArgu = array(); // extend arguments
		$postArgu['H_ALOG_ACT_ID_last'] = $H_ALOG_ACT_ID_last;
		$postArgu['new_status_name'] = $newStatusName;
		$this->postAuditAct($sql, $postArgu);
	}
	
	if ($wflow_id) { // post trigger
		$workclass->OnTrigger($sql, $wflArgu);
	
		$logarr = $workclass->getlog();
		if (!empty($logarr)) {
			$this->_add2infox('A trigger has been started. Messages:');
			$this->infox = array_merge($this->infox, $logarr);
		}
		if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 3, 'Error got for trigger, calling module '.$wflow_id );
			return;
		}
		
		$mx_warn_flag = $workclass->has_warning();
		if ($mx_warn_flag) {
		    // inherit warning status ...
		    $this->infoxIsWarning = $mx_warn_flag;
		}
	}
	
}	

/**
 * update an entry
 *  - auto update date
 * @param object $sqlo
 * @param array $entryData
 * 	'DB_USER_ID'
 *  'NOTES'
 */
public function updateEntry(&$sqlo, $pos, $entryData) {
	if (!$this->cct_access_id) {
		sys_error_my( 'class '.$this->CLASSNAME.' not initialized' );
	}
	
	$entryData['ACT_DATE'] = date_unix2datestr( time(), 1);
	
	// ... not allowed
	if (isset($entryData['POS'])) unset($entryData['POS']);
	if (isset($entryData['CCT_ACCESS_ID'])) unset($entryData['CCT_ACCESS_ID']);
	
	$idarr  = array('CCT_ACCESS_ID'=>$this->cct_access_id, 'POS'=>$pos);
	
	gObjUpdate::update_row_s ( $sqlo, 'CCT_ACCLOG', $entryData, $idarr );
}

/**
 * remove an entry
 * @param object $sqlo
 * @param array $entryData
 * 	'DB_USER_ID'
 *  'NOTES'
 */
public function removeEntry(&$sqlo, $pos) {
	if (!$this->cct_access_id) {
		sys_error_my( 'class '.$this->CLASSNAME.' not initialized' );
	}
	$sqlo->Deletex('CCT_ACCLOG','CCT_ACCESS_ID='.$this->cct_access_id.' and POS='.$pos);
}

/**
 * get detailed  info for a planned status
 * @param $sqlo
 * @param $statusname e.g. 'released'
 * @return array('ok'=>status, 'txt'=>text)
 */
function getUpfrontInfo(&$sqlo, $statusname) {
	$retarr = array(1,NULL);
	if ( $this->specialPolicy($sqlo)>0 ) {
		$retarr = $this->_checkPolicy($sqlo, $statusname);
	}
	return $retarr;
}


	
}
