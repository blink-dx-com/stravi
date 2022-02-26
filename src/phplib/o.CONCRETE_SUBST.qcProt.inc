<?php
require_once ("o.CS_HAS_PR.subs.inc");
require_once ("o.AS_HAS_PR.subs.inc");
require_once ("o.CCT_ACCLOG.subs.inc");
require_once ('o.CONCRETE_PROTO.qc.inc');

// $Header: trunk/src/phplib/o.CONCRETE_SUBST.qcProt.inc 59 2018-11-21 09:04:09Z $
 
/**
 * mother class summarizes results of ocSUC_qcProto
 * QC protocol analysis
 * - changes for DB-version: 1.0.4.9 
 * @package o.CONCRETE_SUBST.qcProt.inc
 * @swreq   SREQ:0001723: g > qc-policy for QC-protocols 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class ocSUC_qcProtoMeta {
	
	var $XTYPE_QC=2; // defined for QC
	
	function __construct( &$sqlo) {
		$this->worklib = new oConcProto_qc( $sqlo );
		
		$this->AS_HAS_PR_lib   = new oAS_HAS_PR_subs();
		$this->CS_HAS_PR_lib   = new oCS_HAS_PR_subs();
		
		$this->_debug = $_SESSION["userGlob"]["g.debugLevel"];
	}
	
	/**
	 * check one protocol
	 * @param $sqlo
	 * @param $cpid
	 * @return array ('all'=>$statusAll, 'text'=>$text, 'qcval'=>$qcval )
	 */
	function checkOneCproto(&$sqlo, $cpid) {
		global $error;
		
		$qcval    = 1; // the quality value
		$text     =NULL;
		$tmpkomma =NULL;
		
		$this->worklib->setProtoID($sqlo, $cpid, 2);
		if ($error->Got(READONLY))  {
			$errLast   = $error->getLast();
			$error_txt = $errLast->text;
			$error->reset();
			$statusAll  = -1;
			$text .= $tmpkomma . 'error got on analysis of protocol PRC-ID:'.$cpid.
				'; details: '.$error_txt;
			$tmpkomma=', ';
			return array('all'=>$statusAll, 'text'=>$text, 'qcval'=>$qcval);
		} 
		
		$statusVal  = $this->worklib->getStatusVal($sqlo);
		// @swreq SREQ:0001723:SUBREQ:001 check STATUS-value: 0, 0.5, 1 
		if ($statusVal<0.5) {
			$statusAll = -2; // QC not passed
			$text .= $tmpkomma . 'Protocol PRC-ID:'.$cpid.': QC not passed (STATUS must be 1)';
			$tmpkomma=', ';
		} else {
			$qcval = min($statusVal,$qcval);
		}
		
		// @swreq SREQ:0001723:SUBREQ:002: check mandatory values 
		$mandInfo = $this->worklib->getMandProblem($sqlo);
		if ($mandInfo['err']>0) {
			$statusAll = -3; // mandatory problem
			$text .= $tmpkomma . 'MandErr: '. $mandInfo['info'];
			$tmpkomma=', ';
		}
		
		
		// @swreq SREQ:0001723:003: check limits (defined in ACCEPT_PROT)
		$limitInfo = $this->worklib->getLimitProblem($sqlo);
		if ($limitInfo['err']>0) {
			$statusAll = -4; // mandatory problem
			$text .= $tmpkomma . 'LimitErr: '. $limitInfo['info'];
			$tmpkomma=', ';
		}
		
		return array('all'=>$statusAll, 'text'=>$text, 'qcval'=>$qcval);
	}
	
	/**
	 * - check substance QC-protocols
	 * - analyse expected QC-protocols
	 * - QC-protocol with last-step-value < 0.5 : NOT PASSED
	 * @return array( status, text, $qcval) 
	 * 	$status: 
	 * 		0 :o.k., 
	 * 		<0 : error
	 * 		-1 : general problem
	 * 		-2 : QC-Status not reached
	 *  	-3 : mandatory fields missing
	 *   
	 * 	$qcval: resulting STATUS-value of the protocol: 0, 0.5, 1
	 */
	function checkQCProto( &$sqlo, $cSubstID, $aSubstID ) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$p_table='CONCRETE_PROTO';
		$this->AS_HAS_PR_lib->setObj($aSubstID);
		$this->CS_HAS_PR_lib->setObj($cSubstID);
		
		// get only QC-Protocols
		$activeOnly=1; // @swreq:0001723:005 nur die aktiven QC-Protokolle (xorder<10000) werden f�r QC-Check herangezogen
		$cp_array = $this->CS_HAS_PR_lib->getProtoLog($sqlo, $this->XTYPE_QC, $activeOnly);
		$ap_array = $this->AS_HAS_PR_lib->getProtoLog($sqlo, $this->XTYPE_QC);
		
		$qcprotos = $this->CS_HAS_PR_lib->mergeProtoArr($cp_array, $ap_array);
			
		if ( !sizeof($qcprotos) ) {
			return; // nothing 
		} 
	
		$statusAll = 0;
		$qcval_all = 1; // start with 1, than decrease, if errors occure
		
		$text     =NULL;
		$tmpkomma =NULL;
		$accLogSub = new oAccLogC();
		
		if ($this->_debug>1) {
			reset ($qcprotos);
			glob_printr( $qcprotos, "DEBUG:".$FUNCNAME.": qcprotos" );
		}
		
		reset ($qcprotos);
		foreach( $qcprotos as $dummy=>$oneProtoArr) {
			
			$statusLoop = 0;
			do {
				
				$apid = $oneProtoArr['ap'];
				$cpid = $oneProtoArr['cp']; // ID of CONCRETE_PROTO
				
				if ($apid and !$cpid) {
					 $statusLoop = -1;
					 $text .= $tmpkomma . 'protocol missing (expected PRA-ID:'.$apid.').';
					 break;
				}
				
				// check audit status
				$CCT_ACCESS_ID = glob_elementDataGet( $sqlo, $p_table, 'CONCRETE_PROTO_ID', $cpid, 'CCT_ACCESS_ID');
				$accLogSub->setObject( $sqlo, $p_table, $cpid, $CCT_ACCESS_ID );
				if (!$accLogSub->currentIsReleased($sqlo)) {
					$statusLoop = -2;
					$text .= $tmpkomma . 'PRC-ID:'.$cpid.': not "released".';
					$tmpkomma = '; ';
				}
				
				$oneResult = $this->checkOneCproto($sqlo, $cpid);
				$statusLoop = min($statusLoop, $oneResult['all']);
				$qcval_all = min($qcval_all,$oneResult['qcval']);
				
				if ($oneResult['text']!=NULL) $text .= $tmpkomma . $oneResult['text'];

			} while (0);
			
			$statusAll = min($statusAll, $statusLoop); // summary of problems
			
			if ($text!=NULL) $tmpkomma = ', ';
		}
		reset ($qcprotos); 
		
		if ($statusAll<0) $qcval=0;
		// $text .= $tmpkomma . sizeof($qcprotos).' protocols analysed';
		return array( $statusAll, $text, $qcval );
	}
	
	/**
	 * easy check for status "QC checked"
	 * - test, if all QC-protocols exist and are "released"
	 * @param $sqlo
	 * @param $cSubstID
	 * @param $aSubstID
	 * @return  array( 'txt'=>$qc_infoText, 'err'=>$qc_problem );
	 * 	  $qc_problem : 
	 * 		0 : o.k.
	 * 		>0: error
	 */
	function checkForQCCheck(&$sqlo, $cSubstID, $aSubstID) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$this->AS_HAS_PR_lib->setObj($aSubstID);
		$this->CS_HAS_PR_lib->setObj($cSubstID);
		
		// get only QC-Protocols
		$activeOnly=1; // @swreq:0001723:005 nur die aktiven QC-Protokolle (xorder<10000) werden f�r QC-Check herangezogen
		$cp_array = $this->CS_HAS_PR_lib->getProtoLog($sqlo, $this->XTYPE_QC, $activeOnly);
		$ap_array = $this->AS_HAS_PR_lib->getProtoLog($sqlo, $this->XTYPE_QC);
		
		$qcprotos = $this->CS_HAS_PR_lib->mergeProtoArr($cp_array, $ap_array);
		
		if ( !sizeof($qcprotos) ) {
			return; // nothing 
		} 
	
		$statusAll = 0;
		
		$errtext     =NULL;
		$tmpkomma =NULL;
		$accLogSub = new oAccLogC();
		
		if ($this->_debug>1) {
			reset ($qcprotos);
			glob_printr( $qcprotos, "DEBUG:".$FUNCNAME.": qcprotos" );
		}
		$p_table='CONCRETE_PROTO';
		
		foreach( $qcprotos as $oneProtoArr) {
			// get concrete proto for this substance
			do {
				
				$apid = $oneProtoArr['ap'];
				$cpid = $oneProtoArr['cp']; // ID of CONCRETE_PROTO
				
				if ($apid and !$cpid) {
					 $statusAll = 1;
					 $errtext .= $tmpkomma . 'protocol missing (expected PRA-ID:'.$apid.').';
					 break;
				}
				
				// check audit status
				$CCT_ACCESS_ID = glob_elementDataGet( $sqlo, $p_table, 'CONCRETE_PROTO_ID', $cpid, 'CCT_ACCESS_ID');
				$accLogSub->setObject( $sqlo, $p_table, $cpid, $CCT_ACCESS_ID );
				if (!$accLogSub->currentIsReleased($sqlo)) {
					$statusAll = 2;
					$errtext .= $tmpkomma . 'PRC-ID:'.$cpid.': not "released".';
					$tmpkomma = '; ';
					break;
				}

			} while (0);
			
			if ($errtext!=NULL) $tmpkomma = ', ';
		}
		
		
		$qcval = 1.0; // default ??? TBD: think about !
		
		return array( 'txt'=>$errtext, 'err'=>$statusAll, 'qcval'=>$qcval );
	}
}

