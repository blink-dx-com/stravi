<?php
// $Header: trunk/src/www/pionir/glob.obj.acclock.php 59 2018-11-21 09:04:09Z $
/**
 * LOCK or "add action" to object, optional: add audit-log-entry
 * REOPEN must use tool PLUGIN/g.obj.reopen.inc
 * 
 * - if object has manipulation-rights: lock the object (remove all access rights)
 * - user can change rights, if has entail right or is table-admin!
 * - optional: add audit-log-entry
 * 
 * - for RELEASE:
 *   - check release-policy
 *   - if this check failed, offer release for role-right: f.g.QM_expert
 *   
 * @package glob.obj.acclock.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq UREQ:0000704: f.glob.obj.acclock.php > lock object and add audit status 
 * @swreq UREQ:0000898: support $action="add" (OLD requirement)
 * @swreq SREQ:0002056: support $action="add" : new action=add policy  
 * @param  
 * $t         : tablename
   $id 		  : ID of object
   $go		  : 0,1
   $parx      : the audit-log parameters  (acclock_parx_STRUCT)
   	'action'  : ID of H_ALOG_ACT (optional: you can also give $parx['statName'] )
   	  or
   	'statName': NAME of H_ALOG_ACT; alternative to $parx['action']; but 'action' rules, if both given
   	'notes'   : notes
   	'repPolIgn' : 0,1 ignore release policy ???
   	'qm_val'  : QM-value: 0.5 , 1
   	'signpw'  : signin password; needed if signing is required
   	'mo.CERT_FLAG' : (optional) [1],2,3,4 Life cycle flag of MOTHER object
   	'status.mod': allow changing H_ALOG_ACT input ?
   	    [1] : yes
   	    -1  : no
   
   $action = 
   	['lock'], : lock object + add action
   	'add'     : only add an action, no LOCK-operation : NOT allowed for 'released'
   	'reopen'  : reopen object
   
   $backurl   : URL-encoded URL to jump back
   $ignore    : 0,1 (QM-ignore) -- TBD: conflict with variable repPolIgn ?

 */


//extract($_REQUEST); 
session_start(); 


require_once ("reqnormal.inc"); // includes all normal *.inc files
require_once ('access_lock.inc');
require_once ('o.CCT_ACCLOG.wfl.inc');
require_once ('o.CCT_ACCLOG.gui.inc');
require_once ('f.signature.sub.inc');



class gObjaccLockGui {
	
	var $cct_access_id; // access_id of object
	var $action; /**
		['lock'], : lock object + add action
   		'add'     : only add an action, no LOCK-operation : NOT allowed for 'released'
   		'reopen'  : reopen
	*/
	var $ACT_name; // name of FORM parameter of H_ALOG_ACT_ID
	
	function __construct(&$sqlo, $tablename, $id, $action, $parx, $ignore, $backurl) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		
		$this->action  = $action;
		$this->parx    = $parx;
		$this->tablename=$tablename;
		$this->id      = $id;
		$this->ignore  = $ignore;
		//$this->dox     = $do; // "lock"
		$this->backurl = $backurl;
		
		$this->userHasAccLogRights = 0;
		$pk = PrimNameGet2($tablename);
		
		$this->infoIsWarning = 0;
		
		$this->lockObj      = new gObjaccLock();
		$this->accLogLib    = new oAccLogC();
		
		if ( $action =='reopen') {
		    $error->set( $FUNCNAME, 4, 'Status-Change for "reopen" is not supported by this tool. Please use the REOPEN-tool.' );
		    return;
		}
		
		if ( !$this->parx['action'] and $this->parx['statName']!=NULL ) {
		    $statsid = oH_ALOG_ACT_subs::getH_ALOG_ACT_ID($sqlo, $tablename, $this->parx['statName']);
			if (!$statsid) {
				$error->set( $FUNCNAME, 1, 'H_ALOG_ACT:NAME '.$this->parx['statName'].' unknown.' );
			 	return;
			}
			$this->parx['action'] = $statsid;
		}
		$this->ACT_name = NULL;
		if ($this->parx['action']>0) {
		    $this->ACT_name = oH_ALOG_ACT_subs::statusID2Name( $sqlo, $this->parx['action'] );
		} 
		
