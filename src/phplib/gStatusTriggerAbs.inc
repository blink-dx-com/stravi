<?php
/**
 * - workflow solution for status change of a BO
 *
 * @package gStatusTriggerAbs.inc
 * @author  Marco Albrecht (marco_a@clondiag.com)
 * @swreq UREQ:0000124 g.workflow > Implementierung
 *
 */

require_once ("glob.obj.update.inc");
require_once ("access_mani.inc");
require_once ("date_funcs.inc");
require_once ('f.email.inc');
require_once ('o.USER_GROUP.subs.inc');
require_once 'o.MODULE.subs.inc';


/**
 * set workflow-object to object
 * @author steffen
 *
 */
class gStatusTriggerDef {
	
	/**
	 * function for insert wflow_id in cct_access
	 * @param $sqlo
	 * @param string $workflow name of the initialized workflow from module
	 * @param int $cct_access_id where has the workflow id have to be inserted
	 * @return -
	 */
	function setTriggerID (&$sqlo , $triggerid, $cct_access_id) {
		//global $error;
		//$FUNCNAME = __CLASS__.':setObjectID';
		$updateLib = new globObjUpdate();
		$idarr= array('CCT_ACCESS_ID'=>$cct_access_id);
		$argu = array('WFLOW_ID'=>$triggerid);
		$args = array('vals'=>$argu);
		$updateLib->update_raw($sqlo, 'CCT_ACCESS',$args, $idarr);
		
	}
	
	/**
	 * set trigger to object
	 * - do some initial sets
	 * @param $sqlo
	 * @param $triggerid
	 * @param $tablename
	 * @param $objid
	 * @throws exception
	 * @return -
	 */
	function setTrigger2Obj (&$sqlo , $triggerid, $tablename, $objid) {
		// global $error;
		$FUNCNAME = __CLASS__.':setTrigger2Obj';
		
		$pkname = PrimNameGet2($tablename);
		if ($pkname==NULL) sys_error_my( $FUNCNAME.' table '.$tablename. ' has no PK-def' );
		if (!cct_access_has2($tablename)) throw new Exception(' table '.$tablename. ' has no CCT_ACCESS_ID' );
		
		// analyze trigger
		
		$tr_feats = glob_elemDataGet3( $sqlo, 'MODULE', array('MXID'=>$triggerid), array('TYPE') );
		if ($tr_feats['TYPE']!=2) {
		    throw new Exception('Workflow (ID:'.$triggerid.') must have attribute TYPE=2');
		}
		
		$tr_feats2 = glob_elemDataGet3( $sqlo, 'MOD_META', array('MXID'=>$triggerid, 'KEY'=>'TABLE_NAME'), array('VALUE') ); 
		if ($tr_feats2['VALUE']!=NULL) {
		    if ($tr_feats2['VALUE']!=$tablename) {
		        throw new Exception('Workflow (ID:'.$triggerid.') must be configured for TABLE_NAME='.$tablename);
		    }
		}
		
		// update object
		$sqlloc = "CCT_ACCESS_ID from ".$tablename." where ".$pkname."=".$objid;
		$sqlo->Quesel($sqlloc);
		$sqlo->ReadRow();
		$cct_access_id = $sqlo->RowData[0];
		if (!$cct_access_id) return;
		$this->setTriggerID ($sqlo , $triggerid, $cct_access_id);
	}
}

class gStatusTriggerManage{
    
    static function dynamicIncludeLoc (&$sql, $location, $wflow_id=0) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $codepath = $_SESSION['s_sessVars']['AppLabLibDir'] .'/workflow/' . $location;
        if (!file_exists($codepath)) {
            $pre_text='';
            if ($wflow_id) $pre_text='module-ID '.$wflow_id.': ';
            $error->set( $FUNCNAME, 2, $pre_text.'code on location "'.$codepath.'" not found.' );
            return;
        }
        require_once($codepath);
        $relFile  = $codepath;
        //$pureModPathArr = explode('/',$location);
        //$namePos   = sizeof($pureModPathArr)-1;
        //$pureModName    = $pureModPathArr[$namePos];
        $loc = explode('.inc', $location);
        $location  = $loc[0];
        $classname = $location.'_TRG';
        $classname = str_replace('.','_',$classname);
        if (!class_exists($classname)) {
            $error->set( $FUNCNAME, 5, 'Plugin "'.$relFile.'": class:'.$classname.' not found.' );
            return;
        }
        
