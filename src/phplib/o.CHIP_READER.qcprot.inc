<?php
/**
 * QC protocol things
 * @package o.CHIP_READER.qcprot.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
 
class oA_CHIP_READER_QCprot {

function __construct( &$sqlo ) {
	
	$this->protoKindName = "QC test";
	$sqls = "select H_PROTO_KIND_ID from H_PROTO_KIND where NAME='".$this->protoKindName."'";
	$sqlo->query($sqls);
	$sqlo->ReadRow();
	$this->protoKindID = $sqlo->RowData[0];
	$this->StepLastName='STATUS';
	 
}

// get QC-protocol of a abs_device
function getQcProtoId(&$sqlo, $absDeviceID ) {
			
	$sqlsel = "QC_ABSPROTO_ID from A_CHIP_READER ".
			  " where A_CHIP_READER_ID= ".$absDeviceID;
	$sqlo->Quesel($sqlsel);
	$sqlo->ReadRow();
	$protoID = $sqlo->RowData[0];
	return $protoID;
}

/**
 * check, if protocol is a valid QC-protocol
 * @param $sqlo
 * @param $aProtoID
 * @return array($valid, $comment)
 */
function isValid(&$sqlo, $aProtoID) {
	$isvalid = 0;
	$comment = NULL;
	$sqlsel = "H_PROTO_KIND_ID from ABSTRACT_PROTO".
			"    where ABSTRACT_PROTO_ID= ".$aProtoID;	
	$sqlo->Quesel($sqlsel);
	$sqlo->ReadRow();
	$hProtoID = $sqlo->RowData[0];
	if ($hProtoID!=$this->protoKindID) {
		return array(-1, 'has not the "protocol kind" "'.$this->protoKindName.'"');
	}
	
	$sqlsel = "STEP_NR, NAME from ABSTRACT_PROTO_STEP".
			"    where ABSTRACT_PROTO_ID=".$aProtoID." order by MAIN_STEP_NR desc";
	$sqlo->Quesel($sqlsel);
	$sqlo->ReadRow();
	$name = $sqlo->RowData[1];
	if ( $name!= $this->StepLastName ) {
		return array(-3, 'last step-name must be "'.$this->StepLastName.'"');
	}
	return array(1, '');
}

/**
 * - get STEP_NR of STATUS-step
 * @param $sqlo
 * @param $aProtoID
 * @return STEP_NR
 */
function  getStatusStepNr(&$sqlo, $aProtoID) {
	global $error;
	$FUNCNAME= 'getValidStepNr';
	$sqlsel = "STEP_NR, NAME from ABSTRACT_PROTO_STEP".
			  " where ABSTRACT_PROTO_ID=".$aProtoID." order by MAIN_STEP_NR desc";
	$sqlo->Quesel($sqlsel);
	$sqlo->ReadRow();
	$step_nr = $sqlo->RowData[0];
	$name    = $sqlo->RowData[1];
	if ( $name!= $this->StepLastName ) {
		$error->set( $FUNCNAME, 1, 'invalid protocol: last step-name.' );
		return;
	}
	return($step_nr);
}


}

 