		if ($this->ACT_name=='reopen') {
		    $error->set( $FUNCNAME, 2, 'Status-Change for "reopen" is not supported by this tool. Please use the REOPEN-tool.' );
		    return;
		}
		
		$accLogGuigLib = new oAccLogGuiC($this->tablename);
		$toolallow = $accLogGuigLib->checkToolAllowed($sqlo, 'glob.obj.acclock.php', $this->parx['action']);
		if ($toolallow['ok']<0) {
			$error->set( $FUNCNAME, 3, 'Status "'.$this->ACT_name.'" nicht erlaubt: '. $toolallow['info']);
			return;
		}
		
		
		$this->releaseID = oH_ALOG_ACT_subs::getH_ALOG_ACT_ID( $sqlo, $tablename, 'released' );
		$this->finishID  = oH_ALOG_ACT_subs::getH_ALOG_ACT_ID( $sqlo, $tablename, 'finished' );
		$this->rejectedID= oH_ALOG_ACT_subs::getH_ALOG_ACT_ID( $sqlo, $tablename, 'rejected' );
		if (!$this->rejectedID) $this->rejectedID= -1;
		
		
		
		$this->cct_access_id = glob_elementDataGet( $sqlo, $tablename, $pk, $id, 'CCT_ACCESS_ID');
		$this->QM_role_right = 'g.QM_expert';
		$this->isQM_admin    = role_check_f ( $sqlo, $this->QM_role_right );
		$this->adminErrTxt   = 'You have not the role-right "'.$this->QM_role_right.'" .';
		
		list($exeflag, $rightname) = oAccLogStaticC::checkManiRole($sqlo, $tablename);
		