        return $classname;
    }

	/**
	 * include the workflow code
	 * @param $sql
	 * @param $argu
	 * @param $wflow_id
	 * @return string 
	 */
	static function dynamicinclude (&$sql, $argu, $wflow_id){
		global $error;
		$FUNCNAME = __CLASS__.':dynamicinclude';
		
		
		$sqlloc = "LOCATION from MODULE where MXID = ".$wflow_id;
		$sql->Quesel($sqlloc);
		$sql->ReadRow();
		$location = $sql->RowData[0];
		if ($location==NULL) {
			$error->set( $FUNCNAME, 1, 'No location found for module-ID '.$wflow_id.' not found.' );
			return;
		}
		
		return self::dynamicIncludeLoc($sql, $location, $wflow_id);
	}
	
	
	
	
	
}


/**
 * abstract TRIGGER class
 * @author marco_a
 * @param argu see below
 */
class gStatusTriggerAbs {
	var $_argu; /* initial arguments
	    'QM_VAL'        =$QM_VAL; // save only values > 0
		'CCT_ACCESS_ID' = $this->cct_access_id;
		'DB_USER_ID' 	= ${cctuser_id};
		'ACT_DATE' 	  = time();
		'H_ALOG_ACT_ID' =$parx["action"];
		'NOTES' 	  = $parx["notes"];
		'H_ALOG_ACT_ID_last'  = $H_ALOG_ACT_ID_last;
		'tablename'   = $this->tablename;
		'wflow_id' 	  = $wflow_id;
		'new_status_name' name of status
		'old_status_name' name of status
	*/
	var $_moduledata; // key,vals from MODULE
	
	var $_sqlo2; // second sql handle
	var $logarr;
	private $log_IsWarning; // the $logarr array is a warning, else just info-test
	/*
	 * 0: info
	 * 1: info    : stop forward in higher program level
	 * 2: warning : stop forward in higher program level
	 */
	
	function __construct(){
		$this->logarr = NULL;
		$this->rightarr=NULL;
		$this->_sqlo2 = NULL;
		$this->log_IsWarning = 0;
	}
	
	function init(&$argu) {
		$this->_argu = &$argu;
		$this->_moduledata=NULL;
		$this->_moduledataInit=0; // _moduledata was initialized ?
		
	}
	
	function __objurl($tablename, $obj_id) {
	    $base_url= "http://".$_SERVER['SERVER_NAME'];
	    $objurl = $base_url.'/pionir/edit.tmpl.php?t='.$tablename.'&id='.$obj_id;
	    return $objurl;
	}
	
	function __getModuleMetadata(&$sqlo, $wflow_id){
		
		if ( $this->_moduledataInit ) return $this->_moduledata;
		
		$sqlo->Quesel("VALUE, KEY FROM MOD_META where MXID = " . $wflow_id);
		while ( $sqlo->ReadRow() ) {
			$this->_moduledata[$sqlo->RowData[1]]=  $sqlo->RowData[0];
		}
		$this->_moduledataInit=1;
		return $this->_moduledata;
	}
	
	
	
	/**
	 * 
	 * @param object $sqlo
	 * @param array $userArr -- list of nicks
	 * @param array $email_args
	 */
	function __sendEmailRaw(&$sqlo, array $userArr, array $email_args ) {
	    $mailto = new fEmailC ();
	    $send_opt=array('out.type'=>'html');
	    $mailto->sendgrpEmails($userArr, $email_args, $sqlo, $send_opt);
	    $this->_addlog('send email to: '.implode(', ',$userArr), 1);
	    
	    $mail_log = $mailto->getmaillog();
	    if($mail_log!=NULL) {
	        $this->_addlog('Maill-Log: '.$mail_log, 1);
	    }
	    
	}
	
