<?php

/**
 * analyse electronic signature
 * @package o.XESIG.subs.inc
 * @swreq SREQ:0002090: o.XESIG > manage electronic signatures (eSign) 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class oXESIG_subs {
	function __construct() {
		
	}
	
	/**
	 * return the features
	 * @param $sqlo
	 * @param $xesig_id
	 * @return array features + extra features
	 *   'ESIG_NAME' : full qualified eSign (DBID+XESIG)
	 *   'user.name' : full name of user
	 */
	function getSignAttribs(&$sqlo, $xesig_id) {
		
		if (!$xesig_id) return;
		
		$sqlsel = '* from XESIG where XESIG_ID='.$xesig_id;
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadArray();
		$feats = $sqlo->RowData;
		
		$dbid = $_SESSION['globals']['magasin_serial'];
		
		if (!$dbid) {
			sys_error_my( 'missing _SESSION[globals][magasin_serial]' );
		}
		
		$feats['ESIG_NAME'] = $dbid.':'.$xesig_id; // artificial attribute !
		$feats['user.name'] = glob_elementDataGet( $sqlo, 'DB_USER', 'DB_USER_ID', $feats['DB_USER_ID'], 'FULL_NAME'); 
		
		return $feats;
	}
}