		if ($exeflag=="execute") {
			$this->userHasAccLogRights = 1;
		}
	}
	
	function getParx() {
		return $this->parx;
	}

	function _log($text) {
		echo "LOG: ".$text."<br />\n";
	}

	
	
	/**
	 * check input parameters
	 */
	function checkInput(&$sqlo) {
		global $error;
		$FUNCNAME= 'checkInput';
		
		$tablename = $this->tablename;
		
		$parx = $this->parx;
		if ($parx['repPolIgn']>0) {
			 if (!$this->isQM_admin) {
			 	$error->set( $FUNCNAME, 1, $this->adminErrTxt );
			 	return;
			 }
			 if ($parx['qm_val']<0.5) {
			 	$error->set( $FUNCNAME, 1, 'QM-val must be greater than 0.' );
			 	return;
			 }
			 if ($parx['notes']==NULL) {
			 	$error->set( $FUNCNAME, 1, 'Please give notes.' );
			 	return;
			 }
		}
		
		// @swreq SREQ:0000704:11.10(h) double signin on critical actions
		$fSignLib = new fSignatureSub();
		$needpw = $fSignLib->checkSignNeed($sqlo, $tablename);
		if ($needpw or $parx['signpw']!=NULL) {
			if ($parx['signpw']==NULL) {
				$error->set( $FUNCNAME, 4, 'Need User Password.' );
			 	return;
			}
			$pwOK = $fSignLib->checkPW($sqlo, $parx['signpw']); // password o.k. ?
			if ($pwOK<=0) {
				$error->set( $FUNCNAME, 5, 'Wrong User Password.' );
			 	return;
			}
		}
	}

	/**
	 * - lock and add audit-entry
	 * - put "lock" to an transaction, if add audit fails ...
	 */
	function accessLockOne( &$sql, $tablename, $id, $parx ) {
		global $error;
		
		$cct_access_id    = $this->cct_access_id;
		$this->accLogInfo = NULL;
		$sql->SetAutoCommit (false);
		
		$hasManiRights = access_hasManiRights( $sql, $cct_access_id );
		
		if ( $hasManiRights and ($this->action=='lock') ) {
			// only if manipulation rights exist and action=lock
			// for action="add": object will not be locked
			// @swreq 0002056: g > glob.obj.acclock.php : new action=add policy

			$this->lockObj->accessLockOne( $sql, $tablename, $id);
		}
		
		if ($parx["action"]>0 ) { 
			$this->accLogLib->addLogObj( $sql, $tablename, $id, $parx );
		}
		if ($error->Got(READONLY))  { // on error roolback !
			$sql->Rollback();
		} else {
			$sql->Commit();
		}
		$sql->SetAutoCommit (true);
		
		$this->accLogInfo    = $this->accLogLib->getInfo();
		$this->infoIsWarning = $this->accLogLib->getInfoxWarnFlag();
	}
	
	
	/**
	 * - add only the audit status
	 * - lock selected children-objects
	 * @swreq   UREQ:0000898 improve audit status setting (just add logs) 
	 * @param $sqlo
	 * @param $tablename
	 * @param $id
	 * @param $parx
	 */
	function addLogOnly(&$sqlo, $tablename, $id, $parx) {
		
		$this->accLogInfo = NULL;
		if ($parx["action"]>0 ) { 
			$this->accLogLib->addLogObj( $sqlo, $tablename, $id, $parx );
		}
		// do not do this anymore: $this->lockObj->lockChildren( $sqlo, $tablename, $id);
		$this->accLogInfo    = $this->accLogLib->getInfo();
		$this->infoIsWarning = $this->accLogLib->getInfoxWarnFlag();
	}
	
	/**
	 * analyse warning; show warning; return 0,1
	 * @return int warning exists
	 * 	0: nothing
	 *  1: info exists
	 *  2: warning, errro
	 */
	function showWarning() {
	    return oAccLogGuiC::show_Warning($this->accLogInfo, $this->infoIsWarning );
	}
	
	/**
	 * check special STATUS-policies of the company
	 * @param $sqlo
	 */
	function checkStatusPol(&$sqlo) {
		
		$statusname = $this->ACT_name;
		$polOpt = array( 'mo.CERT_FLAG'=> $this->parx['mo.CERT_FLAG'] );
		
		$this->accLogLib->setObject( $sqlo, $this->tablename, $this->id);
		$answer = $this->accLogLib->checkReleasePolicy($sqlo, $statusname, $polOpt);
		$this->qcPolAnswer = $answer;
		if ($answer==NULL) return; // no policy
		if ($answer['ok']<0) {
			htmlInfoBox( "Status Warning", $answer['txt'], "", "WARN" );
			echo '<br />';
		} 
		
	}

	function formAsk(&$sql, $sql2, $parx) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		
		//$releaseID=$this->releaseID;
		$accLogGuigLib = new oAccLogGuiC();
		$accLogGuigLib->setObject( $sql, $this->tablename, $this->id );
		if ($accLogGuigLib->actionsExist($sql) ) {
			$accLogGuigLib->showLogTable($sql, $sql2);
			echo "<br>";
		}
		// get cuurent workflow id
		$module = $accLogGuigLib->getacttrigger( $sql, $this->cct_access_id );
		
		//check if workflow for BO exists
		$flowdef=NULL;
		if ($module["id"] != NULL) $flowdef = oAccLogWfl::testifFlow( $sql, $module["id"] );
		
		if ($parx['repPolIgn']>0 and !$this->isQM_admin) {
			$error->set( $FUNCNAME, 1, $this->adminErrTxt );
			return;
		}
		
		
		$initarr   = NULL;
		$initarr["action"]      = $_SERVER['PHP_SELF'];
		$initarr["title"]       = "Lock object and add status";
		$initarr["submittitle"] = "Lock and add status";
		$initarr["tabwidth"]    = "AUTO";

		$hiddenarr = NULL;
		$hiddenarr["t"]       = $this->tablename;
		$hiddenarr["id"]      = $this->id;
		$hiddenarr["ignore"]  = $this->ignore;
		$hiddenarr["backurl"] = $this->backurl;
		$hiddenarr["action"]  = $this->action;
		$hiddenarr['parx[mo.CERT_FLAG]']  = $parx['mo.CERT_FLAG'];
		$actionReq = 1;
		$formOpt=array('noReopen'=>1);			//remove "reopen" value = 7 from list
		
		// @swreq UREQ:0002289: g > glob.obj.acclock.php : allow setting of audit qm_val 
		$qm_val_isHidden      = 1; // is the field hidden?
		
		if ( $parx['repPolIgn']>0 ) {
			$qm_val_isHidden      = 0; // show it !
		}
		
		//reduce the options for chossing new status, if workflow exists
		if ($flowdef != NULL){
		    $this->accLogLib->setObject( $sql, $this->tablename, $this->id, $this->cct_access_id );
			$status_id = $this->accLogLib->getLastLog($sql);
			if ($status_id == NULL) $status_id = "0";
			$formOpt = array('currentFlowState'=>$status_id, 'flow'=>$flowdef, 'ignore'=>$this->ignore);
		}
		
		$fields = $accLogGuigLib->formParams($sql, $parx, $actionReq, $formOpt);
		if ($fields[0]["object"] == "info"){
			$initarr["title"]       = "Information according the workflow:";
			$initarr["submittitle"] = "Back to object";
		}
		

		// this javascript function is needed for field parx[qm_val]
		?>
	<script type="text/JavaScript">  
      
    <!--  
    function showField(id)  
    {  
         if (document.getElementById(id).style.display == 'none')  
         {  
              document.getElementById(id).style.display = '';  
         }  
    }  
    //-->  
        
    </script>  
    	<?php
		
		
		$formobj = new formc($initarr, $hiddenarr, 0);

		if ($parx['repPolIgn']>0) {
			$fields[]= $accLogGuigLib->form_getRowByType('repPolIgn'); 
		}
		
		/* 
		 * - will now be shown forever (2012-03-28)
		 * - OLD: only shown, if PolicyCheck FAILED ! ???
		     $parx['repPolIgn']>0
		 * 
		 */
		if ( 1 ) {
			$fieldQC = $accLogGuigLib->form_getRowByType('qm_val');
			
			if ($this->qcPolAnswer['qc']!=NULL and $this->qcPolAnswer['qc']<1) {
				$fieldQC["val"] = $this->qcPolAnswer['qc'];
				$fieldQC["object"] = 'info2';
				$fieldQC["inits"]  = NULL;
			}
			
			if ($qm_val_isHidden) {
				$fieldQC["rowStyle"] = "DISPLAY: none";
			}
			
			$fields[]=$fieldQC;
		
		}
		
		foreach( $fields as $fieldx) {
			$name=$fieldx['name'];
			
			if ($parx['repPolIgn']>0) {
				if ($name=='action') {
					$fieldx['view'] = 1;  // no select box 
					$fieldx['hidden'] = 1;  // no select box 
				} else $fieldx['req'] = 1;
			}	
			
			$formobj->fieldOut( $fieldx );
		}
		

		$closeOpt=array();
		if ($qm_val_isHidden) {
			$closeOpt["addObjects"] = "&nbsp;&nbsp;&nbsp;<a class=\"yGgray\" 
				href=\"javascript:showField('qm_val')\">special release</a>";
		}
		
		$formobj->close( TRUE, $closeOpt );
		
		if ($flowdef != NULL){
			if ($this->ignore == 1){
				echo '<font color=gray>Dependencies are ignored by QM member.</font>';
			}else{
				echo '<font color=gray>Dependencies are defined for 
						<a href="edit.tmpl.php?t=MODULE&id='.$module["id"].'"> 
							workflow ID: '.$module["id"].'</a>.<br /></font>';
				$role_right = role_check_f($sql, $this->QM_role_right);
				echo '<font color=gray>Only QM members are able to ';
				if ($role_right != NULL){
					echo '<a href="glob.obj.acclock.php?t='.$this->tablename.'&id='.$this->id.'&ignore=1">
							ignore dependencies</a>.</font>';
				} else {
					echo 'ignore dependencies.</font>';
				}	
			}
		}	
	}
	
	
	
	function getQM_users(&$sqlo) {
		$sqlsel = role_f_get_users ( $this->QM_role_right );
		$sqlo->Quesel($sqlsel);
		$cnt=0;
		$foundMore=0;
		$dbuser=array();
		
		while($sqlo->ReadRow()) {
		  $cnt++;
		  if ( $cnt>10 ) {
		  	$foundMore=1;
		  	break;
		  }
		  $dbuser[] = $sqlo->RowData[0];
		}
		
		if ( !sizeof($dbuser) ) return; 
		
		$outstr   = "";
		$tmpkomma = "";
		foreach( $dbuser as $userid) {
			$sqlo->query("select nick from db_user where db_user_id=".$userid);
			$sqlo->ReadRow();
			$outstr .= $tmpkomma. $sqlo->RowData[0];
			$tmpkomma = ", ";
		}
		if ($foundMore) $outstr .= ", ...";
		$outstr .= "";
		return ($outstr);
	}
	
}