	/**
	 * send AUDIT-STATUS-change emails to "emailuser"
	 * - Email-Text-Example: ------------------------
	 * Das Objekt 'S29198_S26565_Haelfte2' (table:array-batch, ID: 24548) wechselte am 2013-06-27 13:42 den Status von 'rejected' auf 'produced'.
	 * Status-Notes: haha produced
	 *
	 * PS: Module: tiberia2.clondiag.jena/MODULE=fEmailC:sendgrpEmails
	 * PS: Switch on/off personal emails: see user-preferences: "g.emailOpt"
	 * @param $sqlo
	 * @param string $emailuserStr user-NICK string, komma separated
	 * @param array $argu
	 * @param string $content : extra content, line-nbreaks: "\n"
	 * @return -
	 */
	function __sendEmail(&$sqlo, $emailuserStr, $argu, $content = NULL) {
	    
		$new_status_name = $argu['new_status_name'];
		$old_status_name = $argu['old_status_name'];
		$cct_access_id   = $argu['CCT_ACCESS_ID'];
		
		$mess_BR='<br>'."\n";
		
		$tablename = $argu['tablename'];
		$statusDate= date_unix2datestr( $argu['ACT_DATE'] ,2 );
		if($old_status_name == "") {	
			$old_status_name = "NONE";
		}

		$sqlo->Quesel("NAME, ".$tablename."_ID from ".$tablename.
					" where CCT_ACCESS_ID = " . $cct_access_id);
		$sqlo->ReadRow();
		$obj_name = $sqlo->RowData[0];
		$obj_id   = $sqlo->RowData[1];
		$tablenice=tablename_nice2($tablename);
		
		
		$obj_url = $this->__objurl($tablename, $obj_id); 
		$prodsubject =  '"'. $obj_name. '"('.$tablenice.") > Status '" .$new_status_name . "'";
		$prodmessage = 'Das Objekt <a href="'.$obj_url. '">'. $obj_name. "</a> (table:".$tablenice.", ID: ".$obj_id.") ".$mess_BR.
		    "wechselte am ".
			$statusDate . " den Status von '" . $old_status_name 
			. "' auf '". $new_status_name . "'.";
		
		if ($argu['NOTES']!=NULL) {
		    $prodmessage .= $mess_BR."Status-Notes: " . $argu['NOTES'] . $mess_BR;
		}

		if ($content != NULL){
			$prodmessage .= str_replace("\n", "<br>\n", $content);
		}	
		
		// ------------------

		$sqlo->Quesel("NICK from DB_USER where DB_USER_ID in (select DB_USER_ID from CCT_ACCESS".
		    " where CCT_ACCESS_ID = " . $cct_access_id.")");
		$sqlo->ReadRow();
		$creator_nick = $sqlo->RowData[0];
		
		$prodaddtotrim = explode(",", $emailuserStr); //adresses of production managers
		if (!sizeof($prodaddtotrim)) return; // no adresses
		
		$address = NULL;
		$userArr = NULL;
		foreach ($prodaddtotrim as $address){
		    if ($address=='OBJ.creator') $address = $creator_nick;
		    $userArr[] = trim($address);
		}
		
		$email_args = array();
		$email_args['subject'] = $prodsubject;
		$email_args['message'] = $prodmessage;
		
		$this->__sendEmailRaw($sqlo, $userArr, $email_args);
	}
	
