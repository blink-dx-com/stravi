<?php
require_once 'o.CCT_ACCLOG.subs.inc';
require_once ('access_lock.inc');

/**
 * lock + add audit status
 * @package glob.obj.status.mod.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class gObj_StatusMod {
    
    function __construct($sqlo, $tablename, $objid) {
        $this->tablename=$tablename;
        $this->objid=$objid;
        $this->accLogInfo=array();
        
        $this->accLogLib    = new oAccLogC();
        $this->accLogLib->setObject( $sqlo, $tablename, $objid ); // init the lib
    }
    
    // return 0 or 1
    function has_status($sqlo, $statusName) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $status_id_in = oH_ALOG_ACT_subs::getH_ALOG_ACT_ID( $sqlo, $this->tablename, $statusName );
        if (!$status_id_in) {
            $error->set( $FUNCNAME, 1, 'Status "'.$statusName.'" does not exist.' );
            return;
        }
        $lastStatusID = $this->accLogLib->getLastLog($sqlo);
        
        $answer = 0;
        if ($lastStatusID==$status_id_in) $answer = 1;
        
        return $answer;
    }

    public function status_change(&$sqlo, $statusName, $auditNotes) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $tablename=$this->tablename;
        $objid=$this->objid;
        
        $lockObj      = new gObjaccLock();
        
        
        $accLogState = oH_ALOG_ACT_subs::getH_ALOG_ACT_ID( $sqlo, $this->tablename, $statusName );
        if (!$accLogState) {
            $error->set( $FUNCNAME, 1, 'Status "'.$statusName.'" does not exist.' );
            return;
        }
        $lastStatusID = $this->accLogLib->getLastLog($sqlo);
        if ($lastStatusID==$accLogState) {
            $error->set( $FUNCNAME, 2, 'Status already set.' );
            return;
        }
        
        $lockObj->accessLockOne( $sqlo, $tablename, $objid);
        
        $argu=array( "action"=>$accLogState,'notes'=>$auditNotes );
        $this->accLogLib->addLogObj( $sqlo, $tablename, $objid, $argu );
        
        $accLogInfo    = $this->accLogLib->getInfo();
        if (is_array($accLogInfo)) {
            // show info
            $this->accLogInfo=$accLogInfo;
        }
    }
    
    function get_acc_log_info() {
        return $this->accLogInfo;
    }
}