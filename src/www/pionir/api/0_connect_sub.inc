<?php
require_once( dirname(__FILE__)."/../main.sessvars.inc" ); 

/**
 * connect sub functions for API connections
 * @author  Steffen Kube (clondiag)
 * @swreq SREQ:0001070: g > XMLRPC > s_sessVars : harmonize 
 * @version $Header: trunk/src/www/pionir/api/0_connect_sub.inc 59 2018-11-21 09:04:09Z $
 **/
class fConnect {
	
/**
 * 
 * @param object $sql
 * @param array $loginfo
 *  "userid"
 *  "usernick"
 * @param string $LogName
 * @param string $passwd
 * @param string $db
 * @param string $_dbtype
 * @param array $settings
 *    'PHP_SELF'=> URL in Pionir-DIR
 *    'FILENAME'=> FILENAME+PATH in Pionir-Dir
 */
function initvars( &$sql, $loginfo, $LogName, $passwd, $db, $_dbtype, $settings) {
    global $globals, $error, $s_sessVars,  $s_product;
	
	$db_user_id = $loginfo["userid"];
	$suflag		= $loginfo["su"];
	$cctuser    = $loginfo["usernick"];
	
	$s_sessVars = array();
	
	$error = & ErrorHandler::get();
	
	// get the same values of $s_sessVars like main.php (the web-gui-entry)
	
	
	makeSessVars($settings['PHP_SELF'], $settings['FILENAME']); // $php_self_str, $script_file_str
		
	$loginLib = new fLoginC();
	$loginLib->loginGlobals( $sql );
	$loginLib->setSessionVars($s_sessVars);
	
	// get some $userGlob vars, do not import all, to save memory and time ...
	$userGlob     = array(); // still empty
	$userglobKeys = array(
		"g.sql_logging",
		"g.debugKey",
		"g.debugLevel"
			);		
	$varsql = NULL;
	$komma  = "";
	foreach( $userglobKeys as $val) {
		$varsql .= $komma . "'".$val."'";
		$komma =",";
	}
	
	
	$sqls="select VAR_NAME, VALUE from user_pref where DB_USER_ID=".$db_user_id." AND VAR_NAME in (".$varsql.")";
	$sql->query($sqls);
	while ($sql->ReadRow()) {
		$userGlob[$sql->RowData[0]] = $sql->RowData[1];   
	}
	
 
	session_start(); 

	// $_SESSION['cctpwd'] = $cctpwd; // no need anymore
	
	$_SESSION['s_sessVars']= $s_sessVars;
	$_SESSION['sec']=array();
	$_SESSION['sec']['dbuser']=    $LogName;	  // database user
	$_SESSION['sec']['appuser']=    $cctuser;	  // application user nick
	$_SESSION['sec']['db_user_id']= $db_user_id; // application user DB_USER_ID
	$_SESSION['sec']['passwd'] =    $passwd;	  // database user password
	$_SESSION['sec']['db']     =    $db;
	$_SESSION['sec']['_dbtype']=    $_dbtype;	
	$_SESSION['globals']=    $globals;
	$_SESSION['userGlob']   =$userGlob;	 // * @link file://CCT_QM_doc/89_1002_SDS_code.pdf#var:userGlob
	$_SESSION['s_formState']=array();	 // description below
	$_SESSION['s_product']  =$s_product; // gozilla product definitions
	
	require_once("init_cache.inc");
	$CacheLib = new gInitCache();
	$CacheLib->init_cache($LogName, $passwd, $db, $_dbtype);

	$error->resetWarnings(); // warnings from init_cahce ...
	

}

}