	/**
	 * send email to main.group of other workflow $other_ord_id
	 * @param object $sqlo
	 * @param int $other_ord_id
	 * @param string $content
	 */
	function __sendEmailOtherMod($sqlo, $other_ord_id, $content) {
	    
	    $ord_sub_lib = new oMODULE_one($other_ord_id);
	    $grp_id = $ord_sub_lib->get_meta_by_key($sqlo, 'main.group');
	    if (!$grp_id) return; // no group set
	    
	    if (!gObject_exists ($sqlo, 'USER_GROUP', $grp_id) ) {
	        return;
	    }
	    
	    // get users
	    $users = oUSER_GROUP_subs::getUsersOfGroup($sqlo, $grp_id);
	    if (empty($users)) return; // no users
	    
	    $grp_name = glob_elementDataGet( $sqlo, 'USER_GROUP', 'USER_GROUP_ID', $grp_id, 'NAME');
	    $this->_addlog('send emails to main-group "'.$grp_name.'" of other worklfow ID:'.$other_ord_id.'.', 1);
	    
	    $user_nicks = array();
	    foreach($users as $user_id) {
	        $nick = glob_elementDataGet( $sqlo, 'DB_USER', 'DB_USER_ID', $user_id, 'NICK');
	        $user_nicks[] = $nick;
	    }
	    $emailuserStr = implode(',',$user_nicks);
	    
	    $this->__sendEmail($sqlo, $emailuserStr, $this->_argu, $content);
	}
	
	/**
	 * check, if the status-change will send emails
	 *  -check state.STATUS.emailuser
	 * @param object $sqlo
	 * @param array  $argu
	 * @param string $content : extra email content
	 * @return -
	 */
	function __checkTriggerMail( &$sqlo, $argu, $content=NULL){		
		global $error;
		$FUNCNAME = __CLASS__."__checkTriggerMail";
		
		$wflow_id		 = $argu["wflow_id"];
		$new_status_name = $argu['new_status_name'];
		
		$moduledata = $this->__getModuleMetadata($sqlo, $wflow_id);
		if ($moduledata == NULL) { // not an error ...
			return;
		}
		
		
	
		$keyName='state.'.$new_status_name.'.emailuser';
		if ($moduledata[$keyName] != NULL){
			$emailuserStr = $moduledata[$keyName];
			$this->__sendEmail($sqlo, $emailuserStr, $argu, $content);
		}
		
	}
	
	/**
	 * - check, if user is allowed to enter new status
	 * - user is member of this group ???
	 * $keyName='state.'.$new_status_name.'.allowGrp' = group-id
	 * @param $sqlo
	 * @return array($isAllowed,$reason) 
	 * 	  boolean $isAllowed (0,1)
	 *    string reason of denial, if $isAllowed=0
	 */
	function __checkNewStatus(&$sqlo) {
		$isAllowed= 1;
		$reason   = NULL;
		$new_status_name = $this->_argu['new_status_name'];
		$wflow_id		 = $this->_argu["wflow_id"];
		
		if ($_SESSION['sec']['appuser']=='root') return array(1); // root !!!
		
		$moduledata = $this->__getModuleMetadata($sqlo, $wflow_id);
		if ($moduledata == NULL) {
			return array($isAllowed);
		}
		
		$keyName='state.'.$new_status_name.'.allowGrp';
		if ($moduledata[$keyName] == NULL) return array($isAllowed); // no action
		
		$this->_addlog('check your group membership');

		$isMember=0;
		$grpIdArr = explode(',',$moduledata[$keyName]);
		foreach( $grpIdArr as $groupid) {
			if (oUSER_GROUP_subs::userIsMember($sqlo, $_SESSION['sec']['db_user_id'], $groupid) ) {
				$isMember=1;
				break;
			}
		}
		
		
		if ($isMember) $isAllowed=1;
		else {
			$isAllowed=0;
			$reason = 'Not member of groups with ID: '.implode(', ',$grpIdArr);
		}
		
		return array($isAllowed, $reason);
	}
	