// --------------------------------------------------- 
/**
   	@var $userIsReflected : 0,1 user is part of a group in the right mask
   	
*/
global $error, $varcol;

$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] ); // URL-link for the first db-login; e.g. for object ($_SERVER['PHP_SELF']."?id=".$id)
$sql2  = logon2( );
if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();


$t    = $_REQUEST['t'];
$id   = $_REQUEST['id'];
$go		= $_REQUEST['go'];
$action = $_REQUEST['action'];
$backurl= $_REQUEST['backurl'];
$parx   = $_REQUEST['parx'];
$ignore = $_REQUEST['ignore'];

if ( $action==NULL ) {
	$action='lock';
}


$tablename			 = $t;
$i_tableNiceName 	 = tablename_nice2($tablename);
$scriptname 		 = 'glob.obj.acclock.php';
$title       		 = "Change status of object: ".$action;

$infoarr			 = NULL;
$infoarr["title"]    = $title;
$infoarr["title_sh"] = "Lock/Change";
$infoarr["form_type"]= "obj";
$infoarr["obj_name"] = $tablename;
$infoarr["obj_id"]   = $id;
$infoarr["show_name"]= 1;
$infoarr["icon"]     = "images/but.lock.in.gif";
$infoarr["help_url"] = 'glob.obj.acclock.html';
$infoarr['checkid']  = 1;


