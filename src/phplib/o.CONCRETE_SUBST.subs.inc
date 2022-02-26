<?php
require_once ("date_funcs.inc");
require_once ("f.qc_check.inc");
require_once ("o.S_OBJLINK.subs.inc");
require_once ("o.CS_HAS_PR.subs.inc");
require_once ("o.AS_HAS_PR.subs.inc");
require_once ("o.CCT_ACCLOG.subs.inc");
require_once ('o.S_VARIO.subs.inc');

/**
 * sub functions for concrete subst, mainly expiry check
 * @package o.CONCRETE_SUBST.subs.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq   UREQ:0002063: f.object.release.policy > special for o.CONCRETE_SUBST 
 */
class oConcSubst_subC {
    
    const CERT_FLAG_prod    =1;
    const CERT_FLAG_devel   =2;
    const CERT_FLAG_research=3;
    
    var $substFeats;
    var $rel_policy_4eye; /* 
      use 4eye policy?
    	[0],1 : yes
    	-1    : no
    */
    
    function __construct() {
    	$this->qcCheckLib = new fQC_checkC();
    	$this->objLinkLib = new oS_OBJLINK_subs();
    	$this->substFeats = array();
    }
    
    /**
     * init substance
     * get internal array $this->substFeats
     * @param  $sqlo
     * @param  $id
     */
    function initSubst(&$sqlo, $id) {
    	$this->substid = $id;
    	$this->substFeats = array();
    	$this->rel_policy_4eye = 0;
    	
    	
    	$sqlsel = $sqlo->Sql2DateString("expiry_date",4). ", CERT_FLAG, ABSTRACT_SUBST_ID, CCT_ACCESS_ID".
    			" from CONCRETE_SUBST where CONCRETE_SUBST_ID=".$this->substid;
    	$sqlo->Quesel($sqlsel);
    	$sqlo->ReadRow();
    	$this->substFeats['EXPIRY_DATE']       = $sqlo->RowData[0];
    	$this->substFeats['CERT_FLAG']         = $sqlo->RowData[1];
    	$this->substFeats['ABSTRACT_SUBST_ID'] = $sqlo->RowData[2];
    	$this->substFeats['CCT_ACCESS_ID']     = $sqlo->RowData[3];
    	
    	if ($this->substFeats['ABSTRACT_SUBST_ID']) {

    		// get vario-val
    		$varioLib = new oS_VARIO_sub('ABSTRACT_SUBST');
    	   	$this->rel_policy_4eye = $varioLib->getValByKey($sqlo, $this->substFeats['ABSTRACT_SUBST_ID'], 'rel.policy.4eye');
    	   	if (is_numeric($this->rel_policy_4eye)) $this->rel_policy_4eye = intval($this->rel_policy_4eye);
    	}
    	
    	
    }
    
    /**
     * 
     * @param int $expiryUnx
     * @param int $compareDateUnx // optional reference date
     * @return array
     */
    public function check_ExpDateParam( $expiryUnx, $compareDateUnx=NULL ) {
    	
    	if ($compareDateUnx==NULL) $nowDateUnx = time();
    	else {
    		$nowDateUnx = $compareDateUnx;
    	}
    	$unxDiff   = $expiryUnx - $nowDateUnx;
    	$diffFLoat = $unxDiff / (3600*24);
    	
    	$days = round($diffFLoat,2); // do rounding ...
    	return ( array("date"=>$expiryUnx, "days"=>$days) );
    }
    
    /**
     * check, if CERT_FLAG is o.k. for status releasded
     * @swreq UREQ:0001430:004: no error, if (CERT_FLAG=1 otr CERT_FLAG is NULL) 
     */
    function checkCertFlag( ) {
    	$certflag = $this->substFeats['CERT_FLAG'];
    	
    	$ok = 0;
    	if ($certflag==self::CERT_FLAG_prod or !$certflag) $ok=1;
    	
    	return array('ok'=>$ok, 'CERT_FLAG'=>$certflag);
    }
    