	/**
	 * add rights by "state.STATUS.rights"
	 * value: serialized(array(GROUP_ID=>array('write'=>1, 'entail'=>1) )  )
	 * @param $sqlo
	 * @param $group_id
	 * @param $rights
	 * @return -
	 */
	function __addRightsAuto( &$sqlo ) {
		
		$argu  =  $this->_argu;
		$wflow_id		 = $argu["wflow_id"];
		$new_status_name = $argu['new_status_name'];
		
		// comment out next line if adding rights to "released" is not allowes
		//if ($new_status_name=='released') return;
		
		$moduledata = $this->__getModuleMetadata($sqlo, $wflow_id);
		if ($moduledata == NULL) { // not an error ...
			return;
		}
		
		$keyName='state.'.$new_status_name.'.rights';
		if ($moduledata[$keyName] == NULL) return; // no action exists
		
		$rightarr = unserialize($moduledata[$keyName]);
		if (!is_array($rightarr)) return; // error
		
		$cct_access_id = $this->_argu['CCT_ACCESS_ID'];
		if (!$cct_access_id) return; // error
		
		foreach( $rightarr as $group_id=>$rights) {
			access_write( $sqlo, $cct_access_id,  $rights, $group_id);
		}
		reset ($rightarr);
		$this->rightarr = $rightarr;
		$this->_addlog('added rights to defined groups');
		
	}
	
	/**
	 * add rights to an other object, e.g. child protocol of device
	 * @param $sqlo
	 * @param string $table tablename
	 * @param $objid object id
	 * @return -
	 */
	function __addRightsObject(&$sqlo, $table, $objid, $rights=NULL) {
		if (is_array($rights)) $rightarr = $rights;
		elseif (is_array($this->rightarr)) $rightarr = $this->rightarr;
		else return;
		
		$cct_access_id = cct_access_get( $sqlo, $table, $objid ); 
		if (!$cct_access_id) return; // error
		
		foreach( $rightarr as $group_id=>$rights) {
			access_write( $sqlo, $cct_access_id, $rights, $group_id);
		}
		reset ($rightarr);
	}
	
	/**
	 * 
	 * Enter description here ...
	 * @param $sqlo
	 * @param $table
	 * @param $objid
	 * @param $rights
	 */
	function __removeRights(&$sqlo, $table, $objid, $rights=NULL) {
		$f_del_group = 1;
		
		if (is_array($rights)) $rightarr = $rights;
		elseif (is_array($this->rightarr)) $rightarr = $this->rightarr;
		else return;
		
		$cct_access_id = cct_access_get( $sqlo, $table, $objid ); 
		if (!$cct_access_id) return; // error
		
		foreach( $rightarr as $group_id=>$rights) {
			access_write( $sqlo, $cct_access_id, $rights, $group_id, $f_del_group);
		}
		reset ($rightarr);
	}
	
