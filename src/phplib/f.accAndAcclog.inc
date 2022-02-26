<?php
require_once ('o.CCT_ACCLOG.subs.inc');
require_once ('access_check.inc');

/**
 * - sub functions to handle CCT_ACCESS_RIGHTS and CCT_ACCLOG
 * - check, if user can change the rights ...
 * @package f.accAndAcclog.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq   UREQ:0001150: g > modify access for list of objects
 */
class fAccAndAcclogC {

function __construct( &$sql ) {
	$this->accLogLib = new oAccLogC();
}	

/**
 * check, if object has a object log, if it has manipulation rights
 * @return array ("haslog"=>0,1, "lastId"=>Audit status ID, 'hasMani'=>0,1)
 */
function objHasLog( &$sql, $access_id ) {
	// global $error;
	
	$this->haslog    = 0;
	$this->lastlogId = 0;
	
	//if ( !$this->accLogLib->implemented ) return;
	
	// check HAS_RIGHTS AND CCT_ACCLOG
	$hasManiRights = access_hasManiRights( $sql, $access_id );
	$tablename = glob_elementDataGet( $sql, 'CCT_ACCESS', 'CCT_ACCESS_ID', $access_id, 'TABLE_NAME');
	
	$this->accLogLib->setObject( $sql, $tablename, NULL, $access_id );
	$this->lastlogId = $this->accLogLib->getLastLog($sql);
	
	$h_alog_reopen_id = oH_ALOG_ACT_subs::getH_ALOG_ACT_ID( $sql, $tablename, "reopen" );
	
	if ($this->lastlogId AND $this->lastlogId!=$h_alog_reopen_id ) {
		// and is NOT "reopen"
		$this->haslog=1;
	}
	// }
	
	return array("haslog"=>$this->haslog, "lastId"=>$this->lastlogId, 'hasMani'=>$hasManiRights );
}

function reopenCheck($change_allow) {}

}
