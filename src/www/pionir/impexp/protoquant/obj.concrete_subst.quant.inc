<?php
require_once ('o.CS_HAS_PR.subs.inc');
require_once ('o.AS_HAS_PR.subs.inc');

/**
 * helper functions for ProtoQuant
 * @swreq   UREQ:12963 Material Monitoring Concept 2018
 * @package obj.concrete_subst.quant.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/www/pionir/subs/obj.concrete_subst.quant.inc 59 2018-11-21 09:04:09Z $
 */
class oConcSubstProtQuant_H {
	
	function __construct($sqlAfter) {
		$this->sqlAfter = $sqlAfter;
	}
	
	
	/**
	 * get all planned ABSTRACT_PROTO_ID from SUC => SUA
	 * @param object $sqlo
	 * @param int $suc_id
	 * @param array $aprotoarr : RETRUN as reference
	 */
	private function _get_apid_from_sucsua($sqlo, $suc_id, &$aprotoarr) {
		
		$sua_id = glob_elementDataGet( $sqlo, 'CONCRETE_SUBST', 'CONCRETE_SUBST_ID', $suc_id, 'ABSTRACT_SUBST_ID');
		
		$sua_ProtoLib = new oAS_HAS_PR_subs();
		$sua_ProtoLib->setObj($sua_id);
		$pra_array = $sua_ProtoLib->getProtoLog($sqlo);
		if (!sizeof($pra_array)) return;
		
		foreach($pra_array as $pra_row) {
			$apid = $pra_row['ap'];

			if (!in_array($apid, $aprotoarr)) {
				$aprotoarr[]=$apid;
			}
		}
	}
	
	/**
	 * get possible abstract protocols
	 * return $this->abstractProtos = $aprotoarr;
	 * @param unknown $sqlo
	 * @param unknown $sqlo2
	 * @return array $aprotoarr - array of ABSTRACT_PROTO_ID
	 */
	function getAbsProtos( &$sqlo, &$sqlo2 ) {
		
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		if ($this->sqlAfter==NULL)  {
			$error->set( $FUNCNAME, 1, 'Need a selction of SUCs' );
			return;
		}
		
	
		$substProtoLib = new oCS_HAS_PR_subs();
	
		$aprotoarr = array();
	
		$sqlsel = "CONCRETE_SUBST_ID from ". $this->sqlAfter;
		$sqlo2->Quesel($sqlsel);
		$suc_cnt=0;
		$first_suc_id = 0; // save this to get the planned protocols from this SUC
	
		while ( $sqlo2->ReadRow() ) {
	
			$loop_substid = $sqlo2->RowData[0];
			if (!$suc_cnt) {
				$first_suc_id = $loop_substid;
			}
		  
			// get all abstract protocols
			$substProtoLib->setObj($loop_substid);
			$protoLog = $substProtoLib->getProtoLog($sqlo);
		  
			// get abstract protocol IDs
			if (is_array($protoLog)) {
				reset ($protoLog);
				$cpCollect = NULL;
				foreach( $protoLog as $key=>$val) {
					$cpid = $val['cp'];
					if ($cpid) $cpCollect[]=$cpid;
				}
	
				if (is_array($cpCollect)) {
					 
					// get abstract protocols from concrete protocols
					$sqlsel = "ABSTRACT_PROTO_ID from CONCRETE_PROTO where CONCRETE_PROTO_ID in (".implode(',',$cpCollect).")";
					$sqlo->Quesel($sqlsel);
					while ( $sqlo->ReadRow() ) {
						$apid = $sqlo->RowData[0];
	
						if (!in_array($apid, $aprotoarr)) {
							$aprotoarr[]=$apid;
						}
					}
				}
	
			}
			
			$suc_cnt++;
		  
		}
		
		if ($first_suc_id) {
			$this->_get_apid_from_sucsua($sqlo, $first_suc_id, $aprotoarr);
		}
	
		return $aprotoarr;
	}
}