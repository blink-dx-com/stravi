<?php
require_once ('access_lock.inc');
require_once 'o.CCT_ACCLOG.subs.inc';

/**
 * reopen one object
 * @package o.CCT_ACCESS.reopen.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class gObjAccReopen {
	
	
	function __construct(&$sqlo) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
		$this->accLogLib    = new oAccLogC();
		$this->myGroup = access_getPersGrp ($sqlo, $_SESSION['sec']['db_user_id'] );

		if (!$this->myGroup) {
			$error->set( $FUNCNAME, 2, "You have no personal group!");
			return;
		}
	}
	
	/**
		 * - reopen object
		 * - entail-right was checked before ...
		 * @param array $parx
		 * 	 'notes'
		 * 	 'action'
		 *   'repPolIgn'
		 * @param array $roptions options
		 * 	 'ignoreRights' : 
		 *         [0]: normal access check 
		 *          1 : ignore access check
		 * @global 
		 * 
		 */
	function doReopen(&$sqlo, $cct_access_id, $tablename, $id, $parx, $roptions=NULL ) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		// $this->_log( "... reopen object.");
		$h_alog_reopen_id = oH_ALOG_ACT_subs::getH_ALOG_ACT_ID( $sqlo, $tablename, "reopen" );
		if (!$h_alog_reopen_id) {
		    $error->set( $FUNCNAME, 1, "Missing 'reopen' status in system!");
		    return;
		}
	
		if ( !$roptions['ignoreRights'] ) {
			$this->setRights = access__allowed_to_chmod ( $sqlo, $cct_access_id, $tablename );
			
			if ($this->setRights == "no") {  // user has no rights to change rights but nevertheless he tried it
				$error->set( $FUNCNAME, 1, "No right to change rights!");
				return;
			}
		}
		
		$group_id = $this->myGroup;
		if (!$group_id) {
			$error->set( $FUNCNAME, 2, "You must have a personal group!");
			return;
		}
		
		if (!is_array($parx)) $parx=array();
		
		if (trim($parx["notes"])==NULL) {	//notes field for reopen action is null or empty string
			$error->set( $FUNCNAME, 3, "No reason given for reopen an audit-trail object." );
			return;
		}
		
		$rights_arr = access_getRights(); // all rights to 1 !
		$newrights = NULL;
		foreach( $rights_arr as $val) {
			$newrights[$val]=1;
		}
		
		
		$addLogParx = array(
			
			"notes"		=> $parx["notes"],
			'repPolIgn' => $parx["repPolIgn"]
			
		); 
		
		$this->accLogLib->setObject( $sqlo, $tablename, $id, $cct_access_id);
		$this->accLogLib->do_reopen( $sqlo, $addLogParx );
		if ($error->Got(READONLY)) {
			$error->set( $FUNCNAME, 5, "Adding 'reopen'-status failed." );
			return;
		}
		
		// unlock must come after STATUS-check ! Otherwise on error, the object is open!
		$lockLib = new gObjaccLock();
		$lockLib->reopen ( $sqlo, $tablename, $id, $newrights, $group_id );
		if ($error->Got(READONLY)) {
			$error->set( $FUNCNAME, 4, "access_write failed." );
			return;
		}
	}
	
	public function getInfo() {
		return $this->accLogLib->getInfo();
	}
	
	public function getInfoxWarnFlag() {
		return $this->accLogLib->getInfoxWarnFlag();
	}
}
