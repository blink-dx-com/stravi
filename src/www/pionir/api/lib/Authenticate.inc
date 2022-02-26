<?php
require_once("logincheck.inc");
require_once('0_connect_sub.inc');
require_once 'f.debug.inc';

// namespace JsonRpcGoz;


/**
 * authenticate
 * @package AuthenticatedServer.inc
 */
class Authenticate
{
    

    /**
     * 
     * 
     * @param unknown $sess_id
     * @param unknown $methodname
     * @return array(
     *    $errnum,
     *    $message
     */
    public function startSession($sess_id, $methodname) {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        if( $sess_id==NULL ) return array(-1,"no session ID given.");
              
        session_id($sess_id);
        session_start();

        if( $_SESSION['sec']['appuser']==NULL ) return array(-2,"session not active."); //invalid session id
        
        $modLogLib = new fModulLogC();
        $sqldummy  = NULL;
        $modLogLib->logModul($sqldummy, 'JSONRPC:'.$methodname);
        
        return array(1,"");
    }
    
    /**
     * 
     * @param array $arguments
     *   'dbid' or 'cid' : identifier of customer DBID, where 'cid' comes from HUBE or HUBD technology
     * @return number[]|string[]
     */
    public function checklogin($arguments)
    {
        global $database_access, $database_dbid_def; // comes from the config.local file
        global $cctuser;
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $error = & ErrorHandler::get();
        
        $cctuser = &$_SERVER['PHP_AUTH_USER'];
        $cctpwd  = &$_SERVER['PHP_AUTH_PW'];
        
        $dbid = '';
        if (array_key_exists('dbid', $arguments) ) {
            $dbid = $arguments['dbid'];
        }
        if (array_key_exists('cid', $arguments) ) {
            $dbid = $arguments['cid'];
        }
        
        if ($dbid=='') {
            # use default DBID
            if ($database_dbid_def!='') {
                $dbid = $database_dbid_def;
            }
        }
        
        if (!is_array($database_access[$dbid])) {
            $error->set($FUNCNAME, 1, 'DBID "'.$dbid.'" unknown.');
            return;
        }
        
        $db		= $database_access[$dbid]["db"];
        $LogName= $database_access[$dbid]["LogName"];
        $passwd	= $database_access[$dbid]["passwd"];
        $_dbtype= $database_access[$dbid]["_dbtype"];
        $dbalias = $database_access[$dbid]["alias"];		// new: web_service_name
        
        
        
        // Try the database connection
        // Open the database connection
        
        $sqlo   = logon_to_db($LogName, $passwd, $db, $_dbtype, '', false);
        if(!$sqlo) {
            $error->set($FUNCNAME, 2, "INTERN-DB User and/or  password incorect");
            return;
        }
        
        // Test the cct_user and pass
        
        
        $logopt = NULL;
        
        $loginLib     = new fLoginC();
        $loginfo      = $loginLib->loginCheck($sqlo, $cctpwd, $cctuser, "", $logopt);
        $login_logarr = $loginLib->getLoginLog();
        $login_txt = NULL;
        if (is_array($login_logarr)) $login_txt = implode( "; ", $login_logarr);
        
        if ($loginfo["logok"] < 1) {
            $errLast = $error->getLast();
            $error->set( $FUNCNAME, 3, $errLast->text.' ('.$errLast->id.')'); // put the error message
            return;
        }
        
        $initLib = new fConnect();
        $init_set_arr = array();
        $tempstr  	     = $_SERVER['SCRIPT_FILENAME'];
        $init_set_arr['FILENAME'] = str_replace("/api", "", $tempstr);
        $init_set_arr['PHP_SELF'] = str_replace("/api", "",$_SERVER['PHP_SELF']);
        
        $initLib->initvars($sqlo, $loginfo, $LogName, $passwd, $db, $_dbtype, $init_set_arr);
        
        
        
        $tmpsess = session_id() ;
        
        
        return array('sessionid'=>$tmpsess);
    }
    
   
    
   
}