    /**
     * check expiry date
     * @param $sql
     * @param $compareDateUnx : optional reference date
     * @return array( "date"=> unixStamp, 
    				  "days"=> >0 o.k.
    						   =<0 expired )
     */
    function checkExpiry( &$sql, $compareDateUnx=NULL ) {
    
    	$expiry_date = $this->substFeats['EXPIRY_DATE'];
    
    	if ($expiry_date=="") {
    		return;
    	}
    	
    	$expiryUnx =  date_YYYYMMDD2unix($expiry_date);
    	$resarr    = $this->check_ExpDateParam($expiryUnx, $compareDateUnx);
    	return ( $resarr );
    }
    
    
    
    /**
     * 4-Augen-Prinzip : es muss vorher ein anderer Mitarbeiter einen Status eingestellt haben
        erlaubt: Suche letzten Status: "qc_checked", "produced", "reviewed" (seit 2013-01-15)
     * @swreq 0002063:006: 4-Augen-Prinzip
     * @param  $sqlo
     * @return array($okflag, $comment)
     * 	 $okflag:
     * 		0 : o.k.
     * 	   >0 : error
     */
    function pol_4EyesPrinciple(&$sqlo) {
        
    	if ( !$this->substFeats['CCT_ACCESS_ID'] ) {
    		sys_error_my('method pol_4EyesPrinciple() not initialized.');
    	}
    	
    	if ( $this->rel_policy_4eye<0 ) {
    		// @swreq 0002063:006:a : kann abgeschalten werden f�r eine SUA: setze vario-variable:"rel.policy.4eye"=-1
    		return array(0,'Four-eyes principle: switched off by SUA.');
    	}
    	
    	$goodStates = array('QC checked','produced','reviewed');
    	$audName2ID=array();
    	$table='CONCRETE_SUBST';
    	foreach( $goodStates as $key) {
    	    $audName2ID[$key] = oH_ALOG_ACT_subs::getH_ALOG_ACT_ID( $sqlo, $table, $key );
    	}
    	
    	
    	
    	// search from end of status queue
    	$sqlsel = "POS, H_ALOG_ACT_ID, DB_USER_ID  from CCT_ACCLOG where CCT_ACCESS_ID=".$this->substFeats['CCT_ACCESS_ID']. 
    			  " order by POS DESC";
    	$sqlo->Quesel($sqlsel);
    	$statusFound=0;
    	while ($sqlo->ReadRow() ) {
    		$H_ALOG_ACT_ID = $sqlo->RowData[1];	
    		$userid        = $sqlo->RowData[2];	
    		
    		if ( in_array($H_ALOG_ACT_ID, $audName2ID) ) {
    			$statusFound=1;
    			break;
    		}
    	}
    	
    	//$okflag=0; // o.k.
    	
    	if (!$statusFound) {
    		return array(-1,'Four-eyes principle: reviewer missing. (4-Augen-Prinzip)');
    	}
    	
    	if ( $_SESSION['sec']['db_user_id']==$userid ) {
    		return array(-2,'Four-eyes principle: need an other person for release. (4-Augen-Prinzip)');
    	}
    	return array(0,'');
    }

}


// ----------------------------------
/**
 * mother class summarizes results of Prep-Protocol validation
 */
class ocSUC_prepProtoMeta {
	
	var $XTYPE_PREP=1; // defined for QC
	
	function __construct( &$sqlo) {
		$this->qcCheckLib = new fQC_checkC();
		$this->AS_HAS_PR_lib   = new oAS_HAS_PR_subs();
		$this->CS_HAS_PR_lib   = new oCS_HAS_PR_subs();
		$this->objLinkLib      = new oS_OBJLINK_subs();
	}
	
