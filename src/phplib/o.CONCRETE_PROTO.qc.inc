<?php
require_once ("o.CONCRETE_SUBST.subs.inc");
require_once ('o.CCT_ACCLOG.subs.inc');
require_once ("o.ABSTRACT_SUBST.qcprot.inc");
require_once ("o.ABSTRACT_SUBST.subs.inc");
require_once ("o.ACCEPT_PROT.subs.inc");

/**
 * sub-methods for concrete protocol : QC tests for PREP/QC-Protocols
 * - collects all subtests, which can be done for a protocol
 * @package o.CONCRETE_PROTO.qc.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq SREQ:0001723: g > qc-policy for QC-protocols 
 * @swreq UREQ:0001430: g > pol.objRelease special LAB-policies 
 * $Header: trunk/src/phplib/o.CONCRETE_PROTO.qc.inc 59 2018-11-21 09:04:09Z $
 */
class oConcProto_qc {
	
var $accLogState;// cache for audit state name
var $accProID;   // ACCEPT_PROT_ID of concrete_proto 
var $apid; 		 // ABSTRACT_PROTO_ID
var $cpid; 		 // CONCRETE_PROTO_ID
var $StatusStepNr; // STATUS step number for QC-Protocols

function __construct($infolevel=1) {
	$this->substExpiryLib = new oConcSubst_subC();
	$this->accLogLib      = new oAccLogC();
	$this->CLASSNAME = "oConcProto_qc";
	$this->infolevel = $infolevel; // 0,1,2
	$this->infoarr  = NULL;
	$this->absSubstLib = NULL;
	$this->accLogState = array();
}

/**
 * set object
 * @param object $sql
 * @param int $id
 * @param int $typex 1:PREP, 2:QC
 * @param array $propt
 * 	 ['CERT_FLAG'] : [1],2,3,4
 */
function setProtoID( &$sql, $id, $typex, $propt=NULL ) {
	// - reset the lib
	// - do first checks
	global $error;
	$FUNCNAME = $this->CLASSNAME.":setProtoID";
	
	$PROTO_TYPE_QC=2;
	$this->infoarr     = NULL;
	$this->cpid        = $id;
	$this->execDateUnx = NULL;
	$this->apid  	   = NULL;
	$this->StatusStepNr= 0;
	$this->propt 	   = $propt;
	
	if ($this->absSubstLib===NULL) $this->absSubstLib    = new oAbsSubstQCprot($sql);
	
	$sqls = "select ".$sql->Sql2DateString("EXEC_DATE",5).", CCT_ACCESS_ID, ABSTRACT_PROTO_ID, ACCEPT_PROT_ID from CONCRETE_PROTO ".
			"where CONCRETE_PROTO_ID=".$this->cpid;
	$sql->query($sqls, $FUNCNAME);
	$sql->ReadRow();
	$execDate        = $sql->RowData[0];
	$cct_access_id   = $sql->RowData[1];
	$this->apid		 = $sql->RowData[2];
	$this->accProID  = $sql->RowData[3]; 
	
	if ($execDate!="") $this->execDateUnx = date_YYYYMMDD2unix($execDate);
	else {
		// FALLBACK: get from cct_access_id
		$sqls = "select ".$sql->Sql2DateString("CREA_DATE",5)."  from CCT_ACCESS ".
			"where CCT_ACCESS_ID=".$cct_access_id;
		$sql->query($sqls, $FUNCNAME);
		$sql->ReadRow();
		$execDate  = $sql->RowData[0];
		$this->execDateUnx = date_YYYYMMDD2unix($execDate);
	}
	
	if ($typex==$PROTO_TYPE_QC) {
		$aProtoID = $this->apid;
		$step_nr  = $this->absSubstLib->getStatusStepNr($sql, $aProtoID);
		if (!$step_nr) {
			$error->set( $FUNCNAME, 1, 'no Status-Step-Nr found.' );
			return;
		}
		$this->StatusStepNr = $step_nr;
	}
	
	if ($execDate=="") {
		$error->set("setProtoID", 1, "EXEC_DATE is missing");
	}
	
}

/**
 * - check expiry-date of substances from protocol
   - reference date is the EXEC_DATE of protocol
 * @param object $sql
 * @return $expiryArr = array( ID => days )
 */
function checkSubstExpiry(&$sql) {
	
	$FUNCNAME= "checkSubstExpiry";
	
	$expiryArr = NULL;
	$substArr  = NULL;
	$sqls = "select CONCRETE_SUBST_ID from CONCRETE_PROTO_STEP ".
			"where CONCRETE_PROTO_ID=".$this->cpid. " AND CONCRETE_SUBST_ID>0";
	$sql->query($sqls, $FUNCNAME);
	while ( $sql->ReadRow() ) {
		$substArr[] = $sql->RowData[0];
	}
	
	if (empty($substArr)) return; 
	
	foreach( $substArr as $substid) {
		$this->substExpiryLib->initSubst($sql, $substid);
		$infoarr = $this->substExpiryLib->checkExpiry( $sql, $this->execDateUnx );
		if ($infoarr!=NULL) {
			
			if ($infoarr["days"]<0) {
				$expiryArr[$substid] = $infoarr["days"];
			} 
		}
	}
	
	return ($expiryArr);
}

/**
 * check status RELEASED of substances from protocol
 * - input: $this->propt['CERT_FLAG']
 * @swreq SREQ:0001723:004: checkReleased
 * @return: $numNotReleased  == NULL means o.k.
 *	   					> 0   qc_problem
 */
function checkReleased(&$sql) {
    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	$table='CONCRETE_PROTO';
	debugOut('(140) check_SUC_released', $FUNCNAME, 2);
	
	$CERT_DEV_FLAG = 2;
	
	$acc_ids = NULL;
	$sqls = "select c.CCT_ACCESS_ID, c.CONCRETE_SUBST_ID ".
			" from CONCRETE_PROTO_STEP s JOIN CONCRETE_SUBST c ON c.CONCRETE_SUBST_ID=s.CONCRETE_SUBST_ID".
			" where s.CONCRETE_PROTO_ID=".$this->cpid. " AND s.CONCRETE_SUBST_ID>0";
	$sql->query($sqls, $FUNCNAME );
	while ( $sql->ReadRow() ) {
		$acc_ids[] =  array($sql->RowData[0],$sql->RowData[1]);
	}
	
	if ( $acc_ids==NULL ) return;
	
	if ( !isset($this->accLogState["released"]) ) {
	    $this->accLogState["released"] = oH_ALOG_ACT_subs::getH_ALOG_ACT_ID( $sql, $table, "released" );
	}
	if ( !isset($this->accLogState["finished"]) ) {
	    $this->accLogState["finished"] = oH_ALOG_ACT_subs::getH_ALOG_ACT_ID( $sql, $table, "finished" );
	}
	if ( !isset($this->accLogState["D_released"]) ) {
	    $this->accLogState["D_released"] = oH_ALOG_ACT_subs::getH_ALOG_ACT_ID( $sql, $table, "D_released" );
		if ( !$this->accLogState["D_released"] ) {
			$this->accLogState["D_released"] = 9999; // fallback
		}
	}
	
	if ( !$this->accLogState["released"] ) {
		return; // TBD: error
	}
	$numNotReleased=0;
	$substids = NULL;
	foreach( $acc_ids as $accarr) {
	    
		$acc_idx = $accarr[0];
		$suc_idx = $accarr[1];
		
		$sqls = "select H_ALOG_ACT_ID from CCT_ACCESS where CCT_ACCESS_ID=".$acc_idx;
		$sql->query($sqls);
		$sql->ReadRow();
		$this_statusID =  $sql->RowData[0];
		
		// do checks
		do {
			if ($this_statusID == $this->accLogState["released"] ) {
				break; // o.k.
			}
			if ($this->propt['CERT_FLAG']==$CERT_DEV_FLAG) {
				// check "D_released"
				if ($this_statusID == $this->accLogState["D_released"] ) {
					break; // o.k.
				}
			}
			
			// if is finished ... may be before released
			if ($this_statusID == $this->accLogState["finished"]) {
				// get status before
				
			    $this->accLogLib->setObject( $sql, $table, $suc_idx, $acc_idx );
				$allEntries = $this->accLogLib->getAllEntries($sql);
				if (sizeof($allEntries)>=2) {
					// prior status exists
					end ($allEntries); // last element (finished)
					$priorStatus = prev($allEntries); // rewind one
					if ($priorStatus == $this->accLogState["released"]) {
						break; // o.k. was released
					}
					if ($this->propt['CERT_FLAG']==$CERT_DEV_FLAG) {
						if ($priorStatus == $this->accLogState["D_released"] ) {
							break; // o.k.
						}
					}
				}
			}
			
			$numNotReleased++;
			if ($this->infolevel>1) {
				$substids[] = $accarr[1];
			}
		} while (0);
		
	}
	reset ($acc_ids); 
	if ($this->infolevel>1 and sizeof($substids)) 
		$this->infoarr[] = 'Non-released substance-IDs: '.implode(', ',$substids);
	
	return ($numNotReleased);
}

/**
 * - check if CONCRETE_SUBST_ID is missing in protocol steps
   - if !CONCRETE_SUBST_ID AND  NOT_DONE=1 : o.k. (step invalid)
 * @param object $sql
 * @return return: array($errflag, $numOfMissingSteps)
			   NULL : o.k.
 */
function checkConSubstMiss(&$sql) {
    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
    debugOut('(239) checkConSubstMiss', $FUNCNAME, 2);
    
	$stepnr=array();
	$sqls = "select STEP_NR from ABSTRACT_PROTO_STEP ".
			"where ABSTRACT_PROTO_ID=".$this->apid. " AND ABSTRACT_SUBST_ID>0 order by STEP_NR";
	$sql->query($sqls);
	while ( $sql->ReadRow() ) {
		$stepnr[] = $sql->RowData[0];
	}
	if ( empty($stepnr) ) return;
	
	$numAbsSteps = sizeof($stepnr);
	// analyze substance-relevant steps
	$sqls = "select count(1) from CONCRETE_PROTO_STEP ".
			" where CONCRETE_PROTO_ID=".$this->cpid. " AND".
			" (CONCRETE_SUBST_ID>0 OR NOT_DONE=1)".
			" AND STEP_NR in (".implode(",",$stepnr).")";
	$sql->query($sqls);
	$sql->ReadRow();
	$numConcSubst = $sql->RowData[0];
	if ($numConcSubst<$numAbsSteps) {
		return array(1, $numAbsSteps-$numConcSubst);
	}
	return;
}



/**
 * - get STATUS value (only QC-Protocols)
 * @swreq SREQ:0001723:SUBREQ:001 check STATUS-value: 0, 0.5, 1 
 * @param $sqlo
 * @return int 
 * 		-1 : not set
 * 		 0
 * 		 0.5
 * 		 1
 * 
 */
function getStatusVal(&$sqlo) {
	
	
	$statusVal = -1;
	$cProtoID  = $this->cpid;
	$sqlsel = "QUANTITY from CONCRETE_PROTO_STEP".
			" where CONCRETE_PROTO_ID=".$cProtoID. ' and STEP_NR='.$this->StatusStepNr;
	$sqlo->Quesel($sqlsel);
	$sqlo->ReadRow();
	$statusVal = $sqlo->RowData[0];
	if ($statusVal===NULL) $statusVal=-1;
	return ($statusVal);
}

/**
 * get mandatory value problems
 * @swreq SREQ:0001723:SUBREQ:002: check mandatory values 
 * @param $sqlo
 * @return array ('err'=>0,1, 'info'=>text)
 */
function getMandProblem(&$sqlo) {
    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	$aProtoID = $this->apid;
	$cProtoID = $this->cpid;
	if (!$aProtoID) return;
	
	debugOut('(303) check_Mandatory_values', $FUNCNAME, 2);
	
	$stepNrInfo   = $this->absSubstLib->getMandatoryInfo($sqlo, $aProtoID);
	
	if (!is_array($stepNrInfo)) {
		return array('err'=>0, 'info'=>NULL);
	}
	
	$stepNrArr = array_keys($stepNrInfo);
	
	$sqlsel = "STEP_NR, QUANTITY, CONCRETE_SUBST_ID, DEV_ID, NOT_DONE, NOTES from CONCRETE_PROTO_STEP".
			  " where CONCRETE_PROTO_ID=".$cProtoID. ' and STEP_NR in('.implode(',',$stepNrArr).')';
	$sqlo->Quesel($sqlsel);
	$csteps=NULL; /** array of key/value
		'i' : > 0 step is inactive
		's' : substance id
		'd' : device id
		'q' : quantity
		'n' : notes
	*/
	while ($sqlo->ReadRow() ) {
		if ($sqlo->RowData[4]>0) {
			// step is inactive
			$csteps[$sqlo->RowData[0]] = array('i'=>1);
		} else {
			if ($sqlo->RowData[5]!=NULL) $notes='X'; // pseudo notes
			else $notes=NULL;
			$csteps[$sqlo->RowData[0]] = array('s'=>$sqlo->RowData[2], 'q'=>$sqlo->RowData[1], 'd'=>$sqlo->RowData[3], 'n'=>$notes);
		}
		
	}
	
	$errcnt    =0; // number of errors
	$lastErrcnt=0;
	$badSteps  =array(); // store array of bad step_nr
	
	reset ($stepNrInfo);
	foreach( $stepNrInfo as $stepnr=>$keys) {
		
		if ($csteps[$stepnr]['i']>0) {
			continue; // step is inactive
		}
		if (strstr($keys,'s')!=NULL) {
			if (!$csteps[$stepnr]['s']) $errcnt++;
		}
		if (strstr($keys,'d')!=NULL) {
			if (!$csteps[$stepnr]['d']) $errcnt++;
		}
		if (strstr($keys,'q')!=NULL) {
			if ($csteps[$stepnr]['q']===NULL) $errcnt++;
		}
		if (strstr($keys,'n')!=NULL) {
			if ($csteps[$stepnr]['n']===NULL) $errcnt++;
		}
		if ($lastErrcnt!=$errcnt) $badSteps[]=$stepnr;
		$lastErrcnt=$errcnt;
	}
	if (!$errcnt) return array('err'=>0, 'info'=>NULL);
	
	// Step_no:'.implode(', ',$badSteps)
	return array('err'=>1, 'info'=>'missing ' .$errcnt. ' mandatory values.');
}

/**
 * get Limit-problems, derived by ACCEPT_PROT
 * @swreq SREQ:0001723:003: check limits (defined in ACCEPT_PROT)
 * @param $sqlo
 * @return array ('err'=>0,1, 'info'=>text)
 */
function getLimitProblem(&$sqlo) {
    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
    
	$cProtoID = $this->cpid;
	$accProID = $this->accProID;
	debugOut('(372) getLimitProblem: accProID:'.$accProID, $FUNCNAME, 2);
	
	if (!$accProID) return; // no check needed
	
	$accProtLib = new oACCEPT_PROT_prc($sqlo, $cProtoID);
	$result = $accProtLib->getAllLimitProblems($sqlo);
	
	return $result;
}


/**
 * get advanced info-array
 */
function getInfoArray() {
	return $this->infoarr;
}

}

