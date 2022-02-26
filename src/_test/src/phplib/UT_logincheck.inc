<?php
require_once ("insertx.inc");
require_once ("f.assocUpdate.inc");
require_once ("glob.obj.update.inc");
require_once ("f.password.inc");

class UT_logincheck_inc extends gUnitTestSub {
	
function __construct() {

}

/**
 * create a new test account, if needed
 * @param object $sqlo
 * @param array $access_arr
 * @param $access 'intern', 'LDAP'
 */
function _prepare(&$sqlo, $access_arr, $access='intern') {
	// prepare
	$tablename = 'DB_USER';
	$crealib   = new insertC();
	
	$user_id = glob_elementDataGet( $sqlo, 'DB_USER', 'NICK', $access_arr['nick'], 'DB_USER_ID');
	
	$pw_db = fPasswordC::hash_pw($access_arr['pw']);
	if ( !$user_id ) {
		// create account
   
		$args = array('vals'=>array(
				'NICK'      => $access_arr['nick'],
				'EMAIL'     => 'test@gmx.de',
				'FULL_NAME' => 'test',
				'LOGIN_DENY'=> 0,
		        'PASS_WORD' => $pw_db
		));
	
		$user_id = $crealib->new_meta($sqlo,$tablename,$args);
	} else {
	    
	    // update password
	    $UpdateLib = new globObjUpdate();
	    $args = array('vals'=>array(
	        'LOGIN_DENY'=> 0,
	        'PASS_WORD' => $pw_db
	    ));
	    $UpdateLib->update_meta( $sqlo, $tablename, $user_id, $args );
	    
	}

	
	if ($access=='LDAP') {
		return; // no need for special role settings
	}
	
	$db_pw_is = glob_elementDataGet( $sqlo, $tablename, 'DB_USER_ID', $user_id, 'PASS_WORD');
	$this->_infoNow( 'PREP user password', print_r($args,1) );
	$this->_infoNow( 'crypted in DB', $db_pw_is );
	
	// role
	$role_name = 'UnitTest_login_norm';
	$db_role_id = glob_elementDataGet( $sqlo, 'ROLE', 'NAME', $role_name, 'ROLE_ID');
	if (!$db_role_id) {
	    throw new Exception('UnitTest Object "ROLE:'.$role_name.'" missing.');
	}
	
	$has_role = glob_elementDataGet2( $sqlo, 'USER_ROLES', array('ROLE_ID'=>$db_role_id, 'DB_USER_ID'=>$user_id), 'ROLE_ID');
	if (!$has_role) {
	    $assoclib = new  fAssocUpdate();
	    $assoclib->setObj( $sqlo, 'USER_ROLES', $db_role_id );
	    $argu = array( 'DB_USER_ID'=>$user_id );
	    $assoclib->insert( $sqlo, $argu );
	}
	
	
}


/**
 * 
 * @param object $sqlo
 * @param string $nick
 * @param string $pw
 * @param string $testname
 * @param int $expectlogin
 *    1: login ok
 *    -1: login fail
 */
function _onetest($sqlo, $nick, $pw, $testname, $expectlogin, $notes=NULL) {
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	$testLib = new fLoginC();
	
	$answer  = $testLib->loginCheck ( $sqlo, $pw, $nick, 0);
	$infolog = $testLib->getLoginLog();

	$this->_infoNow( $testname. ' Input:', $nick.' pw:'.$pw );
	$this->_infoNow( $testname. ' Answer:', print_r($answer,1) );
	$this->_infoNow( 'answer infolog:'.$testname, print_r($infolog,1) );
	if ($error->Got(READONLY))  {
		$errLast   = $error->getAllAsText();
		$this->_infoNow( 'answer ERROR:'.$testname, $errLast);
	}
	
	if ($expectlogin>0) {
		if ($answer['logok']<1) {
			$this->_saveTestResult($testname, 0, 'user: '.$nick.'; '.$notes);
			$error->reset();
			return;
		}
		$this->_saveTestResult($testname, 1);
	} else {
		if ($answer['logok']>0) {
			$result = 0;
		} else {
			$result = 1;		
		}
		$this->_saveTestResult($testname, $result);
		$error->reset();
	}
}

// return: 0 : not passed, 1: passed
function dotest( &$sqlo, $options ) {
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	$tablename = 'DB_USER';
	$access_arr = array(
		'nick' => 'user_logincheck',
		'pw'   => 'qu78xyQ9'
	);
	
	// the user of $access_arr2 must be registered in the ATJ-ActiveDirectory !
	// get user-access-info from config
	$conf_info = $this->_get_test_configval('logincheck.inc');
	if ($conf_info['user']==NULL) {
		$error->set( $FUNCNAME, 1, 'bad unittest configuration: user missing.' );
		return;
	}
	
	$access_arr2 = array(
		'nick' => $conf_info['user'],   
		'pw'   => base64_decode($conf_info['pw_b64'])
	);
	
	
	$this->_prepare($sqlo, $access_arr,  'intern');
	$this->_prepare($sqlo, $access_arr2, 'LDAP');


	
	$this->_onetest($sqlo, $access_arr['nick'],  $access_arr['pw'], 'TEST1', 1);
	$this->_onetest($sqlo, $access_arr['nick'],  NULL, 'TEST2', -1);
	$this->_onetest($sqlo, $access_arr['nick'],  'ksklsda', 'TEST2.1', -1);
	
	$this->_onetest($sqlo, $access_arr2['nick'],  NULL, 'TEST3', -1);
	// $this->_onetest($sqlo, $access_arr2['nick'],  $access_arr2['pw'], 'TEST4', 1, 'This user must have a valid login in the ActiveDirectory!');
	$this->_onetest($sqlo, $access_arr2['nick'],  'dreck', 'TEST5', -1);

	$retval = 1;
	
	
	return ($retval);
}

}
