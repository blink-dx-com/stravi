<?php
/**
 * - called by cronjob
 * use for standard $actions: XMLRPC-function "cronjob"
 * - example# /usr/bin/php /opt/partisan/lab/server_scripts/json_rpc_client_vario.php UNITTEST
 * 0 9 * * MON 
 * @param ARGU[0] : action
 *   'UNITTEST'
 *   ... more ..
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @package xml_rpc_client_vario.php
 * @swreq UREQ:35 (FS-ARC01-01) g.cronjob: support cronjobs in a crontab
 */
//DO NOT START a session here with session_start(); 


require_once (dirname(__FILE__).'/../../phplib/db_access.inc');
require_once (dirname(__FILE__).'/../../phplib/globals.inc');
require_once (dirname(__FILE__).'/../../www/pionir/api/jsonrpc_client.inc');


function load_json_config() {
    include (dirname(__FILE__)."/../../config/config.service.inc");
    return $connect_params;
}

$scriptx = basename(__FILE__);
$action  = $argv[1];

echo "XXX: ".$scriptx.": Start; Action: ".$action."\n";

if ($action==NULL) {
	echo "ERROR: No input-action given!\n";
	return;
}


$jsonrpc_config = load_json_config();
if (!is_array($jsonrpc_config)) {
	echo "ERROR: Config-data missing.\n";
	exit;
}

$error = & ErrorHandler::get();

// Send your requests to the remote server:
try {
    
   $rpclib = new goz_rcp_client($jsonrpc_config);
    
    
   echo "- Try to connect ...\n";
   $rpclib->connect();

   echo "- Connected ...\n";
   $params = array($action); // e.g. 'OEE.SHIFT_STOP'
   $answer = $rpclib->call("LAB/cronjob", $params );
   echo "- Called ...\n";
    
} catch (Exception $exception) {
    $mess = $exception->getMessage();
    echo "ERROR:JSONRPC-CALL ".$mess."\n";
    exit; // break here !
}


$result = "flag:".$answer[0]."; details:";
if (is_array($answer[1])) $result .= print_r($answer[1],1);
else  $result .= print_r($answer[1],1);
echo $scriptx.': RESULT:'.$result."\n";
echo $scriptx.": END\n";



