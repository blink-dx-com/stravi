<?php
require_once ("o.CONCRETE_PROTO.qc.inc");
require_once ("o.CONCRETE_SUBST.qcProt.inc");

/**
 * SILENT and GUI-functions for substance/protocol QC checking
 * @package f.qc_check.inc
 * @swreq UREQ:0001430: g > pol.objRelease special LAB-policies 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/phplib/f.qc_check.inc 59 2018-11-21 09:04:09Z $
 * @unittest EXISTS
 */
class fQC_checkC {
	
var $qcProblem; // string of QC problem



function getQcProblem() {
	return ($this->qcProblem);
}

	
/**
 * - QC checks of a substance preparation protocol
 * - collect all QC-problems
 * - global setting $this->qcProblem [RETURN]
 * @todo check if EXEC-DATE was set on protocol !
 * @param $sql
 * @param $protoID
 * @param $algo : ["full"], "weak", 'full2';
 * @param array $chopt
 * 	'CERT_FLAG' = [1] ,2,3,
 * @return array('all'=>$statusAll, 'text'=>qcProblem) 
 * 	'all'=> 1 o.k.; <= 0 : error
 *  'text' error text
 * 
 */
function protoCheckBase( &$sql, $protoID, $algo = NULL, $chopt=NULL ) {
	
	$this->qcProblem = NULL;
	if ($algo == "") $algo = "full";
	
	$infolevel=1;
	if ($algo == "full2") $infolevel=2;
	$qcLib = new oConcProto_qc($infolevel); // do not dewfine as CLASS-var (iteration problem)
	$qcLib->setProtoID($sql, $protoID, 1, $chopt);
	$tmpkomma="";
	
	do {
		if ($qcLib->execDateUnx==NULL) {
			$this->qcProblem = "EXEC_DATE of protocol missing.";
			break;
		} 
		
		$expiryArr = $qcLib->checkSubstExpiry($sql);
		
		if ($expiryArr!=NULL) {
			$numsubst = sizeof($expiryArr);
			$temp_expText  = $numsubst > 1 ? "substances are expired" : "substance is expired";
			$this->qcProblem .= "<b>$numsubst</b> ".$temp_expText.": \n";
			
			foreach( $expiryArr as $substid=>$days) {
				$this->qcProblem .= $tmpkomma . "<b>".
						obj_nice_name( $sql, "CONCRETE_SUBST", $substid )."</b>".
						" (".abs($days)." days)";
				$tmpkomma = "; ";
			} 
			reset($expiryArr);
			
		} 
		
		if ( $algo == "full" or $algo == "full2" ) {
			
			// @swreq SREQ:0001723:SUBREQ:002: check mandatory values 
			$mandInfo = $qcLib->getMandProblem($sql);
			if ($mandInfo['err']>0) {
				$this->qcProblem .= $tmpkomma . 'MandErr: '. $mandInfo['info'];
				$tmpkomma='; ';
			}
			
			// @swreq SREQ:0001723:003: check limits (defined in ACCEPT_PROT)
			$limitInfo = $qcLib->getLimitProblem($sql);
			if ($limitInfo['err']>0) {
				$this->qcProblem .= $tmpkomma . 'LimitErr: '. $limitInfo['info'];
				$tmpkomma='; ';
			}
			
			$ansarr = $qcLib->checkConSubstMiss( $sql );
			if ($ansarr[0]>0) {
				$this->qcProblem .= $tmpkomma . $ansarr[1]." substances missing in protocol";
				$tmpkomma = "; ";
			} 
			
			$releasedMis = $qcLib->checkReleased( $sql );
			if ($releasedMis>0) {
				$this->qcProblem .= $tmpkomma . $releasedMis. " substances not 'released' in protocol";
				$tmpkomma = "; ";
				if ($algo == "full2" ) {
					$infoarr = $qcLib->getInfoArray();
					if (sizeof($infoarr)) $this->qcProblem .= '<br>'.implode(', ',$infoarr);
				}
				
			}
			
		}
	} while (0);
	
	$statusAll=1;
	if ($this->qcProblem!=NULL) $statusAll = -1;
	
	return array('all'=>$statusAll, 'text'=>$this->qcProblem);
}

function warningsOut() {
	if ( $this->qcProblem!="" ) {
		echo "<ul>";
		htmlInfoBox( "QC Warning", "", "open", "WARN" );
		echo $this->qcProblem;
		htmlInfoBox( "", "", "close" );
		echo "</ul>\n";
	}
}

/**
 * qc-protocol check
 */
function qcprotoCheck( &$sqlo, $substid, $aSubstId ) {
	
	$this->qcProblem = NULL;
	$cSubstQCLib = new ocSUC_qcProtoMeta($sqlo);
	$answer = $cSubstQCLib->checkQCProto( $sqlo, $substid, $aSubstId );
	if ($answer[0]<0) {
		$this->qcProblem = 'error in QC-Proto: '.$answer[1];
	}
	
	$this->warningsOut();
}

}

