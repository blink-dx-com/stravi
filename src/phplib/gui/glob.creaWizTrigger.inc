<?php
require_once 'glob.obj.conabs.inc';
require_once ("o.S_VARIO.subs.inc");

/**
 * Workflow (trigger) selection management
 * @package glob.creaWizTrigger.inc
 * @swreq   GLOBAL
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/phplib/gui/glob.creaWizTrigger.inc 59 2018-11-21 09:04:09Z $
 */
class glob_creaWizTrigger {
	
	/**
	 * STATIC
	 * - get GLOBAL workflow
	 * - get USER-selected workflow (can overrule GLOBAL trigger!)
	 * - check if trigger exists
	 * @param $sqlo
	 * @param $table
	 * @param $triggerOfUser
	 * @return unknown_type
	 */
	static function getTriggerOfTable(&$sqlo, $table, $triggerOfUser ) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
		$varname       ='o.'.$table.'.initTriggerID';
		$globTriggerID = glob_elementDataGet( $sqlo, 'GLOBALS', 'NAME', $varname, 'VALUE'); 

		$useTrigger=NULL;
		if ($globTriggerID) $useTrigger=$globTriggerID;
		if ($triggerOfUser) $useTrigger=$triggerOfUser; // overrule GLOBAl trigger

		if (!$useTrigger)	return;
		
		if ( !gObject_exists ($sqlo, 'MODULE', $useTrigger) ) {
			$error->set( $FUNCNAME, 1, 'MODULE ID:'.$useTrigger. ' does not exist.' );
			return;
		}
		
		return $useTrigger;
	}
	
	/**
	 * get trigger of ABSTRACT-object (VARIO:conc.WorkflowID)
	 * @param $sqlo
	 * @param $tablename ABSTRACT-table name
	 * @param $objid ID of ABSTRACT-object
	 * @return ID of workflow (if defined)
	 * 	  can be NULL or 0
	 */
	static function getWfl_ofAbstract( &$sqlo, $tablename, $objid) {
		if (!$objid) return;
		if ($tablename==NULL) return;
		
		$key = 'conc.WorkflowID';
		$moduleLib = new oS_VARIO_sub($tablename);
		$moduleid  = trim($moduleLib->getValByKey( $sqlo, $objid, $key ));
		
		return $moduleid;
	}
}