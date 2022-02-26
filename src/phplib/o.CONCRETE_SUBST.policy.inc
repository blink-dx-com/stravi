<?php

require_once ('access_lock.inc');
require_once ("o.CONCRETE_SUBST.subs.inc");
require_once ('o.CCT_ACCLOG.subs.inc');
require_once ("o.CONCRETE_SUBST.qcProt.inc");

/**
 * QC functions for substance
 * @package o.CONCRETE_SUBST.policy.inc
 * @swreq   UREQ:0002063: f.object.release.policy > special for o.CONCRETE_SUBST 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class oSUC_policy {

    private $features;
    private $substid;

function __construct(&$sqlo) {

	$this->lockObj      = new gObjaccLock();
	$this->accLogLib    = new oAccLogC();
	$this->substQclib   = new oConcSubst_subC();
	$this->cSubstQCLib  = new ocSUC_qcProtoMeta($sqlo);
	$this->cSubstPRLib  = new ocSUC_prepProtoMeta($sqlo);
	$this->features=array();
	$table='CONCRETE_SUBST';
	
	$this->accLogState["released"] = oH_ALOG_ACT_subs::getH_ALOG_ACT_ID( $sqlo, $table, "released" );
	$this->accLogState["finished"] = oH_ALOG_ACT_subs::getH_ALOG_ACT_ID( $sqlo, $table, "finished" );
}

function getDefStates() {
	return ($this->accLogState);
}

function setSubst( &$sqlo, $substid ) {
	$this->substid = $substid;
	$this->features = glob_elemDataGet3( $sqlo, 'CONCRETE_SUBST', array('CONCRETE_SUBST_ID'=>$this->substid), array('ABSTRACT_SUBST_ID','CERT_FLAG') );
}


/**
 * provide all possible tests for a SUC; the $qc_tests array defines, which tests are performed
 * @param object $sqlo
 * @param int $substid
 * @param int $aSubstID
 * @param array $qc_tests
 *   'ExpiryDate'
 *   'CERT_flag'
 *   'Prep_proto'
 *   'QC_proto'
 *   '4EyesPrinciple'
 *   'SUA.params' : check some SUA parameters
 *   'SUA.audit'  : check SUA audit trail (FUTURE)
 *   
 * @return number[]|string[]|array([]
 */
private function _check_matrix(&$sqlo, $CERT_FLAG, $qc_tests) {
    global $error;
    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
    
    $substid  = $this->substid;
    $aSubstID = $this->features['ABSTRACT_SUBST_ID'];
    
    if (!$substid or !$aSubstID) {
        sys_error_my( 'Input-parameters missing.' );
    }
    
    $qc_problem  = 0;
    $expiry_flag = 0;
    $errPre      = 'Err: ';
    
    $qc_infoArr=NULL;
    $this->substQclib->initSubst($sqlo, $substid);
    
    $expiryInfo = $this->substQclib->checkExpiry( $sqlo );
    $expirDays  = $expiryInfo["days"];
    
    if ($expiryInfo!=NULL) {
        if ($expirDays<=0) {
            $expiry_flag = 1;
        }
    }
    
    if (in_array('SUA.params', $qc_tests)) { 
        // $aSubstID
        $sua_feats = glob_ObjDataGet( $sqlo, 'ABSTRACT_SUBST', $aSubstID, array('H_USAGE_ID', 'H_UNIT_ID') );
        foreach($sua_feats as $key=>$val) {
            if (!$val) {
                $qc_infoArr[] = $errPre . 'in MAA: value missing for "'. columnname_nice2('ABSTRACT_SUBST', $key) .'"';
                $qc_problem  = 1;
            }
        }
    }
    
    if (in_array('ExpiryDate', $qc_tests)) {
 
        if ($expiryInfo!=NULL) {
            if ($expirDays>0) {
                // NEW: do not put info text to the WARNINGS: $qc_infoArr[] = "<font color=green>$expirDays</font> days";
            } else {
                $qc_infoArr[] = $errPre . "EXPIRED since:".abs($expirDays). " days";
                $qc_problem  = 1;
            }
        } else {
            // @swreq UREQ:0002063:001.a: no expiry-date set
            $qc_infoArr[] = $errPre . "Expiry date missing";
            $qc_problem  = 5;
        }
    }
    
    if (in_array('CERT_flag', $qc_tests)) {
        
        $qcinfArr = $this->substQclib->checkCertFlag();
        
        if ($CERT_FLAG==1) {
            if ($qcinfArr['CERT_FLAG']!=1) {
                $qc_infoArr[] = $errPre . ' "life cycle status" must be 1 for normal release.';
                $qc_problem = 4;
            }
        }
        
        if ($CERT_FLAG==2) {
            if ($qcinfArr['CERT_FLAG']!=1 and $qcinfArr['CERT_FLAG']!=2) { // D_released special
                $qc_infoArr[] = $errPre . ' "life cycle status" must be (1 or 2) for D_released.';
                $qc_problem = 4;
            }
        }

        
        if ($CERT_FLAG==3) {
            // no special CERT check
        }
    }
    
    if (in_array('Prep_proto', $qc_tests)) {
        $prepopt = array('CERT_FLAG'=>$CERT_FLAG);
        $prepProtInf = $this->cSubstPRLib->checkPrepProtos( $sqlo, $substid, $aSubstID, $prepopt );
        if ($prepProtInf[0]<0) {
            $qc_infoArr[] = $errPre . "in Preparation-Proto: ".$prepProtInf[1];
            $qc_problem = 2;
        }
    }
    
    if (in_array('QC_proto', $qc_tests)) {
        
        /**
         *- check substance QC-protocols
         * - analyse expected QC-protocols
         * - QC-protocol with last-step-value < 0.5 : NOT PASSED
         */
        $qcProtInf = $this->cSubstQCLib->checkQCProto( $sqlo, $substid, $aSubstID );
        if ($qcProtInf[0]<0) {
            $qc_problem   = 3;
            $qc_infoArr[] = $errPre .' in QC-Proto: '.$qcProtInf[1];
        }
    }
    
    if (in_array('4EyesPrinciple', $qc_tests)) {
        // @swreq UREQ:0002063:006: 4-Augen-Prinzip
        $infoarr = $this->substQclib->pol_4EyesPrinciple($sqlo);
        if ($infoarr[0]<0) {
            $qc_problem   = 6;
            $qc_infoArr[] = $errPre . $infoarr[1];
        }
    }
    
    $qc_infoText = "";  // initial
    if ($qc_infoArr!=NULL) {
        $qc_infoText = implode('<br />', $qc_infoArr);
    }
    
    return array( 'txt'=>$qc_infoText, 'err'=>$qc_problem, 'expi'=>$expiry_flag, 'qctest'=>$qcProtInf[2] );
}


