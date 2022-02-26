<?php
// 

class UT_f_ldap_subs_inc extends gUnitTestSub {
	
function __construct() {

}


function dotest( &$sqlo, $options ) {
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	$this->_saveTestResult('TEST_TBD', 1, 'Define the tests !!!');
	
	return 1;
	
	$ldap_login = $this->_get_test_configval('logincheck.inc');
	$expect = array('ldapserver', 'userdomain', "user", "pw_b64");
	foreach($expect as $key) {
		if ($ldap_login[$key]==NULL ) {
			$error->set( $FUNCNAME, 4, 'UnitTest not configured for KEY "'.$key.'".' );
			return;
		}
	}
	
	$ldap_login['pw'] = base64_decode($ldap_login['pw_b64']);
	
	
	$testLib = new f_LDAP_subs();
	//USE the MOCK object ...$connectDict = $testLib->getConnectDict($sqlo);
	$connectDict = $ldap_login;
	
	if ($error->Got(READONLY))  {
		$error->set( $FUNCNAME, 1, 'no valid connectDict' );
		return;
	}
	$this->_infoNow( 'connectDict', print_r($connectDict,1) ); // just info for the test
	
	if ($connectDict==NULL) {
		$error->set( $FUNCNAME, 2, 'no LDAP-config given.' );
		return;
	}
	
	$testLib->connect($connectDict);
	if ($error->Got(READONLY))  {
		$error->set( $FUNCNAME, 3, 'connect failed' );
		return;
	}
	
	$userName  = 'skube';
	$filter    = '(&(objectClass=user)(objectCategory=person)(samaccountname='.$userName.'))'; // cn
	// $filter    = 'samaccountname='.$userName;
	$this->_infoNow( 'filter:', $filter );
	
	$justthese = array("telephonenumber", "mail", "mobile");
	$results   = $testLib->getEntries($filter, $justthese);
	if ($error->Got(READONLY))  {
		$error->set( $FUNCNAME, 3, 'connect failed' );
		return;
	}
	
	$this->_infoNow( 'data_of_user:', print_r($results,1) ); 
	
	$ldapKeys   = $justthese;
	$niceresult = $testLib->getResultAsDict( $results, $ldapKeys );
	$this->_info( 'NICE answer', print_r($niceresult,1) );
	
	if ($results!=NULL) $retval = 1;
	else  $retval = 0;
	
	return ($retval);
}

}
