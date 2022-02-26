<?php

// $Header: trunk/src/phplib/o.ABSTRACT_PROTO.policy.inc 59 2018-11-21 09:04:09Z $
/**
 * special release policy for ABSTRACT_PROTO
 * called by e.g. www/lab/pol.objRelease.inc
 * @package o.ABSTRACT_PROTO.policy.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq   SREQ:0003356: f.object.release.policy > special for o.ABSTRACT_PROTO 
 */
class oPRA_policy {
	
	function __construct( &$sqlo ) {
		if ( $_SESSION["userGlob"]["g.debugLevel"]>0 ) {
		    $this->_debug = $_SESSION["userGlob"]["g.debugLevel"];
		}
		$this->acclog = new oAccLogC();
	}
	
	/**
	 * get reopen info:
	 * deny reopen, when protocol is used by ABSTRACT_SUBST and ABSTRACT_SUBST is 'released', 'D_released', 'R_released'
	 * @param $sqlo
	 * @param $objid
	 * @param array $polOption
 	 *   'mo.CERT_FLAG' : [1],2,3,4 planned Life Cycle Flag of mother object
	 * @return array ( 'txt'=>$qc_infoText, 'err'=>$qc_problem )
	 * 	'txt' => string
	 *  'err' <= 0 o.k.
	 *  	  >  0 error
	 */
	function pol_reopen(&$sqlo, $objid, $infolevel=0, $polOption=NULL) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$pol_problem = 0;
		$outtext     = NULL;

		$a_proto_id = $objid;
		$table='ABSTRACT_PROTO';
		
		// deny these states
		$checkStates = array('released', 'D_released', 'R_released');
		$stateArr = NULL; // transform $checkStates
		
		foreach( $checkStates as $statname) {
		    $statsid = oH_ALOG_ACT_subs::getH_ALOG_ACT_ID( $sqlo, $table, $statname );
			$stateArr[] = $statsid;
		}
		$statsSQL = 'a.H_ALOG_ACT_ID in ('.implode(', ',$stateArr).')';

		$sqlsel = 'count(1) from ABSTRACT_SUBST x join CCT_ACCESS a on x.CCT_ACCESS_ID=a.CCT_ACCESS_ID'.
			' where x.ABSTRACT_SUBST_ID in (select ABSTRACT_SUBST_ID from AS_HAS_PR where ABSTRACT_PROTO_ID='.$a_proto_id.')'.
			' and '.$statsSQL;
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$cnt = $sqlo->RowData[0];
		
		
		if (!$cnt) return array ( 'txt'=>$outtext, 'err'=>$pol_problem );
		
		$pol_problem=1;
		$outtext='Ich kann kein "reopen" durchf&uuml;hren, weil dieses Protokoll in einer freigegebenen SUA genutzt wird! 
			Option: &ouml;ffne erst diese Substanzen! L&ouml;sung: erst SUA auf "reopen" setzen, dann dieses Protokoll &ouml;ffnen.';
		
		return array ( 'txt'=>$outtext, 'err'=>$pol_problem );
	}
}