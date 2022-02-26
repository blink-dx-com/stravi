<?php
 
/**
 * QC protocol things
 * @package o.ABSTRACT_SUBST.qcprot.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq   SREQ:0001723: g > qc-policy for QC-protocols 
 */
class oAbsSubstQCprot {
    
    const STATUS_STEP_NAME='STATUS';

    function __construct( &$sqlo ) {
    	
    	$this->protoKindName = "QC test";
    	$sqls = "select H_PROTO_KIND_ID from H_PROTO_KIND where NAME='".$this->protoKindName."'";
    	$sqlo->query($sqls);
    	$sqlo->ReadRow();
    	$this->protoKindID = $sqlo->RowData[0]; 
    }
    
   
    
    /**
     * check, if protocol is a valid QC-protocol
     * @param $sqlo
     * @param $aProtoID
     * @return array($valid, $comment)
     *    1 : ok
     *    <0: error
     */
    function isValid(&$sqlo, $aProtoID) {
    	//$isvalid = 0;
    	//$comment = NULL;
    	$sqlsel = "H_PROTO_KIND_ID from ABSTRACT_PROTO".
    			"    where ABSTRACT_PROTO_ID= ".$aProtoID;	
    	$sqlo->Quesel($sqlsel);
    	$sqlo->ReadRow();
    	$hProtoID = $sqlo->RowData[0];
    	if ($hProtoID!=$this->protoKindID) {
    		return array(-1, 'expect "protocol kind"="'.$this->protoKindName.'"');
    	}
    	return array(1, '');
    	
    // 	$sqlsel = "STEP_NR from ABSTRACT_PROTO_STEP".
    // 			"    where ABSTRACT_PROTO_ID=".$aProtoID." and ABSTRACT_SUBST_ID>0 AND IS_SAMPLE>0";
    // 	$sqlo->Quesel($sqlsel);
    // 	$sqlo->ReadRow();
    // 	$stepnr = $sqlo->RowData[0];
    // 	if (!$stepnr) {
    // 		return array(-2, 'steps contain no substance with SAMPLE-flag"');
    // 	}
    	
    	$sqlsel = "STEP_NR, NAME from ABSTRACT_PROTO_STEP".
    			"    where ABSTRACT_PROTO_ID=".$aProtoID." order by MAIN_STEP_NR desc";
    	$sqlo->Quesel($sqlsel);
    	$sqlo->ReadRow();
    	$name = $sqlo->RowData[1];
    	if ( $name!= self::STATUS_STEP_NAME ) {
    		return array(-3, 'last step-name must be "'.self::STATUS_STEP_NAME.'"');
    	}
    	return array(1, '');
    }
    
    /**
     * - get STEP_NR of STATUS-step
     * @param $sqlo
     * @param $aProtoID
     * @return int STEP_NR
     */
    static function getStatusStepNr(&$sqlo, $aProtoID) {
    	global $error;
    	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
    	
    	$sqlsel = "STEP_NR, NAME from ABSTRACT_PROTO_STEP".
    			  " where ABSTRACT_PROTO_ID=".$aProtoID." order by MAIN_STEP_NR desc";
    	$sqlo->Quesel($sqlsel);
    	$sqlo->ReadRow();
    	$step_nr = $sqlo->RowData[0];
    	$name    = $sqlo->RowData[1];
    	if ( $name!= self::STATUS_STEP_NAME ) {
    	    $error->set( $FUNCNAME, 1, 'PRA:'.$aProtoID.' invalid QC-protocol: last step-name  must be: '.self::STATUS_STEP_NAME );
    		return;
    	}
    	return($step_nr);
    }
    
    /**
     * - get STEP_NR, INFO of mandatory values
     * @param $sqlo
     * @param $aProtoID
     * @return $retarr; possible values:
     *  - NULL : nothing
     * 	- array(STEP_NR=>INFO)
     */
    function  getMandatoryInfo(&$sqlo, $aProtoID) {
    	//global $error;
    	
    	$sqlsel = "STEP_NR, MANDATORY_VALUES from ABSTRACT_PROTO_STEP".
    			  " where ABSTRACT_PROTO_ID=".$aProtoID." and MANDATORY_VALUES is not NULL order by STEP_NR";
    	$sqlo->Quesel($sqlsel);
    	$retarr=NULL;
    	
    	while ($sqlo->ReadRow()) {
    		$step_nr = $sqlo->RowData[0];
    		$keys    = $sqlo->RowData[1];
    		if ( $keys!=NULL ) {
    			$retarr[$step_nr]=$keys;
    		}
    	}
    	return($retarr);
    }
    
    /**
     * get step_nr of sample-step
     * @param $sqlo
     * @param $aProtoID
     * @return int 
     */
    function getStepsNrOfSample(&$sqlo, $aProtoID, $aSubstID) {
    	$sqlsel = "STEP_NR from ABSTRACT_PROTO_STEP".
    			"    where ABSTRACT_PROTO_ID= ".$aProtoID." and IS_SAMPLE>0";
    			
    	$sqlo->Quesel($sqlsel);
    	$sqlo->ReadRow();
    	$step_nr = $sqlo->RowData[0];
    	return ($step_nr);
    }

}

 