<?php
require_once ('date_funcs.inc');
require_once 'f.password.inc';

/**
 * checks for login
 * - since 0.1 - 20020904
 * @package logincheck.inc
 * @swreq UREQ:0001688: g > user login to system 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/phplib/logincheck.inc 59 2018-11-21 09:04:09Z $
 */
class fLoginC {
	
function __construct() {
}

/**
 *  - get the global variables from databse (keys are defined in $globalsSave), which should be in the variable $globals
	- database GLOBALS overwrites presetted variables in $globals from file config.local.inc
	- "htmlFrameTop.homeBut" comes from config.local.inc
	- "o.%.editScript" replace edit.tmpl.php script e.g. "o.EXP.editScript"
	- "o.DB_USER.#USERID#.mygroup" : - USER_GROUP_ID of personal user group, if $globals["security_write"] 						 
	- calculated dynamically once per session
 * @param object $sql
 */
function loginGlobals( &$sql ) {
	global $globals;
	
	// get globals: save ONLY these important variables in $_SESSION['globals']
	// if one VAR is an array:
	//   array('v'=>'sys.net2srv', 'type'=>'json'),
	$globalsSave   = array (
	     'app.advmod'
	    ,'app.log.dir'
		,'app.logfile'
		,'DBVersion'
		,'DbArchiveMode'
		,'Db.encoding'
		,'dbms'
		,'img_path'
		,'work_path'
		,'img_convert'
		,'img_identify'
		,'http_cache_path'
		,'data_path'
		,'magasin_serial' 
		,'security_level'
		,'security_write'
	    , array('v'=>'sys.net2srv', 'type'=>'json')
		,'lab_path'
		,'globLogFlag'		// 0,1,...
		
		);
	// 
	  
	foreach($globalsSave as $varx_row) {
	    $typex='default';
	    if (is_array($varx_row)) {
	        $varx = $varx_row['v'];
	        $typex= $varx_row['type'];
	    } else {
	        $varx = $varx_row;
	    }
	    $sql->query("SELECT NAME, VALUE FROM globals where NAME=".$sql->addQuotes($varx) );
		if ( $sql->ReadRow() ) {
		    $val_loop = $sql->RowData[1];
		    if ($typex=='json') {
		        $val_loop=json_decode($val_loop, TRUE);
		    }
		    $globals[$sql->RowData[0]] = $val_loop;
		}
	}
	
	// load edit.tmpl.php forwards
	$sql->query("SELECT NAME, VALUE FROM globals where NAME like 'o.%.editScript'");
	while ( $sql->ReadRow() ) {
		$globals[$sql->RowData[0]] = $sql->RowData[1];
	}
	
	
	// post preparation of $globals
	if ( $globals["htmlFrameTop.homeBut"] =="" ) $globals["htmlFrameTop.homeBut"]="home.php"; 
	if ( $globals["Db.encoding"] == "" )  $globals["Db.encoding"]="ISO-8859-1"; 
	// TBD: - variable "lab_path": 
	//		- test if directory exists !!!
	//      - if not, make string empty, than it is easier to do tests in other script; no Fileexists() needed anymore !!!
	
	$tmppos = strpos($globals["DBVersion"]," ");
	$globals["DBVersionShort"] = substr($globals["DBVersion"],0,$tmppos);

}

/**
 * check if user login expired
 * @param $userid
 * @return $retval
 *    NULL no expiry date
//	  >0 number of days for expiry
//	  -1 expired
 */
function checkExpiry( &$sql, $userid ) {

	$retval = NULL; 
	
	$sql->query('SELECT '.$sql->Sql2DateString('expiry_date').' expiry_date FROM db_user WHERE db_user_id = '.$userid);
	$sql->ReadRow();
	$sql_date=$sql->RowData[0];
	
	if ($sql_date != '') {
		$user_expiry = date_dtsql2unix( $sql, $sql_date );
		$unx_stamp = time();

		if ($unx_stamp>$user_expiry)  $retval = -1;
		else {  
			$retval = (int)(($user_expiry - $unx_stamp) / (3600*24));
		}
	}
	return ($retval);
}

/**
 * CHECK ip NUMBERS
 * FORMAT: comma separated list of "DbLoginIPOnly": 128.1.23.1,192.34.1.67
 * @return $denied
 * 	0, NULL: OK
	-1:      denied
 */
function _loginCheckIP( &$sql ) {
	$sql->query("SELECT value FROM globals WHERE name='DbLoginIPOnly'");
	$sql->ReadRow();
	$tmpstr     = $sql->RowData[0];
	if (trim($tmpstr)=="") return (0);
	
	$ipnow = $_SERVER['REMOTE_ADDR'];
	$denied = -1;
	$tmparr = explode(",", $tmpstr);
	foreach($tmparr as $allowIP) {
	// foreach( $tmparr as $pos=>$allowIP) {
		if (trim($allowIP) == $ipnow) {	// found ???
			$denied=0;
			break;
		}
	}
	return ($denied);
}

/**
 * do a LDAP login
 * @param string $user user name
 * @param string $pw  clear password
 * @param string $ldapserver  e.g. "dc1.clondiag.jena"
 * @param string $cnstring   e.g.  "cn=users,dc=clondiag,dc=jena"
 * @param string $userdomain e.g.  "clondiag.jena" : will extend username
 * @return mixed array(flag, text)
 *		  <pre>
 *		  flag: 0 : undefined
 *              1 : o.k.
 *				2 : no LDAP-support
 *				>=3: login failed
 *		   $this->funclog["LDAP"] 
 * 		  </pre>
 */
function _ldapLoginSub($user, $pw, $ldapserver, $cnstring, $userdomain) {
	
	$retval   = 0; // undefined
	$rettxt   = "";
	$username = $user."@".$userdomain;
	$this->funclog["LDAP"] = "server:".$ldapserver.";username:".$username;
	
	if ( $pw==NULL ) {
		return array(4, "ldap_connect no password given!");
	}
	
	
	if ( !function_exists("ldap_connect") ) {
		return array(2, "ldap_connect not implemented!");
	}
	
	$ad = ldap_connect( $ldapserver );
	if (!$ad) {
		$retval = 2;
		$rettxt = "ldap_connect failed to server: ".$ldapserver;
		return array($retval, $rettxt);
	}
	
	//Set some variables
	ldap_set_option($ad, LDAP_OPT_PROTOCOL_VERSION, 3);
	ldap_set_option($ad, LDAP_OPT_REFERRALS, 0);
	
	//Bind to the ldap directory
	
	$answer = @ldap_bind($ad,$username,$pw);
	# Debugging:
	# $errno  = ldap_errno ( $ad );
	# $errstr = ldap_err2str ($errno);
	# echo "DEBUUUUG:$username|$pw|$answer|$errno|$errstr|$ldapserver<br>";
	
	if ($answer == False) {
		$retval = 3;
		$errno  = ldap_errno ( $ad );
		$errstr = ldap_err2str ($errno);
		$rettxt = "Login User/Password failed. (LDAP: ".$errstr.")";
	} else {
	    $retval = 1; // ok
	}
	
	return array($retval, $rettxt);
}

/**
 * prepare a LDAP login
 * @param string $user user name
 * @param string $pw  clear password
 * @param string $globLdapVar:  "server:dc1.clondiag.jena;server:dc2.clondiag.jena;userdomain:clondiag.jena;userbase:cn=users,dc=clondiag,dc=jena"
 * @return mixed array(flag, text)
 */
function _ldapLogin($user, $pw, $globLdapVar ) {
	
	$globLdapVar = trim($globLdapVar);
	if ($globLdapVar=="") {
		return array(4, "LDAP: globals['DbLogin.LDAP'] invalid.");
	}
	
	$userdomain = NULL;
	$ldapserver = NULL;
	$cnstring   = NULL; // can stay NULL
	
	$ldapArr = explode(";",$globLdapVar );
	foreach( $ldapArr as $keyval_string) {
		
		if ($keyval_string==NULL) continue; // ignore NULL value; e.g. SEMICOLON on end of string
		
		$minivals =  explode(":", $keyval_string );
		if ( sizeof($minivals)<2 ) {
			return array(5, "LDAP: globals['DbLogin.LDAP'] invalid. ldapArr:$keyval_string");
		}
		if ($minivals[0]=="server") {
			$ldapserver = $minivals[1];
		} 
		if ($minivals[0]=="userbase") {
			$cnstring = $minivals[1];
		}
		if ($minivals[0]=="userdomain") {
			$userdomain = $minivals[1];
		}
	}
	reset ($ldapArr);
	
	if ($ldapserver=="" OR $userdomain=="") {
		return array(6, "LDAP: globals['DbLogin.LDAP'] invalid. Missing params.");
	}  
	
	return $this->_ldapLoginSub($user, $pw, $ldapserver, $cnstring, $userdomain);
}

/**
 * try Kerberos KADM5 login
 * @param string $ServerVar example: 'server:dfs1;domain:clondiag.jena'
 */
function _kadm5Login($user, $pw, $ServerVar) {
	$errstr='KADM5: globals[DbLogin.KADM5]';
	
	if ( !function_exists("kadm5_init_with_password") ) {
		return array(1, "kadm5_init_with_password not implemented!");
	}
	
	$ServerVar = trim($ServerVar);
	if ($ServerVar==NULL) {
		return array(4, $errstr.' config empty.');
	}
	$configArr = explode(";",$ServerVar );
	foreach( $configArr as $valarr) {
		$minivals =  explode(":", $valarr );
		if ( sizeof($minivals)<2 ) {
			return array(5, $errstr." invalid. config-part: ".$valarr);
		}
		if ($minivals[0]=="server") {
			$server = $minivals[1];
		} 
		if ($minivals[0]=="domain") {
			$domain = $minivals[1];
		}
	}
	reset ($configArr);
	if ($server=="" OR $domain=="") {
		return array(6, $errstr." invalid. Missing params.");
	}  
	$handle = kadm5_init_with_password($server, $domain, $user, $pw);
	if (!$handle) {
		return array(3, "KADM5 login failed.");
	}
	kadm5_destroy($handle);
}

function rootLoginCheck( &$sql, $cctpwd ) {
	global $error;
	$FUNCNAME="rootLoginCheck";

    $sqls = "select DB_USER_ID, PASS_WORD, su, login_deny from DB_USER  where nick='root'";
    $sql->query($sqls);
    if ( !$sql->ReadRow() )  {
		 $error->set( $FUNCNAME, 3, "User 'root' not found." );
		 return;
	}	
	$root_cryptpw = $sql->RowData[1];

	if ( fPasswordC::verify_pw($root_cryptpw, $cctpwd) ) { 
	    // ok
	} else {
		$error->set( $FUNCNAME, 4, "Wrong password." );
		return;
	} 
	return;
}

/**
 * - check loginMethod for user X
 * @return string $loginMeth : "NORM", "LDAP", "KADM5"
 */
function checkLoginMeth( &$sqlo, $usernick ) {
	
	$loginMeth="NORM";
	$sqlo->query("SELECT name, value FROM globals WHERE name='DbLogin.method'");
	$sqlo->ReadRow();
	$loginMethGlob = $sqlo->RowData[1];
	
	if ( $loginMethGlob!="" and $loginMethGlob!='NORM' AND $usernick!="root" ) {
		$userid = glob_elementDataGet( $sqlo, "DB_USER", "NICK", $usernick, "DB_USER_ID");
		$role_right_name = "g.login.NORM";
		$role_right      = role_check_f($sqlo, $role_right_name, $userid);
		if ( $role_right!="execute" ) {
			$loginMeth = $loginMethGlob;
		}
	}
	return ($loginMeth);
}

/**
 * - check login information for a user
 * - support for LDAP: globals["DbLogin.LDAP"] =
 *         "server:dc1.clondiag.jena;server:dc2.clondiag.jena;userbase:cn=users,dc=clondiag,dc=jena"
 * - support for KADM5: globals["DbLogin.KADM5"] = "server:xxx;domain:xxx"
 *         
 * - if user has role-right: f.g.login.NORM = "execute"  => ignore  $loginMethAr["DbLogin.method"]
 * @param object $sql SQL_handle
 * @param string $passwd clear password
 * @param string $usernick user name
 * @param boolean $su_cctuser >0 : login with root-password
 * @param mixed  $option 
		"loginDateNoSave" : [0],1 do not save "login_last" (for checking last global login)
		[DEPRECATED] "pwMD5" = 1 : $passwd is already MD5() crypted => ignore $loginMethAr["DbLogin.method"]
					              use INTERNAL authorization
 * @return mixed array ( <pre>
	"userid"   => $userid, 
	"usernick" => $usernick (of database)
	"su"       => $suflag, 
	"logok"    => <0 : ERROR; 
			      1 : OK
	 )
	</pre>
 */
function loginCheck ( &$sql, $passwd, $usernick, $su_cctuser, $option = NULL ) {
	global $error;
	
	$error      = & ErrorHandler::get();
	$loggedin   = 1;
	$error_msg  ='Logged in';
	$this->funclog = NULL; // some login log info


	$loginMethAr=NULL;
	$sql->query("SELECT name, value FROM globals WHERE name in ".
			"('DbLoginDeny', 'magasin_serial', 'DbLogin.method', 'DbLogin.LDAP', 'DbLogin.KADM5' )");
	while ( $sql->ReadRow() ) {
		$loginMethAr[$sql->RowData[0]] = $sql->RowData[1];
	}
	
	$sql->query('SELECT nick, pass_word, db_user_id, su, login_deny FROM db_user WHERE nick = '.$sql->addQuotes($usernick));
	
	/**
	 * @var int $pwMatch - passowrd matches ?
	 *  <0 : no match, other method
	 *   0 : no match
	 *  >0 : ok
	 */
	$pwMatch= 0;
	$userid = 0;
	
	if ( $sql->ReadRow() ) {
	    
	  $this->funclog['user_exists_in_DB']='o.k.';
	  $pw_cyrpt_user    = $sql->RowData[1];
	  $userid     = $sql->RowData[2];
	  $suflag     = $sql->RowData[3];
	  $login_deny = $sql->RowData[4];

	  // echo "DDDX:$pw_cyrpt_user|&nbsp;PURE:$passwd|<br>";
	  if ( fPasswordC::verify_pw($pw_cyrpt_user, $passwd) ) {
	  		$pwMatch = 1;  
	  		$this->funclog['db_pw_match']='o.k.';
	  } else {
	  	    $this->funclog['db_pw_match']='no_match';
	  }
	  if ( $pw_cyrpt_user === 'nopasswd' ) {
	  	    $pwMatch = 1; 
	  	    $this->funclog['db_pw_match']='nopasswd: o.k.';
	  }
	}
	
	$alterMethodErr = NULL;
	
	// perform alternative login-method ?
	if (  
		$loginMethAr["DbLogin.method"]!=NULL and 
		$loginMethAr["DbLogin.method"]!='NORM' and 
		$usernick!="root" and 
		$userid>0 
		) {
		// only for normal users ...
		// only, if input-password is NOT MD5 crypted
		// check, if user can use NORMAL login method
		$role_right_name = "g.login.NORM";
		$role_right      = role_check_f_pure($sql, $role_right_name, $userid);
		
		$this->funclog["g.login.NORM?"] = $role_right.':userid:'.$userid;
		
		if ( $role_right!="execute" ) {
			
			$pwMatch = 0; // reset
			switch ($loginMethAr["DbLogin.method"]) {
				case "LDAP":
					$this->funclog["method"] = "LDAP";
					$ldapAnswer = $this->_ldapLogin($usernick, $passwd, $loginMethAr["DbLogin.LDAP"]);
					if ( $ldapAnswer[0]==1 ) {
					    $pwMatch = 1;
					} else {
						$pwMatch = -1; // alternative method
						$this->funclog["LDAP_error"] = $ldapAnswer[0].":".$ldapAnswer[1];
						$alterMethodErr = $ldapAnswer[1];
					}
					break;
				
			}
		}
	}
	
	if ( !empty($su_cctuser) ) { // log in as different user
		$sql->query("SELECT pass_word FROM db_user WHERE nick='root'" );
		if ( $sql->ReadRow() ) {
		    $pwroot_crypt=$sql->RowData[0];
		    if ( fPasswordC::verify_pw($pwroot_crypt, $passwd) ) {
				$pwMatch = 1;
				$this->funclog["su_match"]=1;
			}
		}
	}
	 
	do {
	
		// DO NOT CHANGE THE ORDER of the tests !!!
		
		if (!$userid) { // must be checked, if logged in with LDAP, but user not exists on Partisan ...
			$error_msg ='User or Password invalid!';
			$loggedin  = -7;
		}
		
		if ($alterMethodErr!=NULL) {
		    $error_msg = 'User or Password invalid!';
			$loggedin  = -8;
		    break;
		}
		
		if ($this->_loginCheckIP( $sql ) < 0 ) {
			$error_msg ='IP-number of your client is not registered for this database!';
			$loggedin  = -5;
		    break;
		}
	
		if ( $pwMatch < 1 ) { 
			$error_msg ='User name "'.$usernick.'" or password invalid!';
			// if ($pwMatch==-1)  $error_msg .= " (".$this->funclog["method"].")";
			$loggedin  = -1;
			break;
		}
		 
		if ( $usernick=="root"  ) break; // no more tests for root !
		
		if ( $login_deny>0) { 
			$error_msg = "Login for user " . $usernick. " deactivated!";
			$loggedin  = -2;
			break;
		}
		
		$login_expiry = $this->checkExpiry($sql, $userid );
		if ( $login_expiry<0 ) { 
			$error_msg = "Login for user " . $usernick. " expired!";
			$loggedin  = -3;
			break;
		}
		
		if ( $loginMethAr["DbLoginDeny"] != "" AND  $loginMethAr["DbLoginDeny"] != "0") { 
			$error_msg = "Login for all users denied! Reason: ".$loginMethAr["DbLoginDeny"];
			$loggedin  = -4;
			break;
		}
		
	} while (0);
	

	if ( $loggedin < 0 ) {
	
	  // @swreq UREQ:0001688:LOG1: log all "login fail" actions; asked by FDA21CFRPart11 
	  $error->set("loginCheck()", abs($loggedin), $error_msg);
	  if ($loggedin!=-4) {
	  	// log error to log-file !
	  	$logmess = "LOGIN: magasin_serial:".$loginMethAr["magasin_serial"]." login_error:".abs($loggedin)." :".$error_msg;
	  	if (is_array($this->funclog)) {
	  		$logmess .= '; extra: '.glob_array2String( $this->funclog,1 );
	  	}
	  	$error->logx("USERERROR", 3, $logmess);
	  }
	  return;
	}
	
	// now to last login-time
	if ( $option["loginDateNoSave"] !=1 ) {
		$sql->Updatex("db_user", 'login_last = '.$sql->Timestamp2Sql().' WHERE db_user_id = '.$userid);
 	}
 	return array("userid"=>$userid, "usernick"=>$usernick, "su"=>$suflag, "logok"=>$loggedin);
}

/**
 * check only the password, log errors
 * - can also use LDAP, if configured
 * - needed for fSignatureSub::checkPW()
 * @return array
 *   ('ok'=>0,1, 'reason'=>text)
 */
function checkOnlyPW(&$sqlo, $passwd, $usernick) {
	global $error;
	
	$userid  = $_SESSION['sec']['db_user_id'];
	$pwMatch = 0;
	$extinfo = NULL;
	
	$loginMethAr=NULL;
	$sqlo->query("SELECT name, value FROM globals WHERE name in ".
			"( 'DbLogin.method', 'DbLogin.LDAP' )");
	while ( $sqlo->ReadRow() ) {
		$loginMethAr[$sqlo->RowData[0]] = $sqlo->RowData[1];
	}
	
	// perform alternative login-method ?
	if ($usernick=="root") $loginMethAr["DbLogin.method"] = NULL; // use internal Password
	
	if ( $loginMethAr["DbLogin.method"]!=NULL ) {
		// only for normal users ...
		// check, if user can use NORMAL login method
		$role_right_name = "g.login.NORM";
		$role_right      = role_check_f($sqlo, $role_right_name, $userid);
		
		if ( $role_right=="execute" ) {
			$loginMethAr["DbLogin.method"] = NULL; // use internal login
		}
	}
			
	$pwMatch = 0; // reset
	switch ($loginMethAr["DbLogin.method"]) {
		case "LDAP":
			$ldapAnswer = $this->_ldapLogin($usernick, $passwd, $loginMethAr["DbLogin.LDAP"]);
			if ( $ldapAnswer[0]==1 ) {
			    $pwMatch = 1;
			} else {
			    $pwMatch = 0; // alternative method
			    $extinfo = ' DbLogin.method:'.$loginMethAr["DbLogin.method"].':'.$ldapAnswer[1];
			}
			break;
		default:
		    
			$sqlo->Quesel('nick, pass_word FROM db_user WHERE nick like '.$sqlo->addQuotes($usernick));
			$sqlo->ReadRow();
			$pwdb = $sqlo->RowData[1];
			
			if ( fPasswordC::verify_pw($pwdb, $passwd) )  $pwMatch = 1; 
	}

	       

	// @swreq UREQ:0001688:LOG1: log all "login fail" actions to logfile; asked by FDA21CFRPart11 
	if (!$pwMatch) {
	  	$logmess = 'LOGIN:checkOnlyPW() password_error:: nick:"'.$usernick.'" :no password match.'.$extinfo ;
	  	$error->logx("USERERROR", 3, $logmess);
	}
	return array('ok'=>$pwMatch, 'reason'=>'wrong password ');
}

function getLoginLog() {
	return ($this->funclog);
}

/**
 * set $s_sessVars after loginGlobals()
 */
function setSessionVars(&$s_sessVars) {
	global $globals;
	
	$DIRSEP = DIRECTORY_SEPARATOR;
	// post preparation of $s_sessVars
	$s_sessVars["httpCacheDir"] = $s_sessVars["loginPATH"]."/". $globals["http_cache_path"];
	$s_sessVars["httpCacheURL"] = $s_sessVars["loginURL"] ."/". $globals["http_cache_path"];
	if ( $globals["lab_path"]!=NULL ) {
		$s_sessVars['AppLabLibDir'] = $s_sessVars['AppRootDir'] . $DIRSEP . 'www'.  $DIRSEP . $globals["lab_path"];
	}
	$s_sessVars["phpVersMain"] = intval(substr(phpversion(),0,1)); // PHP-Version-Main-Number: e.g. "4" or "5"
	$s_sessVars["REMOTE_ADDR"] = $_SERVER['REMOTE_ADDR']; // IP of client
}

}