/**
 * Non-QM-Tests
 * @author steffen
 * @swreq SREQ:0001032: o.CONCRETE_PROTO > update step > check if concrete object is derrived from abstract object
 *
 */
class oCONCRETE_PROTO_qc2 {

	/**
	 * initialize CONCRETE_PROTO
	 * @param object $sqlo
	 * @param int $cpid
	 * @param int $apid
	 */
	public function setCpid(&$sqlo, $cpid, $apid=0) {
		$this->cpid=$cpid;
		
		if (!$apid) {
			$protoFeats = glob_elemDataGet3( $sqlo, 'CONCRETE_PROTO', array('CONCRETE_PROTO_ID'=>$cpid), 
				array('ABSTRACT_PROTO_ID') );
			$apid = $protoFeats['ABSTRACT_PROTO_ID'];
		}
		
		$this->apid=$apid;
	}
	
	/**
	 * - check, if planned SUA is the same like used SUC=>SUA
	 * - detect the detailed steps for this
	 * @swreq  UREQ:0001032 check if concrete object is derived from abstract object
	 * @param  $sqlo
	 * @return array('err'=>$errflag, 'txt'=>$text)
	 * 		$errflag > 0 : error
			NULL : o.k.
	 */
	function absVersConcSubst(&$sqlo) {
		$cpid  = $this->cpid;
		
		if (!$cpid or !$this->apid) {
			sys_error_my('Class not initialized.');
		}
		
		//$stepnr= NULL;
		$apids = NULL;
		$badsua= array(); // BAD SUAs : array( 'suc'=>SUC_ID, 'suca'=>SUA of SUC, 'suap'=>planned SUA );
		$sqls  = "select STEP_NR, ABSTRACT_SUBST_ID from ABSTRACT_PROTO_STEP ".
				"where ABSTRACT_PROTO_ID=".$this->apid. " AND ABSTRACT_SUBST_ID>0 order by STEP_NR";
		$sqlo->query($sqls);
		// collects SUAs
		while ( $sqlo->ReadRow() ) {
			// $stepnr[] = $sql->RowData[0];
			$apids[$sqlo->RowData[0]]  = $sqlo->RowData[1];
		}
		if ( !sizeof($apids) ) return; // no planned steps

		
		// analyze substance-relevant steps: NR and SUC and SUA of SUC
		// take all steps, even non-planned (gives no error)
		// ignore NOT_DONE==1
		// OLD: " AND st.STEP_NR in (".implode(",",$stepnr).")"
		$sqls = "select st.STEP_NR, st.CONCRETE_SUBST_ID, cs.ABSTRACT_SUBST_ID ".
			" from CONCRETE_PROTO_STEP st join CONCRETE_SUBST cs ".
			" on st.CONCRETE_SUBST_ID=cs.CONCRETE_SUBST_ID".
			" where st.CONCRETE_PROTO_ID=".$cpid. " AND".
			" (st.CONCRETE_SUBST_ID>0 and (st.NOT_DONE=0 or st.NOT_DONE=2 or st.NOT_DONE is NULL))";
			
		$sqlo->query($sqls);
		while ($sqlo->ReadRow() ) {
			$st    = $sqlo->RowData[0];
			$sucid = $sqlo->RowData[1];
			$suaid = $sqlo->RowData[2];
			
			if ( $apids[$st]!= $suaid ) {
				$badsua[$st] = array('suc'=>$sucid, 'suca'=>$suaid, 'suap'=>$apids[$st]);
			}
		}
		
		if ( !empty($badsua) ) {
			// post analysis: is planned SUA a META-SUA ?
			
			$info = NULL;
			$suaAnaLib = new oABSTRACT_SUBST_subs();
			$komma = NULL;
			
			$stkeys     = array_keys($badsua);
			
			// glob_printr( $badsua, "absVersConcSubst:badsua info" );
			
			foreach( $stkeys as $st) {
				$valarr = $badsua[$st];
				if ( $valarr['suap'] and $valarr['suca']) {
					$isChild = $suaAnaLib->suaIsChild($sqlo, $valarr['suap'], $valarr['suca']);
					if ($isChild) {
						unset ($badsua[$st]); // no conflict
						continue;
					}
				}
				
				$html1 = obj_nice_name( $sqlo, 'ABSTRACT_SUBST', $valarr['suap'] );
				$html2 = obj_nice_name( $sqlo, 'ABSTRACT_SUBST', $valarr['suca'] ); 
				$info .= $komma . 'Step:'.$st.' <span style="color:gray;">planned SUA-ID:</span>'.$valarr['suap'].
					' ('.$html1.'); <span style="color:gray;">used SUA-ID:</span>'.$valarr['suca'].' ('.$html2.')';
				$komma='<br />';
			}
			
			if ($info!=NULL) {
				return array('err'=>1, 'txt'=>$info);
			}
			 
		}
		
		return;
	}
}
