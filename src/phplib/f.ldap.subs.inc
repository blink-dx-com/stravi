<?php

/**
 * LDAP connector, needed for anonymous AD queries
 * @package f.ldap.subs.inc
 * @swreq UREQ:0001135: g > LDAP queries 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * GLOBAL VARS: 
 *  - table:GLOBALS: 'NAME='DbLogin.LDAP'
 *  - table:GLOBALS: 'NAME='LDAP.queries', val=NULL or 1 => queries are configured?
 *  @version $Header: trunk/src/phplib/f.ldap.subs.inc 59 2018-11-21 09:04:09Z $
 */

class f_LDAP_subs {
	
	/**
	 * 
	 * @var $connectDict
	 * 	ldapserver
	 *  base_dn
	 *  userdomain
	 *  user - LDAP user
	 *  pw   - clear password
	 */
	var $connectDict;
	
	function __construct() {
		$this->adSession = 0;
		$this->_debug  = 0;
	}
	
	/**
	 * get info, if LDAP queries are configured
	 * @param  $sqlo
	 * @return $flag: 0-no, 1:yes
	 */
	function queries_configured(&$sqlo) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
		
		$globLdapVar = glob_elementDataGet( $sqlo, 'GLOBALS', 'NAME', 'LDAP.queries', 'VALUE');
		$globLdapVar = trim($globLdapVar);
		if ($globLdapVar>0) {
			return 1;
		}
		return 0;
	}
	
	/**
	 * get connect Dict
	 * - do some basic checks for data
	 * @param  $sqlo
	 * @return $connectDict
	 */
	function getConnectDict(&$sqlo) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		// 'pw1' : base64 encoded
		// @var $globLdapVar  "server:dc1.clondiag.jena;base_dn:dc=clondiag,dc=jena;userdomain:clondiag.jena;user:...;pw1:....."
		
		$globLdapVar = glob_elementDataGet( $sqlo, 'GLOBALS', 'NAME', 'DbLogin.LDAP', 'VALUE');
		$globLdapVar = trim($globLdapVar);
		if ($globLdapVar=="") {
			return NULL;
		}
		
		$connectDict=array();
		
		$ldapArr = explode(";",$globLdapVar );
		foreach( $ldapArr as $dummy=>$keyval_string) {
			
			if ($keyval_string==NULL) continue; // ignore
			
			$minivals =  explode(":", $keyval_string );
			
			if ( sizeof($minivals)<2 ) {
				$error->set( $FUNCNAME, 5, "LDAP: globals['DbLogin.LDAP'] invalid. ldapArr:$keyval_string" );
				return;
			}
			if ($minivals[0]=="server") {
				$connectDict['ldapserver'] = $minivals[1];
			} 
			if ($minivals[0]=="base_dn") {
				$connectDict['base_dn'] = $minivals[1];
			}
			if ($minivals[0]=="userdomain") {
				$connectDict['userdomain'] = $minivals[1];
			}
			if ($minivals[0]=="user") {
				$connectDict['user'] = $minivals[1];
			}
			if ($minivals[0]=="pw1") {
				$pw1 = $minivals[1];
				$connectDict['pw']  = base64_decode($pw1);
			}
		}
		reset ($ldapArr);
		
		$needvalues = array('ldapserver', 'userdomain', "user", "pw");
		$no_vals=array();
		foreach($needvalues as $key) {
			if ($connectDict[$key]==NULL) $no_vals[]=$key;
		}
		
		if (sizeof($no_vals)) {
			$error->set( $FUNCNAME, 6, "LDAP: globals['DbLogin.LDAP'] invalid. Missing params:". implode(', ',$no_vals));
			return;
		}  
		
		$this->connectDict = $connectDict;
		return $connectDict;
	}
	
	/**
	 * connect to LDAP
	 * - test: 
	 * @param $connectDict [OPTIONAL], otherwise, get internal connect dict
	 * @return -
	 */
	function connect( $connectDict=NULL ) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$this->adSession = NULL;
		
		if ($connectDict==NULL) {
			$connectDict=$this->connectDict;
		}
		
		if (!is_array($connectDict)) {
			$error->set( $FUNCNAME, 1, 'no connection params given.');
			return;
		}
		
		$ldapserver = $connectDict['ldapserver'];
		$this->adSession = ldap_connect( $ldapserver );
		if ( !$this->adSession ) {
			$rettxt = "ldap_connect failed to server: ".$ldapserver;
			$error->set( $FUNCNAME, 2, $rettxt);
			return;
		}
		
		//Set some variables
		ldap_set_option($this->adSession, LDAP_OPT_PROTOCOL_VERSION, 3);
		ldap_set_option($this->adSession, LDAP_OPT_REFERRALS, 0);
		
		//Bind to the ldap directory
		$dn     = $connectDict['user'].'@'.$connectDict['userdomain'];  // 'cn='.$connectDict['user'].','.$connectDict['cnstring'];
		$answer = ldap_bind($this->adSession,$dn,$connectDict['pw']);
		if ($answer == False) {
			$rettxt = "ldap_bind failed";
			$error->set( $FUNCNAME, 3, $rettxt);
			return;
		} 
		
		
	}
	
	/**
	 * search entries
	 * 
	 * @param $filter example: (&(|(objectClass=user)(objectClass=contact))(objectCategory=person)(cn=*))
	 * @return $resultStr
	 */
	function getEntries($filter, $justthese=NULL) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$adSession = &$this->adSession;
		
		$dn = $this->connectDict['base_dn']; // 'dc=clondiag,dc=jena';
		$sr = ldap_search($adSession, $dn, $filter, $justthese);
		if ($sr===FALSE) {
			$ldap_err = ldap_error ($adSession);
			$error->set( $FUNCNAME, 1, 'ldap_search failed: '.$ldap_err);
			return;
		}
		if ($this->_debug) {
			echo "result_xxx:<br>";
			var_dump($sr);
			echo "<br>";
		}
		$result     = ldap_get_entries($adSession, $sr);
		return $result;
	}
	
	/**
	 * get result like this
	 * 
	 * @param array $result the LDAP_result
	 * @param array $justthese e.g. array("samaccountname","telephoneNumber")
	 * @return array $niceArr
	 *  - index of subarray == index of array $justthese
	 *  [0]=>array([0]=>val1, [1]=>val2)
	 *  [1]=>array([0]=>val1, [1]=>val2)
	 */
	function getResultAsDict( &$LDAP_result, $ldapKeys ) {
		unset($LDAP_result['count']);
		
		$niceResult=NULL;
		reset ($LDAP_result);
		foreach( $LDAP_result as $searchID=>$valarr) {
			
			if (!is_numeric($searchID)) continue; // LDAP result can contain NON-numbers
			
			$niceSubarr = NULL;
			
			reset ($ldapKeys);
			foreach( $ldapKeys as $dummy=>$key) {
				$loopArr      = $valarr[$key];
				$niceSubarr[] = $loopArr[0];
			}
			reset ($ldapKeys); 
			
			$niceResult[$searchID] = $niceSubarr;
		}
		reset ($LDAP_result);
		
		return $niceResult;
	}
	
}
