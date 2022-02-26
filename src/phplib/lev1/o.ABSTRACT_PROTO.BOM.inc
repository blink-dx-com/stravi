<?php
/**
 * ABSTRACT_PROTO : manage BOM (bill of material)
 * @package o.ABSTRACT_PROTO.BOM.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq   SREQ:0001040: g > sub functions for BOM (Bill of material) 
 */
class oABSTRACT_PROTO_BOM {
	
	/**
	 * get materials of BOM
	 * @param $sqlo
	 * @param $abstr_proto_id
	 * @return array of material-IDs (including alternative child_ids)
	 *     [ABSTRACT_SUBST_ID] = array(child_ids) - child-ids are optional
	 *     					   = 0 or NULL : no children
	 */
	function getSubstances(&$sqlo, $abstr_proto_id) {
		$sqlsel = "x.ABSTRACT_SUBST_ID, x.IS_META ".
			" from ABSTRACT_PROTO_STEP s join ABSTRACT_SUBST x on x.ABSTRACT_SUBST_ID=s.ABSTRACT_SUBST_ID ".
			" where s.ABSTRACT_PROTO_ID=".$abstr_proto_id. 
			" ORDER by s.MAIN_STEP_NR";
		$sqlo->Quesel($sqlsel);
		$pannedSubst=NULL;
		$metaHas=0;
		while ( $sqlo->ReadRow() ) {
		    $ABSTRACT_SUBST_ID = $sqlo->RowData[0];
		    $IS_META = $sqlo->RowData[1];
		    $metaarr = $IS_META;
		    $pannedSubst[$ABSTRACT_SUBST_ID]=$metaarr;
		    if ($IS_META>0) $metaHas=1;
		}
		
		if ($metaHas) {
			// substance has alternative substances
			foreach( $pannedSubst as $ABSTRACT_SUBST_ID=>$IS_META) {
				if ($IS_META) {
					// get sub-substances
					$childarr=NULL;
					$sqlsel = "CHILD_SUBST_ID from ABS_CHILD_SUBST ".
						" where ABSTRACT_SUBST_ID=".$ABSTRACT_SUBST_ID. 
						" ORDER by POS";
					$sqlo->Quesel($sqlsel);
					while ( $sqlo->ReadRow() ) {
					    $child_id = $sqlo->RowData[0];
					    $childarr[] = $child_id;
					}
					$pannedSubst[$ABSTRACT_SUBST_ID] = $childarr;
				}
			}
			reset ($pannedSubst); 
		}
		
		
		return $pannedSubst;
	}
}
