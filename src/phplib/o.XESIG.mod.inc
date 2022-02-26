<?php

require_once 'o.XESIG.subs.inc';
require_once 'insertx.inc';
/**
 * modify, create electronic signature
 * @package o.XESIG.mod.inc
 * @swreq SREQ:0002090: o.XESIG > manage electronic signatures (eSign) 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class oXESIG_mod {
	
	function __construct() {
		
	}
	
	/**
	 * create a signature
	 * @param $sqlo
	 * @param array $params allowed:
	 * 	'X_ACCESS_ID'   - [optional] ID of CCT_ACCESS of target object
	 *  'H_ALOG_ACT_ID'	- [optional] ID of status
	 *  'CREAKEY'	    - 'required' key of tool; 
	 *  		- for a report-geneartor: e.g. MOD:QC2-Report
	 *    		- for additional review:  e.g. AUD:review
	 *  'NOTES'
	 * @return ID of signature
	 */
	function create(&$sqlo, $params) {
		$tablename='XESIG';
		
		$allowKey=array(
			'X_ACCESS_ID',
		   	'H_ALOG_ACT_ID',	
		   	'CREAKEY',	
		   	'NOTES'
			);
			
		$newparam=NULL;
		foreach ($allowKey as $keyx) {
			if ($params[$keyx]!==NULL) 
				$newparam[$keyx]=$params[$keyx];
		}
		
		$newparam['DB_USER_ID'] = $_SESSION['sec']['db_user_id'];
		$newparam['XDATE']      = date_unix2datestr(time(), 1);
		
		$insertLib = new insertRawC();
		$newid = $insertLib->insertR($sqlo, $tablename, $newparam, 'XESIG_ID');
		
		return $newid;
	}
	
	/**
	 * create signature and get parameters back
	 * @param $sqlo
	 * @param $params
	 */
	function createAndGet(&$sqlo, $params) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$newid = $this->create($sqlo, $params);
		
		if (!$newid or $error->Got(READONLY)) {
			$error->set( $FUNCNAME, 1, 'create failed.' );
			return;
		}
		
		$eSigFeats = oXESIG_subs::getSignAttribs($sqlo, $newid); 
		
		return $eSigFeats;
	}
}
