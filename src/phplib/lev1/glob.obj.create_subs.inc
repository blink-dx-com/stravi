<?php

require_once ("gStatusTriggerAbs.inc");
require_once ("gui/glob.creaWizTrigger.inc");
require_once ('access_lock.inc');
require_once ('o.CCT_ACCLOG.gui.inc');
	
/**
 * object creation sub methods: trigger/Workflow
 * @package glob.obj.create_subs.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq UREQ:0000972 
 */
class gObjCreaSubs {
    /**
     * add trigger and set audit status to "started"
     * - object will not be locked ($lockObj->accessLockOne()), 
     *    because it makes no sense to reject access rights at the beginning !
     * @param  $sqlo
     * @param  $tablename
     * @param  $objid object id
     * @param  $mxid  module-id : the trigger
     * @return -
     */
    static function addTriggerAndStart( &$sqlo, $tablename, $objid, $mxid) {
    	
    	$triggerLib = new gStatusTriggerDef();
    	$triggerLib->setTrigger2Obj ($sqlo , $mxid, $tablename, $objid);
    	
    	// $lockObj      = new gObjaccLock();
    	$accLogLib    = new oAccLogC();
    	$accLogState = oH_ALOG_ACT_subs::getH_ALOG_ACT_ID( $sqlo, $tablename, 'started' );
    	if (!$accLogState) return; // not exists
    	
    	// $lockObj->accessLockOne( $sqlo, $tablename, $objid);
    	$argu=array("action"=>$accLogState);
    	$accLogLib->addLogObj( $sqlo, $tablename, $objid, $argu );
    	
    	// add access for user
    	$myGroupID   = access_getPersGrp ($sqlo, $_SESSION['sec']['db_user_id']);
    	$w_access_id = getAccessId($sqlo, $tablename, $objid);
    	if ($myGroupID) {
    		$rights = access_getInsRights();
    		$rights['delete']=0;
    		access_write ( $sqlo, $w_access_id, $rights, $myGroupID );
    	}
    }
    
    /**
     * get User-Workflow or use input-Workflow ($mxid)
     * - if input-param $mxid => use this
     * - check GLOBAL trigger of table
     * - check personal trigger (overrule the former one)
     * - check abstract-object trigger (overrule the former one)
     * 
     * - if exists: add Workflow; start Workflow
     * - global: $_SESSION['userGlob']["o.".$tablename.".fCreaOpt"] : contains user workflow
     * @param $sqlo
     * @param $tablename
     * @param $objid
     * @param $mxid
     * @param int $abstract_obj ID of abstract object (can contain a workflow-ID)
     */
    static function addUserWorkflow(&$sqlo, $tablename, $objid, $mxid=NULL, $abstract_obj=0) {
    	
    	$userWorkflow = NULL; // the workflow of the user
    	$useTriggerID = 0;
    	$workflowAnaLib = new glob_creaWizTrigger();
    	
    	do {
    		if ($mxid>0) {
    			$useTriggerID = $mxid;
    			break;
    		}
    		
    		if ($abstract_obj) {
    			$conAbsLib     = new gConcAbsC();
    			$abs_tablename = $conAbsLib->getTemplate($tablename);
    			$useTriggerID  = $workflowAnaLib->getWfl_ofAbstract($sqlo, $abs_tablename, $abstract_obj);
    			
    			if ($useTriggerID) {
    				break;
    			}
    		}
    		
    		// personal trigger
    		if ($_SESSION['userGlob']["o.".$tablename.".fCreaOpt"]!=NULL) {
    			$wizardOpts   = unserialize($_SESSION['userGlob']["o.".$tablename.".fCreaOpt"]);
    			$userWorkflow = $wizardOpts['trigger'];
    		}
    		
    		/**
    	 	 *optional: add trigger
    	 	 */
    		$useTriggerID  = $workflowAnaLib->getTriggerOfTable($sqlo, $tablename, $userWorkflow ); 
    		
    		
    	} while (0);
    
    	if ($useTriggerID>0) {
    		self::addTriggerAndStart( $sqlo, $tablename, $objid, $useTriggerID );
    	}
    }

}
