<?php

require_once ('access_lock.inc');
require_once ("o.CONCRETE_SUBST.subs.inc");
require_once ('o.CCT_ACCLOG.subs.inc');
require_once ("o.CONCRETE_SUBST.qcProt.inc");
require_once ("o.CONCRETE_SUBST.policy.inc");

/**
 * QC functions for substance: check for INVALID substances
  - Action: Set status from 'Released' to 'Finished' for expired substances
 * @package o.CONCRETE_SUBST.qcInv.inc
 * @swreq UREQ:0002055 o.CONCRETE_SUBST > Tool QC_checker, mehrere Substanzen
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class oConcSubstQcInv {

var $go; // 0,1
var $substX; // save status for a substance for _autofini()

function __construct(&$sqlo, $go) {

	$this->lockObj      = new gObjaccLock();
	$this->accLogLib    = new oAccLogC();
	
	$this->go = $go;
	
	$this->subsPolicyLib = new oSUC_policy($sqlo);
	$table='CONCRETE_SUBST';
	$this->accLogState["released"] = oH_ALOG_ACT_subs::getH_ALOG_ACT_ID( $sqlo, $table, "released" );
	$this->accLogState["finished"] = oH_ALOG_ACT_subs::getH_ALOG_ACT_ID( $sqlo, $table, "finished" );
}

function getDefStates() {
	return ($this->accLogState);
}

function initAutoFini(&$sqlo) {
	global $error;
	$FUNCNAME= "initAutoFini";
	
	$this->action = "autofini";
	
	$accAnswer = oAccLogStaticC::checkManiRole($sqlo, "CONCRETE_SUBST");
	if ($accAnswer[0]!="execute") {
		$error->set( $FUNCNAME, 1, "role problem: ". $accAnswer[1]);
		return;
	}
}

function setSubst( &$sqlo, $substid ) {
	$this->substid = $substid;
	$this->substX  = NULL;
	
}

function _autofini( 
		&$sqlo,
		$manirights   // hasManipulation rights ? 0,1
		) {
		
	// FUNCTION: switch to 'finish'
	// INPUT:   $this->substX
	// RETURN: 0 : nothing
	//		   1 : prepare
	//		   2 : finish done
	global $error;
	$FUNCNAME= "_autofini";
	
	$answer = 0;
	$substX = $this->substX;
	
	if ( $substX["expiry_flag"]>0 AND $substX["status"] != $this->accLogState["finished"] ) {
	
		// check access: lock object
		$tablename = "CONCRETE_SUBST";
		
		if ($manirights) {
			$accModAll =  access__allowed_to_chmod( $sqlo, $this->acc_id);
			if ( $accModAll=="yes" AND $accModAll=="entail" ) {
				 $error->set( $FUNCNAME, 1, "no entail permissions on object." );
				 return (1);
			}
		}
		
		if ( $this->go==1) {
			if ($manirights) {
				$this->lockObj->accessLockOne( $sqlo, $tablename, $this->substid);
			}
			$argu = array("action" => $this->accLogState["finished"] );
			$this->accLogLib->addLogSub( $sqlo, $argu);
			$answer = 2;
		} else {
			$answer = 1;
		}
	} 
	return ($answer);
}
	
/**
 *  - check QC-status
	- manage action "autofini"
 * @param $sqlo
 * @return $retarr
 */
function singleCheck( &$sqlo ) {
			
	$substid = $this->substid;
	
	$sqls = "select c.CCT_ACCESS_ID from CONCRETE_SUBST c".
			" where c.CONCRETE_SUBST_ID=".$substid;
	$sqlo->query($sqls);
	$sqlo->ReadRow();
	$feat1   = $sqlo->RowData;
	$this->acc_id = $feat1[0];
	
			
	$this->accLogLib->setObject($sqlo, 'CONCRETE_SUBST', $substid);
	$manirights = $this->accLogLib->hasManiRights( $sqlo );
	$CCT_ACCLOG_stat = $this->accLogLib->getLastLog($sqlo);
	
	
	
	$this->subsPolicyLib->setSubst($sqlo, $substid);
	$qcinfoArr  = $this->subsPolicyLib->policy_released( $sqlo );
	
	$qcinfo     =$qcinfoArr['txt'];
	$qcproblem  =$qcinfoArr['err'];
	$expiry_flag=$qcinfoArr['expi'];
	$qcTestVal  =$qcinfoArr['qctest'];
	
	$actAnswer = "";
	if ( $this->action=="autofini" ) {
		$this->substX = array( 
			"expiry_flag"=>$expiry_flag, 
			"status"     =>$CCT_ACCLOG_stat 
			);
		$actAnswer = $this->_autofini($sqlo, $manirights);
	}
	
	$retarr = array(
		"manirights"  => $manirights,
		"ACCLOG_stat" => $CCT_ACCLOG_stat,
		"qcinfo"	  => $qcinfo,
		"actAnswer"	  => $actAnswer,
		"qcproblem"	  => $qcproblem,
		);

	
	return ($retarr);
}

}