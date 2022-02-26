<?php
require_once ("date_funcs.inc"); 

/**
 * - auto check for expiry date
 * 
 * @package o.CONCRETE_SUBST.autofini.inc
 * @author  Marco Albrecht (marco_a@clondiag.com)
 * @swreq UREQ:0000825
 * @unittest: exists
 * 
 *   
 */
class  o_CONCRETE_SUBST_autofini{
	
	/**
 	* function for change audit-status from released to finished when expiry date is passed
 	* @return 
 	*/
	function autoFinish(&$sql, &$sql2){
		$FUNCNAME="autoFinish";
		global $error;
		require_once ("o.CONCRETE_SUBST.qcInv.inc");
		require_once ('o.CCT_ACCLOG.subs.inc');
	
		$error = & ErrorHandler::get();
	
		$accLogLib = new oAccLogC();
		$keys = array("released", "D_released", "finished");
		
		$table='CONCRETE_SUBST';
		$accLogState=array();
		
		
		foreach($keys as $key) {
		    $accLogState[$key]   = oH_ALOG_ACT_subs::getH_ALOG_ACT_ID( $sql, $table, $key );
		    if (!$accLogState[$key]) {
		        $error->set( $FUNCNAME, 1, 'H_ALOG_ACT ("'.$key.'") not found.' );
		        return;
		    }
		}
		
		$dateUnx = time();
		$datehuman = date_unix2datestr($dateUnx, 1);
		
		$sqlsubst 	 = "s.CONCRETE_SUBST_ID from CONCRETE_SUBST s, 
					CCT_ACCESS c where s.CCT_ACCESS_ID = c.CCT_ACCESS_ID AND
					s.EXPIRY_DATE < '" . $datehuman . "' and 
					c.H_ALOG_ACT_ID in (".$accLogState["released"].", ".$accLogState["D_released"].")";
		$sql->Quesel($sqlsubst);
		$errInit=0;
		$errAdd=0;
		$succInit =0;
		while ( $sql->ReadRow() ) {
			$substid = $sql->RowData[0];
			$accLogLib->setObject($sql2, 'CONCRETE_SUBST', $substid);
			if ($error->Got(READONLY))  {
				$errInit++;
				$error->reset();
			} else {
				$argu = array("action"=>$accLogState["finished"], 'notes'=>'expired' );
				$accLogLib->addLogSub( $sql2, $argu);
				if ($error->Got(READONLY))  {
					$errAdd++;
					$error->reset();
				}
			}
			$succInit++;
		}
		if ($errInit) {
				$error->set( $FUNCNAME, 1, '.... Problem on substance-acclog-init; '
						.$errInit.' substances.' );
		}
		if ($errAdd) {
				$error->set( $FUNCNAME, 2, '.... Failed to change status for '
						.$errAdd.' substances.' );
		}
		$finarr=NULL;
		$finarr["finished"] = $succInit;
		return $finarr;
	}

}