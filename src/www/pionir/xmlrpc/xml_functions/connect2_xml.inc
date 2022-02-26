<?php
/**
 * @namespace core::xmlrpc
 */


require_once("logincheck.inc");
require_once(dirname(__FILE__)."/../../api/0_connect_sub.inc"); 
require_once("f.modulLog.inc");
   
/**
* open session for XML-RPC to AppServer
*
* - connect via clear password
* - for external usage call via SSL to protect the password
*
* @author  Steffen Kube
* @param array[int]  $f_param <pre>
* [0] partisan user nick
* [1] $cctpw base64-coded user password
* [2] $dbid  database identifier; index of $database_access (in config.local.inc)
* </pre>
* @return array[int]string
*		<pre>
* array( 0, session_id)     : o.k.
* array( Error, Error_text) : Error
*	   </pre>
*/
function connect2_xml( $f_name,$f_param ){
	global $cctuser,$db,$LogName,$passwd,$_dbtype,$database_access;
	global $error;
	$FUNCNAME="connect2_xml";
	
	// Set the values from config.local.inc for the first database in the list
	// TBD: load config.local.inc only at connect ...
	$dbid	 = "";	// default
	$dbalias = $database_access[0]["alias"];		// new: web_service_name
	$db		 = $database_access[0]["db"];
	$LogName = $database_access[0]["LogName"];
	$passwd	 = $database_access[0]["passwd"];
	$_dbtype = $database_access[0]["_dbtype"];
	
	
	$loginDateNoSave = 0;
	
	if( sizeof($f_param)<3 )
			return array(1,"Incorrect number of parameters");
	
	//Set variables
	$cctuser      = $f_param[0];
	$cctPwBase64  = $f_param[1];
	

	if($f_param[2]!="") $dbid    = $f_param[2];
	
	if ( (is_string($dbid) or !is_numeric($dbid)) and $dbid!=NULL ) {	// if $dbid set => take params from $database_access
		
		if (!is_array($database_access[$dbid]) ) {
			return array(4, 'database service "'.$dbid.'" not found in server configuration!');
		}
		
		$db		= $database_access[$dbid]["db"];
		$LogName= $database_access[$dbid]["LogName"];
		$passwd	= $database_access[$dbid]["passwd"];
		$_dbtype= $database_access[$dbid]["_dbtype"];
	} 
	
	
	// Try the database connection
	// Open the database connection
	$error = & ErrorHandler::get();
	$sql   = logon_to_db($LogName, $passwd, $db, $_dbtype, '', false);
	if(!$sql) return array(2,"DB User and/or  password incorect"); // DB User and/or  password incorect
	
	// Test the cct_user and pass
	// $cctpwd is base64(pw)
	$cctpwd      =  base64_decode($cctPwBase64);
			
	$logopt = NULL;
	if ($loginDateNoSave) $logopt["loginDateNoSave"] = $loginDateNoSave;
	
	$loginLib = new fLoginC();
	$loginfo = $loginLib->loginCheck($sql, $cctpwd, $cctuser, "", $logopt);
	
	if ($loginfo["logok"] < 1) {
		$errLast = $error->getLast();
		return array(3, $FUNCNAME.":".$errLast->text); // put the error message
	}
		
	$initLib = new fConnect();
	
	$init_set_arr = array();
	$tempstr  	     = $_SERVER['SCRIPT_FILENAME'];
	$init_set_arr['FILENAME']         = str_replace("/xmlrpc", "", $tempstr); // get ".../www/pionir"
	$init_set_arr['PHP_SELF']    = str_replace("/xmlrpc", "",$_SERVER['PHP_SELF']);
	
	$initLib->initvars($sql, $loginfo, $LogName, $passwd, $db, $_dbtype, $init_set_arr);
	
	$modLogLib = new fModulLogC();
	$sqldummy  = NULL;
	$modLogLib->logModul($sqldummy, 'XMLRPC:'.substr($FUNCNAME,0,-4), 0); 
	
	
		
	return array(0,session_id());

};