$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);


if ( $_SESSION["userGlob"]["g.debugLevel"]>2 ) {
    echo "<B>INFO:</B> DEBUG: backurl:".htmlentities($backurl)."; action:".$action."<br>\n";
    echo "Input-Params: ".print_r($parx,1)."<br>\n";
}

$isbo = cct_access_has2($tablename);
if ( !$isbo ) {
	 $pagelib->htmlFoot("INFO", "This function only works for business objects!");
}

switch ($action) {
	
	case 'lock':
	    // no info needed here ...
		//echo "&nbsp;<i>Info: This tool removes all access-rights from this OBJECT. \n".
		//"Optional, an audit-status can be added to the object.</i>";
		break;
	case 'add':
		echo "&nbsp;<i>Info: Add a status to object.</i>";
		break;
	default:
		htmlFoot("ERROR", "Unknown mode");
}

echo "<ul>\n";
$mainLib = new gObjaccLockGui($sql, $tablename, $id, $action, $parx, $ignore, $backurl);
$pagelib->chkErrStop();

$parx = $mainLib->getParx(); // parx was modified by method gObjaccLockGui()
$H_ALOG_ACT_ID   = $parx['action'];
$o_rights        = access_check($sql, $tablename, $id);
$userIsReflected = access_userIsReflected( $sql, $mainLib->cct_access_id);
$hasWarning = 0; /*
	 0: nothing, 
	 1: info: delay forward, 
	 2: warning or error: stop
	 */