/**
 * analyse substance for status "released"
 * - return main values of the test
 * - OLD: was method _qcinfo()
 * @return array 'txt'=>$qc_infoText, 'err'=>$qc_problem, 'expi'=>$expiry_flag, 'qctest'=> QC_test_val )
 * 		$qc_errflag : 
 * 			1 : EXPIRED
 * 			2 : Err in Proto:
 * 			3 : QC-protocol error
 * 			4 : CERT_FLAG problem
 *			5 : Expiry date missing
 *			6 : Four-eyes principle problem
 * 			
 */
function policy_released( &$sqlo) {
    
    if (!$this->substid) throw new Exception('Class not initialized.');
	
	$qc_tests= array(
	    'ExpiryDate',
	    'CERT_flag',
	    'Prep_proto',
	    'QC_proto',
	    '4EyesPrinciple',
	    'SUA.params'
	);
	return $this->_check_matrix($sqlo, 1, $qc_tests);
}

/**
 * analyse substance for status "D_released"
 * - return main values of the test
 * @return array 'txt'=>$qc_infoText, 'err'=>$qc_problem, 'expi'=>$expiry_flag, 'qctest'=> QC_test_val )
 * 		$qc_errflag : 
 * 			1 : EXPIRED
 * 			2 : Err in Proto:
 * 			3 : QC-protocol error
 * 			4 : CERT_FLAG problem
 *			5 : Expiry date missing
 *			6 : Four-eyes principle problem
 * 			
 */
function policy_D_released( &$sqlo) {
    if (!$this->substid) throw new Exception('Class not initialized.');
	
	$qc_tests= array(
	    'ExpiryDate',
	    'CERT_flag',
	    'Prep_proto',
	    'QC_proto',
	    'SUA.params'
	);
	return $this->_check_matrix($sqlo, 2, $qc_tests);
}

/**
 * analyse substance for status "R_released"
 * - return main values of the test
 * @return array 'txt'=>$qc_infoText, 'err'=>$qc_problem, 'expi'=>$expiry_flag, 'qctest'=> QC_test_val )
 * 		$qc_errflag :
 * 			1 : EXPIRED
 * 			2 : Err in Proto:
 * 			3 : QC-protocol error
 * 			4 : CERT_FLAG problem
 *			5 : Expiry date missing
 *			6 : Four-eyes principle problem
 *
 */
function policy_R_released( &$sqlo) {
    if (!$this->substid) throw new Exception('Class not initialized.');
    
    $qc_tests= array(
        'ExpiryDate',
        'CERT_flag',
        'QC_proto',
    );
    return $this->_check_matrix($sqlo, 3, $qc_tests);
}

/**
 * check for "QC checked"
 * - get all QC-protocols
 * - check, if released
 * @param $sqlo
 * @param $substid
 * @param $aSubstID
 */
function policy_qcchecked(&$sqlo) {
    if (!$this->substid) throw new Exception('Class not initialized.');
    $qcProtInfo  = $this->cSubstQCLib->checkForQCCheck($sqlo, $this->substid, $this->features['ABSTRACT_SUBST_ID']);
	return array( 'txt'=>$qcProtInfo['txt'], 'err'=>$qcProtInfo['err'], 'qctest'=>$qcProtInfo['qcval'] );
}

function policy_produced(&$sqlo) {
    if (!$this->substid) throw new Exception('Class not initialized.');
    
    if (!$this->features['CERT_FLAG'] or $this->features['CERT_FLAG']==3) {
        // lower CERT_FLAG: no test !
        return array( 'txt'=>'no special test', 'err'=>0, 'qctest'=>1 );
    }
	
	$qc_infoText = NULL;
	$qc_problem  = 0;
	$checkOpts   = array('onlyAuditStatus'=>1); // no deep analysis !!!
	$prepProtInf = $this->cSubstPRLib->checkPrepProtos( $sqlo, $this->substid, $this->features['ABSTRACT_SUBST_ID'], $checkOpts );
	if ($prepProtInf[0]<0) {
		$qc_infoText = "Preparation-Proto: ".$prepProtInf[1];
		$qc_problem  = 2;
	}
	
	return array( 'txt'=>$qc_infoText, 'err'=>$qc_problem, 'qctest'=>1 );
}

}
