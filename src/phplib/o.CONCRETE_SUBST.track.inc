<?php
/**
 * track  concrete substances
 * @package o.CONCRETE_SUBST.track.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq UREQ:0002413: o.CONCRETE_SUBST > kleine Hilfsfunktionen 
 */
class oCONCRETE_SUBST_track {
	
	/**
	 * track usage of substance, get parents
	 * - throw error, if more than 100 matches
	 * @param $sqlo
	 * @param $objid CONCRETE_SUBSTANCE-child-ID
	 * @return array of SUBSTANCE-IDs
	 */
	function track_usage(&$sqlo, $objid) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
		$MAX_CACHE = 100;
		$parentarr = NULL;
		
		$sqls_after = "CONCRETE_PROTO_ID in (select CONCRETE_PROTO_ID from ".
			      " CONCRETE_PROTO_STEP where CONCRETE_SUBST_ID=".$objid.")";
		$sqlsel = "CONCRETE_SUBST_ID from CONCRETE_SUBST where ".$sqls_after;
		$sqlo->Quesel($sqlsel);
		$cnt=0;
		while ( $sqlo->ReadRow() ) {
			if ($cnt>$MAX_CACHE) {
				$error->set( $FUNCNAME, 1, 'Too many parents found. Limit: '.$MAX_CACHE );
				return;
			}
		    $parentid = $sqlo->RowData[0];
		    $parentarr[]=$parentid;
		    
		    $cnt++;
		}
	  	return $parentarr;
	}
}
