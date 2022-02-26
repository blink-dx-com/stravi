<?php 

require_once 'o.CS_HAS_PR.subs.inc';
require_once 'o.EXP.proto.inc';

/**
 * - special POLICIES of objects (SREQ:0002125:002)
 * @package glob.obj.lockPolicy.inc
 * @swreq SREQ:0002125 g > access_lock (helper lib)
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 *
 */
class oCONCRETE_SUBST_lockPol {
	
	function __construct() {
		$this->protoLib = new oCS_HAS_PR_subs();
		
	}
	/**
	 * get children to lock
	 * @return array('t'=>table, 'id'=>ID)
	 */
	function getLockChildren(&$sqlo, $objid) {
		$this->protoLib->setObj($objid);
		
		// array('cp'=>ID of protocol, 'or'=>order number, 'ty'=>1,2)
		$protoarr = $this->protoLib->getProtoLog($sqlo);
		if (!is_array($protoarr)) return NULL;
		
		// prepare ...
		$retarr=array();
		reset ($protoarr);
		foreach( $protoarr as $dummy=>$valarr) {
			$retarr[]=array('t'=>'CONCRETE_PROTO', 'id'=>$valarr['cp']);
		}
		return $retarr;
	}
}

//
class oCHIP_READER_lockPol {
	/**
	 * get children to lock
	 * @return array('t'=>table, 'id'=>ID
	 */
	function getLockChildren(&$sqlo, $objid) {
		
		$retarr = NULL;
		// ('CONCRETE_PROTO_ID', 'QC_CPROTO_ID')
		$devFeats = glob_elemDataGet3( $sqlo, 'CHIP_READER', 
			array('CHIP_READER_ID'=>$objid), 
			array('CONCRETE_PROTO_ID', 'QC_CPROTO_ID') );
		if ($devFeats['CONCRETE_PROTO_ID']) {
			$retarr[]= array('t'=>'CONCRETE_PROTO', 'id'=>$devFeats['CONCRETE_PROTO_ID']);
		}
		if ($devFeats['QC_CPROTO_ID']) {
			$retarr[]= array('t'=>'CONCRETE_PROTO', 'id'=>$devFeats['QC_CPROTO_ID']);
		}
		return $retarr;
	}
	
	function getOpenChildren(&$sqlo, $objid) {
		$retarr = NULL;
		// ('CONCRETE_PROTO_ID', 'QC_CPROTO_ID')
		$devFeats = glob_elemDataGet3( $sqlo, 'CHIP_READER', 
			array('CHIP_READER_ID'=>$objid), 
			array('CONCRETE_PROTO_ID', 'QC_CPROTO_ID') );
		if ($devFeats['CONCRETE_PROTO_ID']) {
			$retarr[]= array('t'=>'CONCRETE_PROTO', 'id'=>$devFeats['CONCRETE_PROTO_ID']);
		}
		if ($devFeats['QC_CPROTO_ID']) {
			$retarr[]= array('t'=>'CONCRETE_PROTO', 'id'=>$devFeats['QC_CPROTO_ID']);
		}
		return $retarr;
	}
}

/**
 * log EXP protocols
 * @author steffen
 */
class oEXP_lockPol {
	
	function __construct() {
		$this->protoLib = new oEXPprotoC();
	}
	/**
	 * get children to lock
	 * @return array('t'=>table, 'id'=>ID)
	 */
	function getLockChildren(&$sqlo, $expid) {	
		
		$retarr=array();
		
		$protoarr = $this->protoLib->getCProtos ($sqlo, $expid);
		if (is_array($protoarr)) {
			reset ($protoarr);
			foreach( $protoarr as $dummy=>$cpid) {
				if ($cpid)
					$retarr[]=array('t'=>'CONCRETE_PROTO', 'id'=>$cpid );
			}
		}
		
		//get Image
// 		$sqlsel = "IMG_ID from IMG_OF_EXP where EXP_ID=".$expid;
// 		$sqlo->Quesel($sqlsel);
// 		$sqlo->ReadRow();
// 		$imgid = $sqlo->RowData[0];
// 		if ($imgid) $retarr[]=array('t'=>'IMG', 'id'=>$imgid );
		
		return $retarr;
	}
}