if ( $action =='add') {
	
	$denyAddArr = array('released', 'finished', 'rejected'); // they are forbidden for "add"
	do {
	
		if ( in_array($mainLib->ACT_name,$denyAddArr) ) {
			$pagelib->htmlFoot("ERROR", "status ".$mainLib->ACT_name." is not allowed for 'add'. Need to 'lock' the object.");
		}
		if ( !$o_rights["entail"] and !$mainLib->userHasAccLogRights) {
			$pagelib->htmlFoot("ERROR", "you need entail permissions on ".$i_tableNiceName." or you must be a table admin!");
		}
	} while (0);
	
	// do the rest like  $action =='lock'
	// but doe NOT LOCK the object!
	
}


if ( ($action=='lock') or ($action=='add') ) {
	
	do {
		if ( ($H_ALOG_ACT_ID==$mainLib->finishID or $H_ALOG_ACT_ID==$mainLib->rejectedID) and $userIsReflected ) {
			// @swreq UREQ:0000704:SUBREQ:06 bei audit-status="finished": erlaube es dem User, wenn er mindestens in Rechtemaske auftaucht
			break; // do not check hard access rules for "finished"
		}
		if ( !$o_rights["entail"] and !$mainLib->userHasAccLogRights) {
			$pagelib->htmlFoot("ERROR", "you need entail permissions on ".$i_tableNiceName." or you must be a table admin!");
		}
	} while (0);
	
	if (!$go) {
		$mainLib->checkStatusPol($sql);
		$mainLib->formAsk($sql, $sql2, $parx);
		$pagelib->htmlFoot();
	}
	
	// else 
	$mainLib->checkInput($sql);
	if ($error->Got(READONLY)) {
		
		$error->printAll();
		echo "<br />\n";
		$mainLib->formAsk($sql, $sql2, $parx);
		$pagelib->htmlFoot();
	}
	
	$mainLib->accessLockOne( $sql, $tablename, $id, $parx);
	if ($error->Got(CCT_ERROR_READONLY))  {
		if ( $error->Got( READONLY,'addLogSub',22 ) ) { // check release-policy error
			$error->set($scriptname, 100, 'object '.$tablename.':'.$id); // add the object identification to error-log
			$error->printAll();
			echo '<br />';
			$qmusers = $mainLib->getQM_users($sql);
			$infoTxt = 'Only a QM-manager (role-right "'.$mainLib->QM_role_right.
				'") can set the status on this object! He can ignore the policy.<br />'.
				" Following users can help: ".$qmusers;
			
			
			htmlInfoBox( "Info", $infoTxt, "", "INFO" );
			echo '<br />';
			if ( $mainLib->isQM_admin ) {
				$parx['repPolIgn']=1;
				$mainLib->formAsk($sql, $sql2, $parx);
			} 
			$pagelib->htmlFoot();
		}
	}
	$hasWarning = $mainLib->showWarning();
	
}





if ($hasWarning>1) {
	$pagelib->htmlFoot(); // stop page
}

$forwardDelay=0;
if ($hasWarning==1) {
	$forwardDelay = 1000; // delay in milliseconds
}

$pagelib->chkErrStop();

if ($backurl!=NULL) {
	$backurl = urldecode($backurl);
} else {
	$backurl = "edit.tmpl.php?t=".$tablename."&id=".$id;
}

js__location_replace($backurl, "Back to object", NULL, $forwardDelay);

$pagelib->htmlFoot();
