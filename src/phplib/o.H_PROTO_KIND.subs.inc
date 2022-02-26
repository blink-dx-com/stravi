<?php
/**
 * - sub functions for H_PROTO_KIND
 * @package o.H_PROTO_KIND.subs.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class oH_PROTO_KIND_subs {
    
    // defined keys
    const TYPE_QC='QC test';

	static function name2ID(&$sqlo, $name) {
	    $sqlsel = "H_PROTO_KIND_ID from H_PROTO_KIND where NAME=".$sqlo->addQuotes($name);
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		return ($sqlo->RowData[0]);
	}
	
	static function key2ID(&$sqlo, $key) {
	    $sqlsel = "H_PROTO_KIND_ID from H_PROTO_KIND where KEYX=".$sqlo->addQuotes($key);
	    $sqlo->Quesel($sqlsel);
	    $sqlo->ReadRow();
	    return ($sqlo->RowData[0]);
	}
	
	static function get_key(&$sqlo, $id) {
	    if (!$id) return NULL;
	    $sqlo->Quesel("KEYX from H_PROTO_KIND where H_PROTO_KIND_ID=".$id);
	    $sqlo->ReadRow();
	    return ($sqlo->RowData[0]);
	}
	
}
