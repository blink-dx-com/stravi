<?php
require_once ('logincheck.inc');

/**
 * sub-methods for electronic signature
 * @package f.signature.sub.inc
 * @swreq UREQ:0000704:add audit status;
 *   SUBREQ: 11.10(h) : double signin on critical actions 
 *   SUBREQ: 11.200(a)(1)(ii) : double signin after 10 minutes of NON signin
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class fSignatureSub {

	public  $SIGN_MIN_PERIOD=5; // time to next signin in minutes
	private $defTables; /** @var $defTables
		array ( TABLE => array(
			'sig' => : need of PART11-signature here ?
			  [1] : yes
			   '' : DEFAULT
			  -1  : no
			)
		)
	*/
	
	
	function __construct() {
	    
	    // no need of signature for this table, if 'sig'=-1
		$this->defTables= array(
			'MORDER'         => array('sig'=> -1), 
		    'CONCRETE_PROTO' => array('sig'=> -1),
		    'PUR'            => array('sig'=> -1), 
		);
	}
	
	/**
	 * need a signature for this table ?
	 * @param $tablename
	 * @return 0: no, 1:yes
	 */
	function tableInfo($tablename) {
		$needpw = 1;
		$info   = $this->defTables[$tablename];
		if ($info['sig']<0) $needpw = 0;
		return $needpw;
	}
	
	/**
	 * need second signin ?
	 * @param string $tablename [optional] the tablename of the action
	 * @return int 0,1 need new login? no/yes
	 */
	function checkSignNeed($sqlo, $tablename=NULL) {
		$needLogin = 1;
		
		if ($tablename!=NULL) {
			// SUBREQ:11.10(h)X1 : exception: no need for signature for special tables
			$info = $this->defTables[$tablename];
			if ($info['sig']<0) return 0; // not needed for this table
		}
		
		$lastTime  = $_SESSION['s_sessVars']['signLastTime'];
		 // minutes
		$now  = time();
		$diff = $now - $lastTime;
		if ($diff <= $this->SIGN_MIN_PERIOD*60) $needLogin=0;
		 
		return $needLogin;
	}
	
	/**
	 * save successful signing
	 * - save time stamp
	 */
	function saveSigning() {
		$_SESSION['s_sessVars']['signLastTime'] = time();
	}
	
	/**
	 * check password
	 * @return $ok ?
	 * 	0: error; 1: o.k.
	 */
	function checkPW(&$sqlo, $pw) {
		$loginLib = new fLoginC();
		$okarr    = $loginLib->checkOnlyPW($sqlo, $pw, $_SESSION['sec']['appuser']);
		if ($okarr['ok']<=0) return 0;
		
		// save time stamp
		$this->saveSigning();
		return 1;
	}
}