	/**
	 * add object to project by "state.STATUS.project"
	 * value: ID of destination project
	 * @param $sqlo
	 * @param $proj_id
	 * @return -
	 */
	function __linktoproject( &$sqlo, $argu ) {
		global $error;
		$FUNCNAME = __CLASS__."__linktoproject";
		require_once ("o.PROJ.addelems.inc");
		
		$wflow_id  = $argu["wflow_id"];
		$tablename = $argu['tablename'];
		$new_status_name = $argu['new_status_name'];
		
		$moduledata = $this->__getModuleMetadata($sqlo, $wflow_id);
		if ($moduledata == NULL) { // not an error ...
			return;
		}
		
		$keyName='state.'.$new_status_name.'.project';
		if ($moduledata[$keyName] == NULL) return; // no action exists
		
		$projarr = explode(",", $moduledata[$keyName]);
		if (!is_array($projarr)) return; // error
		
		$cct_access_id = $this->_argu['CCT_ACCESS_ID'];
		if (!$cct_access_id) return; // error
		
		$pkname = PrimNameGet2($tablename);
		$sqlo->Quesel($pkname." from ".$tablename.
				" where CCT_ACCESS_ID = " . $cct_access_id);
		$sqlo->ReadRow();
		$obj_id     = $sqlo->RowData[0];
		
		$projAddLib = new oProjAddElem($sqlo);
		$projOpt    = array('noAccCheck'=>1);
		
		foreach( $projarr as $proj_id) {
			if (!gObject_exists ($sqlo, 'PROJ', $proj_id)) {
				$error->set( $FUNCNAME, 1, 'Project-ID '.$proj_id.' does not exist.' );
				return;
			}
			$projAddLib->setProject( $sqlo, $proj_id, $projOpt);
			$projAddLib->addObj( $sqlo, $tablename, $obj_id );
		}
	
		$this->_addlog('added object to defined projects');
		
	}
	
/**
	 * remove object from project by "state.STATUS.remove"
	 * value: ID of destination project
	 * @param $sqlo
	 * @param $proj_id
	 * @return -
	 */
	function __unlinkfromproject( &$sqlo, $argu ) {
		global $error;
		$FUNCNAME = __CLASS__."__removefromproject";
		require_once ("o.PROJ.addelems.inc");
		
		$wflow_id  = $argu["wflow_id"];
		$tablename = $argu['tablename'];
		$new_status_name = $argu['new_status_name'];
		
		$moduledata = $this->__getModuleMetadata($sqlo, $wflow_id);
		if ($moduledata == NULL) { // not an error ...
			return;
		}
		
		$keyName='state.'.$new_status_name.'.unlink';
		if ($moduledata[$keyName] == NULL) return; // no action exists
		
		$projarr = explode(",", $moduledata[$keyName]);
		if (!is_array($projarr)) return; // error
		
		$cct_access_id = $this->_argu['CCT_ACCESS_ID'];
		if (!$cct_access_id) return; // error
		
		$pkname = PrimNameGet2($tablename);
		$sqlo->Quesel($pkname." from ".$tablename.
				" where CCT_ACCESS_ID = " . $cct_access_id);
		$sqlo->ReadRow();
		$obj_id     = $sqlo->RowData[0];
		
		$projAddLib = new oProjAddElem($sqlo);
		$projOpt    = array('noAccCheck'=>1);
		
		foreach( $projarr as $proj_id) {
			if (!gObject_exists ($sqlo, 'PROJ', $proj_id)) {
				$error->set( $FUNCNAME, 1, 'Project-ID '.$proj_id.' does not exist.' );
				return;
			}
			$projAddLib->setProject( $sqlo, $proj_id, $projOpt);
			$projAddLib->unlinkObj( $sqlo, $tablename, $obj_id );
		}
		reset ($projarr);
		$this->_addlog('unlinked object from defined projects');
		
	}
	
	// get object ID
	function __getObjID(&$sqlo) {
		$cct_access_id = $this->_argu['CCT_ACCESS_ID'];
		$tablename     = $this->_argu['tablename'];
		$pkname = PrimNameGet2($tablename);
		$sqlo->Quesel($pkname." from ".$tablename.
				" where CCT_ACCESS_ID = " . $cct_access_id);
		$sqlo->ReadRow();
		$obj_id     = $sqlo->RowData[0];
		return $obj_id;
	}
	
	function _addlog($text, $is_warning=0) {
		$this->logarr[]=$text;
		if ($is_warning) $this->log_IsWarning=1;
	}
	
    // register the second SQL-handle
	function &_regSql2() {
		if ($this->_sqlo2==NULL) {
			$this->_sqlo2  = logon2(  );
		}
		return $this->_sqlo2;
	}
	
	function getlog() {
		return $this->logarr;
	}
	function has_warning() {
	    return $this->log_IsWarning;
	}
	
	/**
	 * function for setting changes when trigger is activated
	 * @param $sqlo
	 * @param $argu array for arguments the trigger should use
	 * @return -
	 */
	function OnTrigger( &$sqlo, $argu){	
	}
	
	/**
	 * function before status is changed
	 * @param $sqlo
	 * @return -
	 */
	function beforeStatus( &$sqlo ) {
	}
}
	
	