	/**
	 * is substance an aliquot ?
	 * @return int 0,1
	 */
	function _isAliquot(&$sqlo) {
		
		$answer = $this->objLinkLib->getMoObjbyKey($sqlo, 'CONCRETE_SUBST', $this->substid, 'ERPchild'); 
		if ($answer[1]>0) return 1;
		else return 0;
	}
	
	/**
	 * - analyse expected Prep-protocols
	 * @param $options
	 *   'onlyAuditStatus' : 0,1 if 1: check only the AUDIT-Status of the protocols !
	 *   'CERT_FLAG' : [1], 2,3,4 : check policy for this defined Life cycle Flag
	 * @return array( status, text) 
	 * 	$status: 
	 * 		0 :o.k., 
	 * 		<0 : error
	 * 		-1 : general problem
	 * 		-2 : QC-Status not reached
	 *  	-3 : mandatory fields missing
	 */
	function checkPrepProtos( &$sqlo, $cSubstID, $aSubstID, $options=NULL ) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$this->substid = $cSubstID;
		
		$this->AS_HAS_PR_lib->setObj($aSubstID);
		$this->CS_HAS_PR_lib->setObj($cSubstID);
		
		// get only QC-Protocols
		$cp_array = $this->CS_HAS_PR_lib->getProtoLog($sqlo, $this->XTYPE_PREP);
		$ap_array = $this->AS_HAS_PR_lib->getProtoLog($sqlo, $this->XTYPE_PREP);
		
		$qcprotos = $this->CS_HAS_PR_lib->mergeProtoArr($cp_array, $ap_array);
			
		if ( empty($qcprotos) ) {
			return array(0,NULL); // nothing 
		} 
		
		// check aliquots
		if ( sizeof($ap_array) and !sizeof($cp_array) ) {
			$isAliquot = $this->_isAliquot($sqlo);
			if ($isAliquot) {
				// @swreq UREQ:0001430:002.b: if substance is aliquot ? no need of prep-protoco
				return array(0,NULL); // o.k.
			}
		}
		
		$accLogSub = new oAccLogC();
		
		$doCheckDeep=1;
		if ($options['onlyAuditStatus']) $doCheckDeep=0;
	
		$statusAll = 0;
		$text     =NULL;
		$tmpkomma =NULL;
		$chopt    =array( 'CERT_FLAG'=>$options['CERT_FLAG'] );
		$p_table = 'CONCRETE_PROTO';
		
		
		foreach( $qcprotos as $oneProtoArr) {
			// get concrete proto for this substance
			do {
				
				$apid = $oneProtoArr['ap'];
				$cpid = $oneProtoArr['cp']; // ID of CONCRETE_PROTO
				
				if ($apid and !$cpid) {
					 $statusAll = -1;
					 $text .= $tmpkomma . 'protocol missing (expected PRA-ID:'.$apid.').';
					 break;
				}
				
				// check audit status
				$CCT_ACCESS_ID = glob_elementDataGet( $sqlo, $p_table, 'CONCRETE_PROTO_ID', $cpid, 'CCT_ACCESS_ID');
				$accLogSub->setObject( $sqlo, $p_table, $cpid, $CCT_ACCESS_ID );

				if (!$accLogSub->currentIsReleased($sqlo)) {
					$statusAll = -2;
					$text .= $tmpkomma . 'PRC-ID:'.$cpid.': not "released".';
					$tmpkomma = '; ';
				}
				
				if ($doCheckDeep) {
					// returns: array('all'=>$statusAll, 'text'=>qcProblem) 
					$oneResult = $this->qcCheckLib->protoCheckBase( $sqlo, $cpid, NULL, $chopt );
					$statusAll = min($statusAll, $oneResult['all']);
					$text .= $tmpkomma . $oneResult['text'];
				}	
				
			} while (0);
			
			if ($text!=NULL) $tmpkomma = '; ';
		}
		
		

		return array( $statusAll, $text );
	}
}

