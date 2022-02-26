<?php
require_once 'o.WORKLIST.subs.inc';

// $Header: trunk/src/phplib/o.WORKLIST.policy_after.inc 59 2018-11-21 09:04:09Z $
/**
 * actions after status change
 * called by e.g. www/lab/pol.objRelease.inc
 * @package o.WORKLIST.policy_after.inc 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq   SREQ:0003319: o.WORKLIST > status policy 
 */
class oWLI_policyAfter {
	private $infoarr;
	
	public function __construct() {
		$this->infoarr = NULL;
		$this->lockObj      = new gObjaccLock();
		$this->accLogLib    = new oAccLogC();
	}
	
	/**
	 * lock one experiment
	 * 
	 */
	private function _lockExp(&$sqlo, $expid) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$accLogState = oH_ALOG_ACT_subs::getH_ALOG_ACT_ID( $sqlo, 'WORKLIST', 'released' );
		if (!$accLogState) {
			$error->set( $FUNCNAME, 1, 'status "released" does not exist.' );
			return;
		}
		
		$this->lockObj->accessLockOne( $sqlo, 'EXP', $expid);
		$argu=array("action"=>$accLogState);
		$this->accLogLib->addLogObj( $sqlo, 'EXP', $expid, $argu );
	}
	
	/**
	 * lock all open Experiments
	 * - get exp columns
	 * - get all experiments
	 * - check, if locked
	 * @param  $sqlo
	 * @param  $objid
	 * @return number of locked objects
	 */
	public function lockSubObjects( &$sqlo, $objid) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
		$wliLib = new oWORKLIST_subs();
		$wliLib->setObj($objid);
		
		$exparr = $wliLib->getAllDataOfType($sqlo, 'EXP');
		
		if (!sizeof($exparr)) {
			$this->infoarr[] = 'no experiments in worklist.';
			return 0;
		}
		
		// check access
		$cntExpClose=0;
		reset ($exparr);
		foreach( $exparr as $dummy=>$expid) {
			$CCT_ACCESS_ID = glob_elementDataGet($sqlo, 'EXP', 'EXP_ID', $expid, 'CCT_ACCESS_ID');
			$isOpen = access_hasManiRights( $sqlo, $CCT_ACCESS_ID );
			
			if ($isOpen) {
				$this->_lockExp($sqlo, $expid); 
				if ($error->Got(READONLY))  {
					$error->set( $FUNCNAME, 1, 'Problem on closing EXP; EXP-ID:'.$expid );
					return;
				}
				$cntExpClose++;
			}
			
			
		}
		
		if ( $_SESSION["userGlob"]["g.debugLevel"]>0 ) {
			$tmpinfo = $this->lockObj->getinfo();
			glob_printr( $tmpinfo, $FUNCNAME.": lockingInfo" );
		}
		
		
		return $cntExpClose;
	} 
}
