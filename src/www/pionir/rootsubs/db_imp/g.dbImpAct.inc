<?php
/**
 * - provide functions for database dump import
 * - currently only ORACLE
 * - DB-user generation
 * 
 * Header: www/pionir/rootsubs/db_imp/g.dbImpAct.inc 59 2018-11-21 09:04:09Z $
 * @package g.dbImpAct
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */

// pure actions
class db_imp_act {

var $db_admin;
	
function __construct() {
    global $error;
    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
    
	$this->roleCCT = NULL;
	$this->modiAllow = 0;
	$this->showCmd = 0;
	$this->OraPath = array();
	$this->OraPath['home'] = '$ORACLE_HOME';
	$this->OraPath['bin']  = $this->OraPath['home'] . '/bin';
	
	$this->app_code_root = realpath(dirname(__FILE__).'/../../../..');
	if ($this->app_code_root==NULL) {
        $error->set( $FUNCNAME, 1, 'could not define app_code_root.' );
        return;
	}
	
}

// $go = 0 : no action, = 1 : allow modifications
function setModiFlag($go) {
	$this->modiAllow = $go;
}
function setshowCmd($show) {
	$this->showCmd = $show;
}

function _info( $text, $textraw=NULL ) {
	if ($text!=NULL) echo "INFO: ".$text."\n";
	if ($textraw!=NULL) echo $textraw;
}

function _getUserTabName($user) {
	$user_tab = $user."_TAB";
	return ($user_tab);
}
function _getUserDbfName($user) {
	$user_tab = $this->_getUserTabName($user);
	$dbfname = strtolower($user_tab) . '.dbf';
	return ($dbfname);
}

function _shCmd( $key, $cmd) {
	echo "CMD:".$key.': '.$cmd."\n";
}

function getAdminParams() {
	return $this->db_admin;
}

// @global $this->db_admin
function &login( &$db_admin ) {
	$useCache = False;
	$this->db_admin = $db_admin;
	$sqlSys = logon_to_db( $db_admin["user"], $db_admin["pw"], $db_admin["db"], $db_admin["dbtype"], "std", $useCache );
	return ($sqlSys);
}

// normal user login
function &userlogin( &$loginarr ) {
	$useCache = False;
	$dummy  = NULL;
	$sqlo = logon_to_db( $loginarr["user"], $loginarr["pw"], $loginarr["db"], $loginarr["dbtype"], "std", $useCache );
	return ($sqlo);
}

function &doLogin($parx) {
	global $error, $db,$_dbtype;
	$FUNCNAME = "doLogin";
	
	if ($parx["user"] == "" OR $parx["pw"]=="" OR  $parx["db"]=="") {
		$error->set( $FUNCNAME, 1, "Give user, password and database." );
		return;
	} 
	
	$db_admin = array(
		"user"=>   $parx["user"],
		"pw"=>     $parx["pw"],
		"db"=>     $parx["db"]
		 );
	
	$sqlSys = $this->login($db_admin);
	
	if ($error->Got(READONLY))  {
    	$error->set( $FUNCNAME, 2, "Login failed." );
		return;
	}
	
	return (array($sqlSys,$db_admin) );
}

function _execQueries( &$sqlSys, &$sqla) {
	global $error;
	$FUNCNAME= "_execQueries";
	
	$modiAllow = 0;
	if ( $this->modiAllow>0 ) $modiAllow = 1;

	foreach( $sqla as $dummy=>$sqls) {
		if ($this->showCmd) $this->_shCmd( "SQL", $sqls);
		if ($modiAllow>0) $ret = $sqlSys->query($sqls);
		else $ret=2;
		
		if (!$ret) {
			echo "ERROR: ". htmlspecialchars($sqls)."<br>";
			$error->set( $FUNCNAME, 1, "SQL-statement failed!" );
			return;
		}
	}
	
	
}

// exec command
function _sysexec( $cmdin ) {

	$cmd = $cmdin . " 2>&1"; // pipe error	
	// $cmdDo = "echo '".$rootpw."' | /usr/bin/sudo -u root -S ". $cmd;
	if ($this->showCmd) $this->_shCmd( "SYS", $cmd);	  
	if ( $this->modiAllow<=0 ) return;
	
	$this->_info( "sysexec: ".htmlspecialchars($cmd)."\n");
	// $answer = shell_exec ( $cmd );
	$retvar = NULL;
	//$this->_info( NULL, "<pre>\n" );
	$answer = system ( $cmd, $retvar );
	//$this->_info(  NULL, "<pre>\n" );
	
	$this->_info( "answer: ".htmlspecialchars($answer). " ret: ".$retvar);
}
	
function checkRole(&$sqlSys) {
	// check role CCT_USER
	$this->roleCCT = NULL;
	$sqls = "SELECT ROLE FROM DBA_ROLES where ROLE='CCT_USER'";
	$sqlSys->query($sqls);
	$sqlSys->ReadRow();
	$roleCCT = $sqlSys->RowData[0];
	$this->roleCCT = $roleCCT;
	return ($roleCCT);
}

function tableSpaceDirGet(&$sqlSys) {
	$sqls = "SELECT file_name FROM dba_data_files where tablespace_name='SYSTEM'";
	$sqlSys->query($sqls);
	$sqlSys->ReadRow();
	$tablesSpaceFile = $sqlSys->RowData[0];
	$tablesSpaceDir = dirname ($tablesSpaceFile);
	return ($tablesSpaceDir);
}

function getSpaceUsersSQL() {
	$sqls = "SELECT file_name, tablespace_name FROM dba_data_files where tablespace_name like '%_TAB'";
	return ($sqls );
}

function SpaceUserCheck(&$sqlSys, $user ) {
	$user_tab = $this->_getUserTabName($user);

	$sqls = "SELECT file_name FROM dba_data_files where upper(tablespace_name) like "
				. $sqlSys->addQuotes ($user_tab);
	$sqlSys->query($sqls);
	if ( !$sqlSys->ReadRow() ) {
		return;
	}
	$userfile = $sqlSys->RowData[0];
	return ( $userfile );
}
	
// @global $this->roleCCT
// @return: -1 : error, 1 : o.k.
function dbUserCheck( &$sqlSys, $user ) {
	global $error;
	$FUNCNAME = "dbUserCheck";

	if ( $this->roleCCT==NULL ) {
		$error->set( $FUNCNAME, 1, 'Intern: roleCCT missing.' );
		return;
	}
	
	$sqls = "SELECT GRANTEE FROM  DBA_ROLE_PRIVS where GRANTED_ROLE=".
				 $sqlSys->addQuotes ($this->roleCCT) .
			   " AND GRANTEE=". $sqlSys->addQuotes ($user)  ;
	$sqlSys->query($sqls, $FUNCNAME );
	if ( $sqlSys->ReadRow() ) {
		return (1);
	}
	
	return (0);
}

function dbUserGetAllSQL() {
	$sqls = "SELECT GRANTEE FROM  DBA_ROLE_PRIVS where GRANTED_ROLE='".$this->roleCCT."'";
	return ($sqls );
}

function do_userCrea( &$sqlSys, $newuser, $pw) {
	$this->_info( "create user: $newuser");
	
	$newuser_tab = $this->_getUserTabName($newuser);
	$sqla[] = "CREATE USER ".$newuser." IDENTIFIED BY \"".$pw."\"";
	$sqla[] = "GRANT cct_user TO ".$newuser;
	$sqla[] = "ALTER USER ".$newuser." DEFAULT TABLESPACE ".$newuser_tab;
	$sqla[] = "ALTER USER ".$newuser." QUOTA UNLIMITED ON ".$newuser_tab;
	
	$this->_execQueries( $sqlSys, $sqla);
}

function do_userRootPW(&$sqlSys, $newuser) {
	$this->_info( "reset password for user root on TABLESPACE: $newuser");
	
	$sqls = "SELECT count(1) FROM ".$newuser.".DB_USER where NICK='root'";
	$sqlSys->query($sqls);
	$sqlSys->ReadRow();
	$rootExists = $sqlSys->RowData[0];
	if (!$rootExists) {
		$sqla=NULL;
		$sqla[] = "INSERT INTO ".$newuser.".DB_USER (LOGIN_DENY, NOTES, FULL_NAME, NICK, PASS_WORD, EMAIL, SHOWN_EMAIL) 
			VALUES ('0', 'just root', 'Root', 'root', 'XXX', 'root@noemail.com', null)";
		$this->_execQueries( $sqlSys, $sqla);
	}
	
	$sqla=NULL;
	$sqla[] = "update ".$newuser.".DB_USER set pass_word='nopasswd' where nick='root'"; // CODE_INFO: update allowed here
	$this->_execQueries( $sqlSys, $sqla);
}

function do_userDrop(&$sqlSys, $olduser) {
	$sqla=NULL;
	$sqla[] = "drop user ".$olduser." cascade";
	$this->_execQueries( $sqlSys, $sqla);
}

function do_tabSpace(&$sqlSys, $newuser, $datadir) {

    throw new Exception('Not implemented');
}

function do_tabSpaceDrop(&$sqlSys, $user) {
	$this->_info( "drop TABLE-space ...");
	throw new Exception('Not implemented');
}

/**
 * do ORACLE import 'imp '
 * @param  $opt : 'seltabs' = string of tablenames 'APP_DATA_TYPE, CCT_COLUMN ...'
 */
function do_impDump($newuser, $fromuser, $file, $opt=NULL) {
	global $error;
	$FUNCNAME = "do_impDump";
	
	$this->_info(  "Import ...\n");
	
	throw new Exception('Not implemented');

	$this->_sysexec( $cmd );
}

// export dump
function do_expDump($user, $pw, $dbBackupDir) {
	global $error;
	$FUNCNAME = "do_expDump";

	$this->_info(  "Export dump ...\n");
	

	throw new Exception('Not implemented');
}

function do_shConfig($dbid, $dbname, $user, $userpw, $alias) {
	$outx = NULL;
	$outx .=  '$database_access["'.$dbid.'"]["alias"]    = "'.$alias.'";'."\n";
	$outx .= '$database_access["'.$dbid.'"]["db"]       = "'.$dbname.'";'."\n";
	$outx .= '$database_access["'.$dbid.'"]["LogName"]  = "'.$user.'";'."\n";
	$outx .= '$database_access["'.$dbid.'"]["passwd"]   = "'.$userpw.'";'."\n";
	return ($outx);

}

/**
 * mkdir
 */
function do_mkdirs($mkdirs) {
	global $error;
	$FUNCNAME= 'do_mkdirs';
	
	foreach( $mkdirs as $key=>$tmparr) {
	
		$dirx=$tmparr[0];
		$doParentCheck = $tmparr[1];
		
		if ($this->showCmd) $this->_shCmd( "PHP", 'mkdir("'. $dirx.'")   (key:'.$key.')' );	
		
		if ( $doParentCheck ) {

			$parentDir = dirname($dirx);
			if ( !is_writable  ( $parentDir ) ) {
				$error->set( $FUNCNAME, 1, 'parent directory '.$parentDir.' of '.$key.' = "'.$dirx.'" is not writeable.' );
				return;
			}
		}
		
		if (file_exists($dirx)) {
			$this->_shCmd( "PHP", 'already exists ...');	
			continue; // already exists
		}
		
		if ( $this->modiAllow>0 ) {
			if ( !mkdir( $dirx ) ) {
				$error->set( $FUNCNAME, 1, 'creation of directory '.$key.' = "'.$dirx.'" failed.' );
				return;
			}
			
			$this->_sysexec( 'chown www-data:www-data '.$dirx );
		}
	}
	
}

function do_saveGlobals(&$sqlo, $appdirs) {
	$this->_info( "set GLOBALS vars");
	$sqla=array();
	foreach( $appdirs as $key=>$dirx) {
	    $sqla[] = "update GLOBALS set VALUE=". $sqlo->addQuotes($dirx) 
			. " where NAME=".$sqlo->addQuotes($key);
	}
	$this->_execQueries( $sqlo, $sqla);

}

/**
 * application data
 * @param  $sqlSys
 * @param  $newuser
 * @param  $configDict
 * 	'appDataDir'
 *  
 */
function do_appDirsSet( &$sqlSys, $newuser, $configDict ) {
	global $error;
	$FUNCNAME= 'do_appDirsSet';


	$appDataDir = $configDict['appDataDir'];
	if ( !file_exists($appDataDir)) {
		$error->set( $FUNCNAME, 1, 'appDataDir "'.$appDataDir.'" does not exists.' );
		return;
	}
	if ( !is_writable($appDataDir)) {
		$error->set( $FUNCNAME, 3, 'appDataDir "'.$appDataDir.'" is not writeable.' );
		return;
	}

	
	$userLow = strtolower($newuser);
	$userDirRoot = $appDataDir .'/'. $userLow;
	
	/*
	if ( file_exists($userDirRoot)) {
		$error->set( $FUNCNAME, 2, 'userDirRoot "'.$userDirRoot.'" already exists.' );
		return;
	}
	*/
	
	
	//$tempHtmlDir = $configDict['appTempRoot'];
	$htmlTemp = 'htmlTemp.'.$userLow;
	$appdirs=array();
	//$appdirs['img_path']  = $userDirRoot  . '/' . 'images';
	$appdirs['data_path'] = $userDirRoot  . '/' . 'data';
	$appdirs['work_path'] = $userDirRoot  . '/' . 'tmp';
	$appdirs['http_cache_path'] = '../'.$htmlTemp;

	$mkdirs=NULL;
	$mkdirs['userDirRoot'] 	= array( $userDirRoot, 1);
	//$mkdirs['img_path'] 	= array( $appdirs['img_path'], 0);
	$mkdirs['data_path'] 	= array( $appdirs['data_path'], 0);
	$mkdirs['work_path'] 	= array( $appdirs['work_path'], 0);
	$mkdirs['http_cache_path'] =  array( $userDirRoot  . '/' . $htmlTemp , 0);
	
	
	$this->do_mkdirs($mkdirs);
	if ($error->Got(READONLY))  return;
	
	$temp_link_file = $this->app_code_root. '/www/'.$htmlTemp;
	if (file_exists($temp_link_file)) {
	    unlink($temp_link_file);
	}
	
	$this->_sysexec( 'ln -s '.$mkdirs['http_cache_path'][0]. ' '.$temp_link_file );

	// save in $appdirs in database
	$this->do_saveGlobals($sqlSys, $appdirs);

}

function do_set_serial( &$sqlo, $newuser, $magasin_serial ) {
    
    if (!is_numeric($magasin_serial)) {
        throw new Exception('magasin_serial: "'.$magasin_serial.'" must be a number.');
    }
    
    $tmparr=array(
        'magasin_serial'=>$magasin_serial
    );
    $this->do_saveGlobals($sqlo, $tmparr);
}

/**
 * create SYS-Views
 * @param  object &$sqlo database-object of USER-login
 */
function do_userTableViews( &$sqlo ) {
    throw new Exception('Not implemented');
}

/**
 * reset all sequences
 * @param  $sqlo
 */
function do_sequenceReset(&$sqlSys, $user) {
	throw new Exception('Not implemented');
	
}

}