<?php
require_once ('o.USER_GROUP.subs.inc');
require_once 'f.update.inc';

/**
 * sub functions for workflows
 *    
 * @package o.MODULE.trigger.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com) 
 */
class oMODULE_trigger {
	
	var $TRIGGER_TYPE=2;
	
	/**
	 * get all triggers of TYPE=2, KEY=TABLE_NAME, VALUE=$table
	 * @param $sqlo
	 * @return array of MX_ID
	 */
	function getTriggersOfTable(&$sqlo, $table) {
		$moduleArr=NULL;
		
		$sqlsel = 'x.MXID from MOD_META meta join MODULE x on x.MXID=meta.MXID where x.TYPE='.
			$this->TRIGGER_TYPE." and meta.KEY='TABLE_NAME' and meta.VALUE=".$sqlo->addquotes($table).' order by MXID';
		$sqlo->Quesel($sqlsel);
		while ($sqlo->ReadRow() ) {
			$mxid = $sqlo->RowData[0];
			$moduleArr[]=$mxid;
		}
		return $moduleArr;
	}
	
	/**
	 * return list of user-NICKs in MODULE: MOD_META
	 * examples of VALUE in MOD_META: "USER:7847,USER_GROUP:343,USER:843"
	 * @param $mxid
	 * @param $modKey KEY of MOD_META
	 * @return $userArr array of nick names
	 */
	function getUsernamesByKey(&$sqlo, $mxid, $modKey) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$moduleidNow = $mxid;
		
		// get email-groups for this status
		$sqlsel = "VALUE from MOD_META where MXID=".$moduleidNow. " and KEY like '".$modKey."'";
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$emailvalue = trim($sqlo->RowData[0]);
		if ($emailvalue==NULL) return;
		
		$groupArr = explode(',',$emailvalue);
		$grpArrFinal= NULL;
		$userArr    = array();
		foreach( $groupArr as $dummy=>$val) {
			$oneUserEmail = explode(':',$val); // $val = "USER:7847", "USER_GROUP:343"
			$entryID=$oneUserEmail[1];
			if (!is_numeric($entryID)) {
				$error->set( $FUNCNAME, 1, 'email-params invalid: '.$oneUserEmail );
				return;
			}
			switch ($oneUserEmail[0]) {
				case "USER_GROUP":
					$userArrTmp = oUSER_GROUP_subs::getUsersOfGroup($sqlo, $entryID);
					if (is_array($userArrTmp)) {
						$userArr = array_merge($userArr,$userArrTmp);
						$userArr = array_unique($userArr);
					}
					break;
				case "USER":
					$userArr[]= $entryID; // add one user
					$userArr  = array_unique($userArr);
					
					break;
			}
		}
		reset ($groupArr);
		
		if (!sizeof($userArr)) return; // nix email
		$nickArr=NULL;
		foreach( $userArr as $dummy=>$userid) {
			$nick = obj_nice_name ( $sqlo, 'DB_USER', $userid ); 
			$nickArr[$nick]=1; // get only unique user-names
		}
		reset ($userArr);
		
		$nickArr = array_keys($nickArr);
		return $nickArr;
	}
	
	static function get_val_of_key(&$sqlo, $mxid, $key) {
	    
	    $sqlsel = "VALUE from MOD_META where MXID=".$mxid. " and KEY like '".$key."'";
	    $sqlo->Quesel($sqlsel);
	    $sqlo->ReadRow();
	    $val = trim($sqlo->RowData[0]);
	    return $val;
	}
	
	static function obj_mod_trigger($sqlo, $newtrigger, $tablename, $objid) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    if ($newtrigger != NULL){
	        $sqlcheck = "NAME, TYPE from MODULE where MXID = ".$newtrigger;
	        $sqlo->Quesel($sqlcheck);
	        $sqlo->ReadRow();
	        if ($sqlo->RowData[0] == NULL && $newtrigger != NULL){
	            $error->set( $FUNCNAME, 1, 'No workflow with this ID!' );
	            return;
	        }
	        $type = $sqlo->RowData[1];
	        if ($type!=2) {
	            $error->set( $FUNCNAME, 2, 'A workflow must be defined: MODULE:TYPE=2. Please choose an other MODULE!' );
	            return;
	        }
	        
	    }
	    // update workflow
	    $feats = glob_ObjDataGet( $sqlo, $tablename, $objid, array('CCT_ACCESS_ID') );
	    $cct_access_id  = $feats['CCT_ACCESS_ID'];
	    if (!$cct_access_id) {
	        $error->set( $FUNCNAME, 5, 'No CCT_ACCESS_ID found for object ('.$tablename.', '.$objid.')' );
	        return;
	    }
	    
	    $argu=array('WFLOW_ID'=>$newtrigger);
	    $idarr=array('CCT_ACCESS_ID'=>$cct_access_id);
	    $up_flag = gObjUpdate::update_row_s ( $sqlo, 'CCT_ACCESS', $argu, $idarr);
	    if  ( $up_flag < 0 ) {
	        $error->set($FUNCNAME, 10, "error on SQL-update");
	        return;
	    }
	}
}
