<?php

/**
 * analysis functions
 * @package o.ABSTRACT_PROTO.subs.inc
 * @swreq SREQ:0001801
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class oABSTRACT_PROTO_subs {
	/**
	 * get number of used PRCs
	 * @param  $sqlo
	 * @param  int $a_proto_id ABSTRACT_PROTO_ID
	 * @param  int $suaid ignore this ABSTRACT_SUBST_ID
	 */
	public function isUsed(&$sqlo, $a_proto_id, $suaid=0) {
		
		$sqlsel = 'count(1) from CONCRETE_PROTO where ABSTRACT_PROTO_ID='.$a_proto_id;
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$prcNum = $sqlo->RowData[0];

		$sqlsel = 'count(1) from AS_HAS_PR where ABSTRACT_SUBST_ID!='.$suaid.
			 ' and ABSTRACT_PROTO_ID='.$a_proto_id;
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$praNum = $sqlo->RowData[0];
		
		return array('PRC'=>$prcNum, 'SUA'=>$praNum);
	}
	
	static function get_PROTO_KIND_key(&$sqlo, $a_proto_id)  {
	    $H_PROTO_KIND_ID = glob_elementDataGet( $sqlo, 'ABSTRACT_PROTO', 'ABSTRACT_PROTO_ID', $a_proto_id, 'H_PROTO_KIND_ID');
	    if (!$H_PROTO_KIND_ID) return '';
	    $KIND_key = glob_elementDataGet( $sqlo, 'H_PROTO_KIND', 'H_PROTO_KIND_ID', $H_PROTO_KIND_ID, 'KEYX');
	    return $KIND_key;
	}
	
	// @return array of H_PROTO_KIND features
	static function get_PROTO_KIND_features(&$sqlo, $a_proto_id)  {
	    $H_PROTO_KIND_ID = glob_elementDataGet( $sqlo, 'ABSTRACT_PROTO', 'ABSTRACT_PROTO_ID', $a_proto_id, 'H_PROTO_KIND_ID');
	    if (!$H_PROTO_KIND_ID) return '';
	    $KIND_info = glob_elemDataGet3( $sqlo, 'H_PROTO_KIND', array('H_PROTO_KIND_ID'=> $H_PROTO_KIND_ID), array('KEYX', 'NAME') );
	    return $KIND_info;
	}
}
