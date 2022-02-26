<?php 
/**
 * REST connect to database, get SESSION-ID
 * OUTPUT:
 *   header("icono-err-code: {NUM}");     // NUM: 0 : ok; NUM>0: ERROR
 *   header("icono-err-text: {MESSAGE}"); // text message
 *   header("sess_id:".  SESSION-ID);
 *   header("Content-type: text/html");
 *   
 * @package rconnect.php
 * @swreq UREQ:FS-INT02.R01 REST connect
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param  string $user   
 * @param  string $pwBase64
 * @param  int  $dbid
 * @version $Header: trunk/src/www/pionir/xmlrpc/rconnect.php 59 2018-11-21 09:04:09Z $
 * @unittest exists
 */

require_once ('reqnormal.inc');
require_once("../../../config/config.local.inc");

require_once("logincheck.inc");
require_once("../api/0_connect_sub.inc");
require_once("f.modulLog.inc");
require_once('rest_support.inc');

$error = & ErrorHandler::get();
$rest_lib = new rest_support();


global $error;
$FUNCNAME= 'rconnect.php';

$loginDateNoSave = 0;

//Set variables
$cctuser      = $_REQUEST['user'];
$cctPwBase64  = $_REQUEST['pwBase64'];
$dbid         = $_REQUEST['dbid'];
if ( !$dbid ) $dbid=0;


if (!is_array($database_access[$dbid]) )
	$rest_lib->errorout("database service '$dbid' not found in server configuration!", 4);

$db		= $database_access[$dbid]["db"];
$LogName= $database_access[$dbid]["LogName"];
$passwd	= $database_access[$dbid]["passwd"];
$_dbtype= $database_access[$dbid]["_dbtype"];
$dbalias = $database_access[$dbid]["alias"];		// new: web_service_name



// Try the database connection
// Open the database connection
$error = & ErrorHandler::get();
$sql   = logon_to_db($LogName, $passwd, $db, $_dbtype, '', false);
if(!$sql) $rest_lib->errorout("DB User and/or  password incorect", 2); // DB User and/or  password incorect

// Test the cct_user and pass
// $cctpwd is base64(pw)
$cctpwd      =  base64_decode($cctPwBase64);
	
$logopt = NULL;
if ($loginDateNoSave) $logopt["loginDateNoSave"] = $loginDateNoSave;

$loginLib = new fLoginC();
$loginfo = $loginLib->loginCheck($sql, $cctpwd, $cctuser, "", $logopt);

if ($loginfo["logok"] < 1) {
	$errLast = $error->getLast();
	$rest_lib->errorout($FUNCNAME.":".$errLast->text, 3); // put the error message
}

$initLib = new fConnect();
$init_set_arr = array();
$tempstr  	     = $_SERVER['SCRIPT_FILENAME'];
$init_set_arr['FILENAME'] = str_replace("/xmlrpc", "", $tempstr); 
$init_set_arr['PHP_SELF'] = str_replace("/xmlrpc", "",$_SERVER['PHP_SELF']);

$initLib->initvars($sql, $loginfo, $LogName, $passwd, $db, $_dbtype, $init_set_arr);

$modLogLib = new fModulLogC();
$sqldummy  = NULL;
$modLogLib->logModul($sqldummy, substr($FUNCNAME,0,-4), 0);

$tmpsess = session_id() ;

header("icono-err-code: 0");
header("icono-err-text: Everything ok !");
header("sess_id:".  $tmpsess );
header("Content-type: text/html");
echo $tmpsess;
