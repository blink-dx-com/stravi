<?php
/**
 * sub-methods for glob.syscheck.php
 * @package glob.syscheck.inc
 * @swreq UREQ:0001115: g > Application Systemcheck 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/www/pionir/subs/glob.syscheck.inc 59 2018-11-21 09:04:09Z $
 */

require_once ("f.help.inc");
require_once ("f.rider.inc");
require_once ("subs/glob.sysmess.inc");
require_once ("f.objview.inc");
require_once ('down_up_load.inc');


// ABSTRACT CLASS
class sysCheckByName_ABS {
    
    protected $blacklist=array(); // array of bad tests
    protected  $sysCheckObj;
    
    function __construct(sysCheckC &$sysCheckObj, $debug=0) {
        $this->blacklist=array();
        $this->sysCheckObj = &$sysCheckObj;
        $this->time_cache=array();
        $this->debug=$debug;
        
        $config_file= dirname(__FILE__).'/../../../config/settings.syscheck.json';
        if (file_exists($config_file)) {
            
            $config_arr = json_decode(file_get_contents($config_file), TRUE);
            if (!empty($config_arr['blacklist'])) {
                $this->blacklist = $config_arr['blacklist'];
            }
        }
    }
    
    function _check( &$sql, $varname ) {}
    
    function checkByName( &$sql, $varname ) {
        
        if (in_array($varname,$this->blacklist)) {
            return;
        }
        
        $this->time_cache[$varname]=array('start'=>microtime());
        
        try {
            $this->_check( $sql, $varname );
        } catch (Exception $e) {
            $mess =  $e->getMessage();
            $trace_string = $e->getTraceAsString();
            $trace_string = str_replace("\n", "<br>", $trace_string);
            $this->sysCheckObj->messageout( $varname, 'TEST?', '?', '', -1, 'INTERNAL ERROR: '.$mess );
            
        }
        
        $this->time_cache[$varname]['end']=microtime();
    }
    
    
    function get_timearr() {
        return $this->time_cache;
    }
}

/**
 * common system check sub-functions
 * @author steffen
 *
 */
class sysCheckC {

var $errcnt;
var $infolevel;
var $all_globals_DB;

function __construct( sysMessC &$sysMessObj, &$all_globals_DB ) {
	$this->errcnt = 0;
	$this->sysMessObj = &$sysMessObj;
	$this->all_globals_DB = $all_globals_DB;
	
	$this->sysModURL = "glob.sysMod.php";
	$this->objLinkLib = new fObjViewC();
	
	$this->infolevel =  $this->sysMessObj->debug;
	
}

static function getFromGlob( &$sql, $keyx ) {
    $sqls   = "select VALUE from GLOBALS where NAME='".$keyx."'";
    $sql->query($sqls);
    $sql->ReadRow();
    $valx = $sql->RowData[0];
    return ($valx);
}

function showHead($debug) {
	
	
	$addcell = "<a href=\"view.tmpl.php?t=GLOBALS\"><img src=\"images/but.list2.gif\" border=0> Go to database table</a>\n";
	$ridopt["addcell"] = $addcell;
	
	$riderobj = new fRiderC();
	$xmodes=array( 
		0=>array("_A_", $_SERVER['PHP_SELF']."?debug=0", "Compact, show only errors"),
		1=>array("_B_", $_SERVER['PHP_SELF']."?debug=1", "Standard view"),
		2=>array("_C_", $_SERVER['PHP_SELF']."?debug=2", "Advanced tests"),
		3=>array("_D_", $_SERVER['PHP_SELF']."?debug=3", "Optional System tests"),
		);
	
	$riderobj->riderShow( $xmodes, $debug, "Levels", $ridopt);
	$riderobj->showNotes( $xmodes[$debug][2] );

}

/**
 * 
 * @param string $dirx
 * @param array $opt
 *   'subdirs':0,1
 *   'write' : 0,1
 * @return string
 */
function _checkDir( $dirx, $opt=array() ) {
    
	$mess_txt="";
	if(!file_exists($dirx))  {
		return ("Does not exist");
	}
	
	if ($opt['subdirs']) {
    	if ($handle = opendir($dirx)) {
    	    $ok=0;
    	    while (false !== ($entry = readdir($handle))) {
    	        if ($entry != "." && $entry != "..") { 
    	            $ok=1;
    	            break;
    	        }
    	    } 
    	    
    	    if (!$ok) {
    	        $mess_txt="Has no Sub-Directories";
    	    }
    	    closedir($handle);
    	    
    	    if ($mess_txt!='') return ($mess_txt);
    	}
	}
	
	if ($opt['write']) {
	    if(!is_writeable($dirx)) {
	        $mess_txt="Is not writeable";
	        return ($mess_txt);
	    }
	}
	
	return ($mess_txt);
}



/**
 * @return string error text
 */
private function _handleError() {
	
}


function errorInc($prio=NULL) {
	$this->errcnt++;
}

function tableClose() {
	$this->sysMessObj->tableClose();
}



// TBD: check this
function messageout( $keyNice, $mess_txt, $keyx, $valx, $error_flag, $notes=NULL, $optx=NULL ) {
	$this->sysMessObj->messageout( $keyx, $keyNice, $mess_txt, $keyx, $valx, $error_flag, $notes, $optx );
}

/**
 * just test, if a variable-value exists ...
 * @param object $sql
 * @param string $keyx  globals variable-name
 * @param string $keyNice  nice name of variable
 * @param string $notes
 */
function simpleTest(	&$sql, 	string $keyx,    $keyNice, $notes=NULL) {
	$error_flag = "";
	$mess_txt   = "OK";
	$valx = $this->getFromGlob($sql, $keyx);
	if ($valx=="") {
		$mess_txt  ="not set";
		$error_flag="-2"; 
	}
	$this->sysMessObj->messageout( $keyx, $keyNice, $mess_txt, $keyx, $valx, $error_flag, $notes );
}

function _createFile($filename) {
	if (!($_fileHDL = @fopen($filename, "a"))) { 
		return -1;
	} 
	fclose($_fileHDL);
	return (0);
}

private function check_writeable_dir($dir) {
	
	$error_flag= "";
	$mess_txt  = "OK";
	$notes     = "";
	
	do {
		if(!file_exists($dir)) {
			$error_flag="1";
			$notes  = "Does not exist.";
			$mess_txt  = "ERROR";
			break; 
		} 
		
		if(!is_writeable($dir)) {
			$error_flag="1";
			$notes  = "Is not writeable.";
			$mess_txt  = "ERROR";
			// continue test ... No break;
		}
		
		
		$filename = 'SystemCheck.dat';
		$testfile_full = $dir.'/'.$filename;
		$notes='TestFile: '.$testfile_full;
		
		if (file_exists($testfile_full)) {
			unlink($testfile_full);
			
			if (file_exists($testfile_full)) {
				$error_flag="1";
				$notes  = "Is not writeable (cannot remove old Testfile).";
				$mess_txt  = "ERROR";
				break;
			}
		}
		
		$fp = fopen($testfile_full, 'w');
		if ($fp==NULL) {
			$error_flag=1;
			$mess_txt  = "ERROR";
			$notes="Dir not writeable (test file).";
			break;
		} 
		
		// write some test data
		$content='Temporary File for SystemCheck.';
		$retVal = fputs( $fp, $content );
		fclose( $fp );
		
		if (!file_exists($testfile_full)) {
			$error_flag = 1;
			$mess_txt  = "ERROR";
			$notes     = "Test file not found.";
			break;
		}
		
		/*
		if (file_exists($testfile_full)) {
			if (!unlink($testfile_full)) {
				$error_flag = 1;
				$mess_txt  = "ERROR";
				$notes     ="Could not remove Test file.";
			}
		}
		*/ 
	} while (0);
	
	return array('error_flag'=>$error_flag, 'mess_txt'=>$mess_txt,  'notes'=>$notes);
}

function fileTest(
	&$sql, 
	$keyx,     // globals variable-name
	$keyNice,  // nice name of variable
	$expected, // 0,1 -- is expected ???
	$notes = NULL,
	$opt   = NULL // "autoCreate" => 0, 1: create if not exists
	) {
	// FUNCTION: if VALUE NOT exists: "not set"
	//		     if VALUE exists:
	//				- test, if file exists
	//				- test, if writeable
	// 			- updates $this->errcnt
	
	
	$error_flag = "";
	$mess_txt   = "OK";
	$valx 		= $_SESSION['globals'][$keyx];
	
	do {
		if ($valx == "") {
			$mess_txt  = "not set";
			$error_flag= "-2"; 
			if ($expected) {
				$error_flag= "1"; 
			}
			break;
		} 
		if( !file_exists($valx) ) {
			$error_flag= "1";
			$mess_txt  = "Not exists!";
			
			if ( $opt["autoCreate"]>0 ) {
				$retval = $this->_createFile($valx);
				if ($retval>=0) {
					$error_flag = "";
					$mess_txt   = "OK";
					break;
				} 
				$notes = "Not creatable! ".$notes;
			}
			break;
		} 
		if( !is_writeable($valx) ) {
			$error_flag = 1;
			$mess_txt   = "Not writeable!";
			break;
		}
	} while (0);
	
	if ($error_flag==1) {
		$this->errorInc();
	}
	
	$this->sysMessObj->messageout( $keyx, $keyNice, $mess_txt, $keyx, $valx, $error_flag, $notes );
}

function test_security_level() {
    
    $error_flag= -2;
    $texter    = "DataReadSecurity";
    $globKey   = "security_level";
    $mess_txt  = "not set";
    $tmpnotes  = "special READ access policy: NOT SUPPORTED anymore.";
    $valx      = $this->all_globals_DB[$globKey];
    if( $valx!="" ) {
        $error_flag = "";
        $mess_txt   = $valx;
        $error_flag= "0";
        if ($mess_txt!="select_on") {
            $error_flag = 1;
            $mess_txt	= "ERROR";
            $tmpnotes   = "keyword unknown!";
        }
        
    }
    $this->sysMessObj->messageout( $globKey, $texter, $mess_txt, $globKey, $valx, $error_flag, $tmpnotes);
}

function test_img_convert($sql) {
    //Test for image convert utility
    $error_flag="";
    $texter="Image convert program";
    $mess_txt="OK";
    if(!is_executable($_SESSION['globals']['img_convert'])) {
        $error_flag="1";
        $mess_txt="Does not exist";
        $this->sysCheckObj->errorInc();
    } else {
        
        $cnv_p=popen($_SESSION['globals']['img_convert'],"r");
        $cnv_res=fgets($cnv_p,1024);
        pclose($cnv_p);
        
        //Test for image convert utility
        $error_flag="";
        $texter="Image convert program";
        $mess_txt="OK";
        if(!strpos($cnv_res,"ImageMagick")) {
            $error_flag="1";
            $mess_txt="is not ImageMagick";
            $this->sysCheckObj->errorInc();
        }
    }
    
    $this->sysMessObj->messageout( "img_convert", $texter, $mess_txt, "img_convert", $_SESSION['globals']['img_convert'], $error_flag );
    
}

function test_img_identify($sql) {
    //Test for image identify utility
    $error_flag="";
    $texter="Image identify program";
    $mess_txt="OK";
    if(!is_executable($_SESSION['globals']['img_identify'])) {
        $error_flag="1";
        $mess_txt="Does not exist";
        $this->sysCheckObj->errorInc();
    }
    $this->sysMessObj->messageout( "img_identify",$texter, $mess_txt, "img_identify", $_SESSION['globals']['img_identify'], $error_flag );
    
}

function test_lab_path($sql) {
    $error_flag="";
    $texter    ="lab_path";
    $mess_txt  ="OK";
    if($_SESSION['globals'][$texter]=="") {
        $mess_txt   = "Not set";
        $error_flag = -2;
    } else {
        if (!file_exists("../".$_SESSION['globals'][$texter])) { // check lab-path
            $mess_txt   = "NOT EXISTS";
            $error_flag = 1;
        }
    }
    $this->sysMessObj->messageout( $texter, $texter, $mess_txt, $texter, $_SESSION['globals'][$texter], $error_flag, "Path of lab specific code" );
    
}

function test_magasin_serial($sql) {
    $error_flag="";
    $texter="magasin_serial";
    $mess_txt="OK";
    if($_SESSION['globals']['magasin_serial']==0) {
        $mess_txt   = "DEMO";
        $error_flag = -1;
    }
    if ( $_SESSION['globals']['magasin_serial']<0 ) {
        $error_flag="1";
        $mess_txt="NOT VALID";
        $this->sysCheckObj->errorInc();
    }
    $this->sysMessObj->messageout( $texter, $texter, $mess_txt, "magasin_serial", $_SESSION['globals']['magasin_serial'], $error_flag );
}

/**
 * 
 * @param object $sql
 * @param string $keyx // globals variable-name
 * @param string $keyNice  // nice name of variable
 * @param int $expected // 0,1 -- is expected ???
 * @param string $notes
 */
function testExecutable(&$sql, string $keyx,  string $keyNice, int $expected, $notes = NULL) {

	$valx = $this->getFromGlob($sql, $keyx);
	$error_flag = 0;
	$mess_txt   = "OK";
	
	do {
		if ($valx=="") {
			$error_flag = -2;
			$mess_txt = "not set";
			if ($expected) {
				$error_flag = 1; 
			}
			break;
		} 
		
		if( !file_exists($valx) ) {
			$error_flag= 1;
			$mess_txt  = "Not exists!";
			break;
		}
		
		if ( !is_executable($valx) ) {
			$error_flag= 1;
			$mess_txt  = "Not executable!";
			$this->errorInc();
			break;
		} 
		
	} while (0);
	
	if ($error_flag==1) {
		$this->errorInc();
	}
	$this->sysMessObj->messageout( $keyx, $keyNice, $mess_txt, $keyx, $valx, $error_flag, $notes );
}

function test_http_cache_path() {
	
	
	$debug = $this->sysMessObj->debug;
	$error_flag="";
	$texter  = "http_cache_path";
	$keyx    = $texter;
	$mess_txt= "OK";
	
	$che_opt=array('write'=>1);
	$tmptxt  = $this->_checkDir( $_SESSION['globals']['http_cache_path'], $che_opt );
	
	if($tmptxt!="") { /* it is a relative path !!! */
		$error_flag	= "1";
		$mess_txt	= $tmptxt;
		$this->errorInc();
	} else {
		if ($debug>1) {
			$tmpcnt = this_httpcache(NULL);
			$notestmp = "<B>$tmpcnt</B> files [<a href=\"".$this->sysModURL."?testid=http_cache_clean\">clean up</a>]";
		}
	}
	$this->sysMessObj->messageout( $keyx, $texter, $mess_txt, $keyx, $_SESSION['globals']['http_cache_path'], $error_flag, $notestmp );

}

function test_app_advmod($sql) {
    
    
    $error_flag="";
    $keyx    = 'app.advmod';
    $key_nice  = 'Advanced Object-Modification-Log';
    $mess_txt= "OK";
    $tmptxt  = $_SESSION['globals'][$keyx];
    $notestmp='set to 1: log all changes of an object, if Audit-status was activated. <br> 5: log all changes of an object.';

    if($tmptxt=="") {
        $error_flag	= -2;
        $mess_txt	= 'not set';
    }
    $this->sysMessObj->messageout( $keyx, $key_nice, $mess_txt, $keyx, $tmptxt, $error_flag, $notestmp );
}

function test_app_modulLog() {
	
	$fileshort = 'app.modulLog.dat';
	$fileName  = $_SESSION["globals"]["app.log.dir"].'/'.$fileshort;
	
	$error_flag="";
	$texter  = $fileshort;
	$keyx    = $texter;
	$mess_txt= "OK";
	
	$che_opt=array('write'=>1);
	$tmptxt  = $this->_checkDir( $fileName, $che_opt );
	
	if($tmptxt!="") { 
		$error_flag	= "1";
		$mess_txt	= $tmptxt;
		$this->errorInc();
	}
	$this->sysMessObj->messageout( $keyx, $texter, $mess_txt, $keyx, $fileName, $error_flag, $notestmp );
	
}


private function test_DbLoginMeth_sub(&$sql) {
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	$keyx    = "LDAP.queries";
	$keyNice = "LDAP: Anonymous login";
	$notes   = "get config from DbLogin.LDAP and LDAP.queries";
	$mess_txt  ="not set";
	$error_flag="-2";
	$valx    = NULL;
	
	require_once('f.ldap.subs.inc');
	
	$ldapLib = new f_LDAP_subs();
	if (!$ldapLib->queries_configured($sql)) {
		$mess_txt  ="not set";
		$error_flag="-2";
		$valx      = 'var "LDAP.queries" not set';
		$this->sysMessObj->messageout( $keyx, $keyNice, $mess_txt, $keyx, $valx, $error_flag, $notes );
		return;
	}
	
	$connectDict = $ldapLib->getConnectDict($sql);
	
	if ($error->Got(READONLY))  {
		$errLast   = $error->getLast();
		$error_txt = $errLast->text;
		$error_id  = $errLast->id;
		$error->reset();
	
		$mess_txt  ="ERROR";
		$error_flag=1;
		$notes   .= '; Error:'.$error_txt;
		$this->sysMessObj->messageout( $keyx, $keyNice, $mess_txt, $keyx, $valx, $error_flag, $notes );
	
		return;
	
	}
	
	if ($connectDict==NULL and !$error->Got(READONLY) ) {
		$mess_txt  ="not set";
		$error_flag="-2";
		$valx      = 'No LDAP config!';
		$this->sysMessObj->messageout( $keyx, $keyNice, $mess_txt, $keyx, $valx, $error_flag, $notes );
		return;
	}
	
	// print_r($connectDict);
	
	if ($connectDict['user']==NULL or $connectDict['pw']==NULL) {
		$mess_txt  = "not set";
		$error_flag= "-2";
		$valx      = 'USER,PW not set in config.';
		$this->sysMessObj->messageout( $keyx, $keyNice, $mess_txt, $keyx, $valx, $error_flag, $notes );
		return;
	}
	
	
	
	$ldapLib->connect($connectDict);
	if ($error->Got(READONLY))  {
		$errLast   = $error->getLast();
		$error_txt = $errLast->text;
		$error_id  = $errLast->id;
		$error->reset();
	
		$mess_txt  ="ERROR";
		$error_flag=1;
		$notes   .= 'Error:'.$error_txt;
		$this->sysMessObj->messageout( $keyx, $keyNice, $mess_txt, $keyx, $valx, $error_flag, $notes );
	
		return;
	
	}
	
	$valx   = 'connected to: '.$connectDict['ldapserver'];
	$mess_txt  ="OK";
	$error_flag=0;
	$this->sysMessObj->messageout( $keyx, $keyNice, $mess_txt, $keyx, $valx, $error_flag, $notes );
	
}

/**
 * test, if a variable-value exists ...
 * test connection ....
 * @param object $sql
 */
function test_DbLoginMeth(&$sql) {
	global $error;
	
	$optx = NULL;
	$keyx = "DbLogin.method";
	$keyNice = $keyx;
	$error_flag = "";
	$mess_txt   = "OK";
	$valx = $this->getFromGlob($sql, $keyx);
	$notes = "[NORM] or LDAP";
	if ($valx=="") {
		$mess_txt  ="not set";
		$error_flag="-2"; 
	}
	if ($valx=="LDAP") {
		$optx["infIcon"] = "INFO"; 
	}
	
	$this->sysMessObj->messageout( $keyx, $keyNice, $mess_txt, $keyx, $valx, $error_flag, $notes, $optx );
	
	$keyx = "DbLogin.LDAP";
	$keyNice = $keyx;
	$notes = "LDAP connection-string e.g.: server:dc1.clondiag.jena;userbase:cn=users,dc=clondiag,dc=jena";
	$mess_txt  ="not set";
	$error_flag="-2";
	$valx = $this->getFromGlob($sql, $keyx);
	if ($valx!="") {
		$error_flag = "";
		$mess_txt   = "OK";
	}
	$this->sysMessObj->messageout( $keyx, $keyNice, $mess_txt, $keyx, $valx, $error_flag, $notes );
	
	// check anonymous login
	$this->test_DbLoginMeth_sub($sql);
	
	$error->reset(); // reset former errors
	
}

/**
 * test, if a variable-value exists ...
 * test connection ....
 * @param object $sql
 */
function test_DbLoginDeny(&$sql) {
    
    $debug = $this->sysMessObj->debug;
    $error_flag= "-2";
    $texter    = "LoginDeny";
    $mess_txt  = "not set";
    $tmpnotes  = "";
    if ($debug){
        $tmpnotes = "set a text to deny login";
    }
    if( $this->all_globals_DB["DbLoginDeny"]!="" ) {
        $error_flag = "-1";
        $mess_txt   = "No login possible";
    }
    $this->sysMessObj->messageout( $texter, $texter, $mess_txt, 'DbLoginDeny', $this->all_globals_DB["DbLoginDeny"], $error_flag, $tmpnotes);
}

function test_DbLoginIPOnly($sql) {
    $error_flag= "-2";
    $texter    = "LoginIP_only";
    $globKey   = "DbLoginIPOnly";
    $mess_txt  = "not set";
    $tmpnotes = "give comma separated list of IP-numbers";
    if( $this->all_globals_DB[$globKey]!="" ) {
        $error_flag = "";
        $mess_txt   = "IP list";   
    }
    $this->sysMessObj->messageout( $globKey, $texter, $mess_txt, $globKey, $this->all_globals_DB[$globKey], $error_flag, $tmpnotes);
}

function test_DbUserMessage(&$sqlo) {
	$optx = NULL;
	$keyx    = "DbUserMessage";
	$keyNice = "UserMessage";
	$error_flag = "";
	$mess_txt   = "OK";
	$valx = $this->getFromGlob($sqlo, $keyx);
	$notes = "Message to users";
	if ($valx=="") {
		$mess_txt  ="not set";
		$error_flag="-2"; 
	}
	
	if ($valx!=NULL) {
		$optx["infIcon"] = "INFO"; 
	}
	$this->sysMessObj->messageout( $keyx, $keyNice, $mess_txt, $keyx, $valx, $error_flag, $notes, $optx );
	
}

function test_security_write($sql) {
    $error_flag= "-2";
    $texter    = "ObjectHiProtectPolicy";
    $globKey   = "security_write";
    $mess_txt  = "not set";
    $tmpnotes  = "OHIPPO - special modification access policy (0: owner of object has full access to it, 1: use group rights also for owned objects)".
        " [<a href=\"rootsubs/o.user_group.secwri.php\">DB update for this flag</a>]";
    $valx      = $this->all_globals_DB[$globKey];
    if( $this->all_globals_DB[$globKey]!="" ) {
        $error_flag = "";
        $mess_txt   = $this->all_globals_DB[$globKey];
        $error_flag= "0";
        if ($mess_txt!="1" AND $mess_txt!="0") {
            $error_flag = 1;
            $mess_txt	= "ERROR";
            $tmpnotes   = "keyword unknown!";
        }
        
    }
    $this->sysMessObj->messageout( $globKey, $texter, $mess_txt, $globKey, $valx, $error_flag, $tmpnotes);
}

/**
 * - load edit.tmpl.php forwards
 * - check if file exists
 */ 
function test_editScript(&$sql) {
	
	
	$sysMessObj = &$this->sysMessObj;
	$errcnt   = 0;
	$foundCnt = 0;
	$globals_tmp = $_SESSION['globals'];
	$test_key = "xEditScrForward";
	
	reset($globals_tmp);
	foreach( $globals_tmp as $tmpname=>$tmpval) {
		
		if (strstr( $tmpname, ".editScript" )!="" ) {
			$error_flag = '';
			$texter     = 'edit form forward '. $tmpname;
			$tmpvarname = $tmpname;
			$mess_txt   = 'OK';
			$globval    = $tmpval;
			
			if (!file_exists($tmpval)) {
				$mess_txt   = 'NOT EXISTS';
				$error_flag = 1;
			}
			if ($error_flag)  $errcnt++;
			
			$sysMessObj->messageout( $test_key, $texter, $mess_txt, $tmpvarname, $globval, $error_flag );
			$foundCnt++;
		}
	}
	reset($globals_tmp);
	
	if (!$foundCnt) {
		$sysMessObj->messageout( $test_key, 'edit form forward ', "not set", "o.[TABNAME].editScript", "", -2 );
	}
		
	if ($foundCnt>0) {
		$this->errorInc();
	}
}



function test_install_wiid ( &$sql ) {  
// test WIID and contact
        
	$test_key   = "xWIIDtest";
    $test_name  = 'installation WIID entry';
    $error_flag = '';
    $tmp_notes  = "system entry in the 
                    <a href=\"view.tmpl.php?t=H_WIID\" target=_help>WIID table</a>";
    $mess_txt   = 'YES';
    $temp_val   = "";
    
    $sql->query( "select VALUE from GLOBALS where NAME='magasin_serial'" );
    $sql->ReadRow();
    $inst_id = $sql->RowData[0];
    $contact_id = 0;
    
    if ( $inst_id > 0 ) {
        $sql->query( "select WIID, CONTACT_ID from H_WIID where NAME='http://www.clondiag.com/magasin/?db=".$inst_id."'" );
        $sql->ReadRow();
        $wiid = $sql->RowData[0];
        $contact_id = $sql->RowData[1];
        if ( $wiid ) {
            $temp_val = "<a href=\"edit.tmpl.php?tablename=H_WIID&id=$wiid\" target=_help>$wiid</a>";
        } else {
            $error_flag = 1;
            $mess_txt   = 'NO';
            $temp_val   = "";
            $tmp_notes  =  $tmp_notes . ", need a WIID entry";
        }
    } else {
         $error_flag = -1;
         $mess_txt   = 'NOT NEEDED';
         $temp_val   = "";
         $tmp_notes  =  $tmp_notes . ", not need =&gt; because no magasin_serial";
    } 
    
	$optmx=array("noglob"=>1);
	$this->sysMessObj->messageout( $test_key, $test_name,  $mess_txt, "", $temp_val, $error_flag, $tmp_notes, $optmx);
    
	$test_key   = "xInstallContact";
    $test_name  = 'installation contact';
    $error_flag = '';
    $tmp_notes  = "system entry in the 
                    <a href=\"view.tmpl.php?t=H_WIID\" target=_help>contact in the WIID</a>";
    $mess_txt   = 'YES';
    $temp_val   = "";
    
    if ( $inst_id > 0 ) { // only if a valid installation exists
        if ($contact_id) {
             $sql->query( "select NAME from CONTACT where CONTACT_ID=". $contact_id );
             $sql->ReadRow();
             $contact_name = $sql->RowData[0];
             $temp_val   = "<a href=\"edit.tmpl.php?tablename=CONTACT&id=$contact_id\" target=_help>$contact_name</a>";
        } else {
            $error_flag = 1;
            $mess_txt   = 'NO';
            $temp_val   = "";
            $tmp_notes  = $tmp_notes . ", need a CONTACT in the WIID entry";
        } 
    } else {             
         $error_flag = -1;
         $mess_txt   = 'NOT NEEDED';
         $temp_val   = "";
         $tmp_notes  =  $tmp_notes . ", not need =&gt; because no magasin_serial";
    }
    $optmx=array("noglob"=>1);
	$this->sysMessObj->messageout( $test_key, $test_name,  $mess_txt, "", $temp_val, $error_flag, $tmp_notes, $optmx);
}   



function test_fModulLogC(&$sqlo) {
	require_once('f.modulLog.inc');
	
	$logobj = new fModulLogC();
	$valx = $logobj->logIsPossible();
	$tmp_filename = $logobj->modLogFile;
	
	$test_key  = NULL;
	
	$test_name = "MODUL_LOG";
	$notes   = "is MODUL_LOG active ? filename:".$tmp_filename;
	$error_flag = "";
	$mess_txt   = "not active";
	
	$datapath = $_SESSION['globals']["app.log.dir"];
	$modLogFile = $datapath . '/app.modulLog.dat';
	if (!file_exists($modLogFile)) {
		$notes .= 'extra-problem: no access:'.$modLogFile;
	}
	
	if ($valx==NULL) {
		$mess_txt  ="not set";
		$error_flag="-2";
	}
	if ($valx<0) {
		$mess_txt  ="not active";
		$error_flag="-2";
	}
	if ($valx>0) {
		$mess_txt   = "OK";
	}
	
	$optmx=array("noglob"=>1);
	$this->sysMessObj->messageout( $test_key, $test_name,  $mess_txt, "", $valx, $error_flag, $notes, $optmx);
	
}

function test_img_path(&$sql) {
    $debug = $this->sysMessObj->debug;
	$error_flag= "";
	$texter    = "Image path";
	$mess_txt  = "OK";
	$tmpnotes  = "";
	
	$answer = $this->check_writeable_dir( $_SESSION['globals']['img_path'] );
	$tmpnotes = $answer['notes'];
	
	
	if ( $this->infolevel>1 ){
		list($tmpcnt, $size) = this_readfiles( $_SESSION['globals']['img_path'], $debug );
		$tmpnotes = "<B>$tmpcnt</B> files";
		if ($debug>1) {
			$sizeout = ceil($size/1000000);
			$tmpnotes .= ", size: <B>$sizeout MB</B>";
		}
	}
	
	$this->sysMessObj->messageout( 'img_path', $texter, $answer['mess_txt'], 'img_path', $_SESSION['globals']['img_path'], $answer['error_flag'] , $tmpnotes);
		
}

private function _sys_net2srv_sub($extPath, $localPath) {
    $loopError=0;

    $che_opt=array('subdirs'=>1);
    $tmptxt  = $this->_checkDir( $localPath, $che_opt );
    if ( $tmptxt!='' ) {
        $loopError = 1;
    }
    
    // get data from directory ...
    
    $niceAnswer = $loopError > 0 ? '<span style="color:red">ERROR</span> '.$tmptxt : 'o.k.';
    $tmp_notes  = $extPath.' => '.$localPath. ' : '.$niceAnswer;
    
    return array($tmp_notes, $loopError);
}

/**
 * test $_SESSION['globals']["sys.net2srv"] 
 */
function test_path_sys_net2srv() {
	
	$tmp_globPaths = $_SESSION['globals']['sys.net2srv'];
	$mess_txt   = "not set";
	$error_flag = "-2";
	$test_key   = 'sys.net2srv';
	$tmp_notes  = NULL;
	$optmx      = array(  );
	$value		= NULL;

	if (is_array($tmp_globPaths)) {
		
		$error_flag   = 0;
		$mess_txt     = "OK";
		$errorOccured = 0;
		$hasError     = NULL;
		
		$tmpBR=NULL;
		foreach( $tmp_globPaths as $dummy=>$oneArr) {
		    
			$extPath  = key($oneArr);
			$localPath= current($oneArr);
			$value  .= $extPath.'=>'. $localPath.' ';
			
			$answer = $this->_sys_net2srv_sub($extPath, $localPath);
			
			$tmp_notes .= $tmpBR . $answer[0];
			$tmpBR="<br />";
			
			if ( $answer[1] ) {
			    $errorOccured = 1;
			}
		}
		
		
		if ( $errorOccured ) {
			$error_flag= 1;
			$mess_txt  = "ERROR";
			$this->errorInc();
		}
		
	}
	$this->sysMessObj->messageout( $test_key, 'Additional Data Paths',  $mess_txt, $test_key, $value, $error_flag, $tmp_notes, $optmx);
	
}

function test_polObjRelease(&$sqlo) {
	$mess_txt  = "not set";
	$error_flag= "-2";
	$test_key  = 'pol.objRelease';
	$optmx = NULL;
	$labFile   = '../'.$_SESSION["globals"]["lab_path"] . "/pol.objRelease.inc";
	$tmp_notes = 'lab-code: '.$labFile.' ';
	
	$value = $this->getFromGlob( $sqlo, $test_key);
	if ($value!=NULL) {
		$error_flag= 0;
		$mess_txt  = "OK";
		// check lab specific code-module
		if ( !file_exists($labFile) ) {
			$error_flag= 1;
			$mess_txt  = "ERROR";
			$tmp_notes .= ' <b>does not exist!</b>';
			$this->errorInc();
		}
	}
	$this->sysMessObj->messageout( $test_key, 'Lab Specific Release Policy',  $mess_txt, $test_key, $value, $error_flag, $tmp_notes, $optmx);
	
}

function test_o_PROJ_userhome($sqlo) {
	$error_flag= "-2";
	$test_key  = 'o.PROJ.userhome';
	$optmx = NULL;
	$tmp_notes = 'Home-project: ';
	$mess_txt  = "not set";
	
	$proj_id = $this->getFromGlob( $sqlo, $test_key);
	$value   = $proj_id;
	if (!$proj_id) {
		$error_flag= "1";
		$mess_txt='not set';
		$tmp_notes .=' not set.';
	} else {
	
		$proj_exists = gObject_exists ($sqlo, 'PROJ', $proj_id);
		$proj_name = obj_nice_name ( $sqlo, 'PROJ', $proj_id );
		if (!$proj_exists) {
			$error_flag= 1;
			$mess_txt='ERROR';
			$tmp_notes .=' not exists.';
		} else {
			$tmp_notes .=' '.$proj_name;
			$error_flag= 0;
			$mess_txt  = "OK";
		}
	}
	
	$this->sysMessObj->messageout( $test_key, 'Home Project',  $mess_txt, $test_key, $value, $error_flag, $tmp_notes, $optmx);
	
}

function test_Upload_zip(&$sqlo) {
	
	$test_key   = 'app.upload_zip';
	$tmp_globPaths = $_SESSION['globals'][$test_key];
	$mess_txt  = "not set";
	$error_flag= "-2";
	$tmp_notes  = NULL;
	$optmx      = array( "noglob"=>1 );
	$value		= NULL;

	if (is_array($tmp_globPaths)) {
		
		$error_flag   = 0;
		$mess_txt     = "OK";
		$errorOccured = 0;
		$hasError     = NULL;
		reset ($tmp_globPaths);
		$tmpBR   = NULL;
		$thisyear= date("Y",time());
		
		if (is_array($tmp_globPaths)) {
			
			$error_flag   = 0;
			$mess_txt     = "OK";
			/**
			 * analyse: ['NGAi'] => array('basedir'=>"Z:\\Forschung\\NGAI\\", 'year_basedir'=>'generated');
			 */
			foreach( $tmp_globPaths as $devicekey=>$oneArr) {
				
				$test_path_win =  $oneArr['basedir']  . $thisyear;
				if ( $oneArr['year_basedir']!=NULL ) $test_path_win .= "\\" . $oneArr['year_basedir'];
				$localPath = netfile2serverpath($test_path_win);
				
				$niceAnswer  = NULL;
				$err_message = NULL;
				$loopError =0;
				$answer = $this->check_writeable_dir( $localPath );
				if ($answer['error_flag']>0) {
					$loopError=1;
					$errorOccured=1;
					$err_message = ' '.$answer['mess_txt'].'; '. $answer['notes'];
				}
				
				$niceAnswer = $loopError > 0 ? '<span style="color:red">ERROR</span>' : 'o.k.';
				$tmp_notes .= $tmpBR . $extPath.' => '.$localPath. ' : '.$niceAnswer. $err_message;
				$tmpBR="<br />";
			}
			reset ($tmp_globPaths); 
		}
		
		if ( $errorOccured ) {
			$error_flag= 1;
			$mess_txt  = "ERROR";
			$this->errorInc();
		}
		
	}
	$this->sysMessObj->messageout( $test_key, 'Data paths for upload_zip',  $mess_txt, $test_key, $value, $error_flag, $tmp_notes, $optmx);
	
}

function logsAccLogEna  ( &$sqlo ) {
	/* check trigger CCT_ACCESS_UPI for table CCT_ACC_UP */
    
    $db_type=$_SESSION['sec']['_dbtype'];
    if ($db_type=='CDB_PG') $incdir='db_postgres';
    if ($db_type=='CDB_OCI8') $incdir='db_oracle';
    require_once $incdir.'/db_funcs.inc';
	
	// trigger exists ?
	// is enabled ???	
	$test_key   = "xObjModiLog";
	$tablename  = "CCT_ACC_UP";			
	$test_name  = "Object-Modification-Log" ;
	$sysmodurl  = $this->sysModURL."?testid=CCT_ACCESS_UPI&parx[val]=";
    $error_flag = -2;
    $tmp_notes  = "[<a href=\"".$sysmodurl."ON\">Switch all ON</a> | <a href=\"".$sysmodurl."AUDIT\">Switch audit only ON</a> | Switch OFF]";
    $mess_txt   = 'not set';
	$statusval  = NULL; 
	
	$statusval = DB_funcs::get_trigger_status($sqlo, 'CCT_ACCESS_UPI');
	
	if ( $statusval=='unknown' ) {
		$mess_txt   = 'not set';
		$tmp_notes  = "Trigger 'CCT_ACCESS_UPI' does not exist";
	}
    $temp_val   = $statusval;
    
	if ($statusval=="ENABLED") {
		$mess_txt   = 'OK';
		$error_flag = 0;
		$tmp_notes  = "[Switch all ON | <a href=\"".$sysmodurl."AUDIT\">Switch audit only ON</a> | <a href=\"".$sysmodurl."OFF\">Switch OFF</a>]";
	}
	
	if ($statusval=="DISABLED") {
	    $statusval2 = DB_funcs::get_trigger_status($sqlo, 'CCT_ACCESS_UPI2');
	    if ( $statusval2=='unknown' ) {
			$mess_txt   = 'not set';
			$tmp_notes  = "Trigger 'CCT_ACCESS_UPI2' does not exist";
		}
		$temp_val2   = $statusval2;
		if ($statusval2=="ENABLED") {
			$mess_txt   = 'OK';
			$error_flag = 0;
			$temp_val 	= "ENABLED(AUDIT)";
			$tmp_notes  = "  | [<a href=\"".$sysmodurl."ON\">Switch all ON</a> | Switch audit only ON | <a href=\"".$sysmodurl."OFF\">Switch OFF</a>]";
		}
	}
	
	$optmx=array("noglob"=>1);
	$this->sysMessObj->messageout( $test_key, $test_name,  $mess_txt, "", $temp_val, $error_flag, $tmp_notes, $optmx);

}

function test_tableTrigger(&$sqlo) {
	$errcnt   = 0;
	$foundCnt = 0;
	$test_key = "initTriggerID";
	$infoName = "initial trigger";
	$infoNotes = "option for glob.obj.crea_wiz.php";
	
	$sqls   = "select NAME, VALUE from GLOBALS where NAME like 'o.%.".$test_key."' order by NAME";
    $sqlo->query($sqls);
    $initTriggerArr=NULL;
    while ( $sqlo->ReadRow() ) {  
	
		$key   = $sqlo->RowData[0]; 
		$flag  = $sqlo->RowData[1]; 
		$initTriggerArr[$key]=$flag;
    }
    
	if (!sizeof($initTriggerArr)) {
		$this->sysMessObj->messageout( $test_key, $infoName, "not set", "o.[TABNAME].".$test_key, "", -2, $infoNotes );
		return;
	}
	
	foreach( $initTriggerArr as $key=>$flag) {	
		
		$tmparr  = explode(".",$key);
		$tmpname = $tmparr[1];
		$error_flag = '';
		$texter     = $infoName ." '". $tmpname."'";
		$mess_txt   = 'OK';
		$answer==NULL;
		$outopt=NULL;
		
		if ($flag>0) {
			if (!gObject_exists ($sqlo, 'MODULE', $flag) ) {
				$mess_txt   = 'ERROR';
				$error_flag = 1;
				$answer='MODULE ID:'.$flag.' not exists';
				$this->errorInc();
			} else {
				$modname = $this->objLinkLib->bo_display( $sqlo, 'MODULE', $flag );
				$answer  = $flag;
				$outopt['addValTxt']=' '.$modname; 
			}
		}
		
		if ($error_flag)  $errcnt++;
		$this->sysMessObj->messageout( $test_key, $texter, $mess_txt, $key, $answer, $error_flag, $infoNotes, $outopt );
		$foundCnt++;
		
	}
}
	
//Test for Work path
function test_work_path(&$sql) {
	
	$error_flag="";
	$texter="Work path";
	$mess_txt="OK";
	
	
	$answer = $this->check_writeable_dir( $_SESSION['globals']['work_path'] );
	
	if ($answer['error_flag']==1) $this->errorInc();
	$this->sysMessObj->messageout( "work_path", $texter, $answer['mess_txt'], "work_path", 
			$_SESSION['globals']['work_path'], $answer['error_flag'], $answer['notes'] );
}

//Test for Work path
function test_data_path(&$sql) {
    $debug = $this->sysMessObj->debug;
	$error_flag="";
	$texter="Data path";
	$mess_txt="OK";
    $pathname = $_SESSION['globals']['data_path'];

	$answer = $this->check_writeable_dir( $pathname );
	$tmpnotes =  $answer['notes'];
	
	
	if ( $this->infolevel>1 ){
		list($tmpcnt, $size) = this_readfiles( $pathname, $this->infolevel );
		$tmpnotes = "<B>$tmpcnt</B> files";
		if ($debug>1) {
			$sizeout = ceil($size/1000000);
			$tmpnotes .= ", size: <B>$sizeout MB</B>";
		}
	}

	if ($answer['error_flag']==1) $this->errorInc();
	$this->sysMessObj->messageout( "data_path", $texter, $answer['mess_txt'], "data_path",
			$pathname, $answer['error_flag'], $tmpnotes );
}

function test_xDocVersControl(&$sql) {
    $test_key = "xDocVersControl";
    $error_flag = "";
    $texter  ="document version control";
    $mess_txt="OK";
    $tmpdir = $_SESSION['globals']["data_path"] . "/vers";
    $valx   = $tmpdir;
    $tmpnotes = "document directory";
    
    if (!file_exists($tmpdir)) {
        $error_flag =-2;
        $mess_txt = "not set";
    } else {
        if( !is_writeable($tmpdir) ) {
            $error_flag =1;
            $mess_txt = "ERROR";
            $tmpnotes = "Is not writeable! ".$tmpnotes;
            $this->sysCheckObj->errorInc();
        }
    }
    $this->sysMessObj->messageout( $test_key, $texter, $mess_txt, "", $valx, $error_flag, $tmpnotes );
}

function test_xml_rpc_debug_dir() {
    $error_flag = '';
    $texter     = 'XML_RPC debug dir';
    $tmpvarname = 'xml_rpc.debug_dir';
    $mess_txt   = 'OK';
    $globval     = isset($_SESSION['globals'][$tmpvarname]) ? $_SESSION['globals'][$tmpvarname] : '';
    if ( $globval != "" ) { // only if set
        if( !is_dir($globval) ) {
            $mess_txt = 'NOT EXISTS';
            $error_flag = 1;
            $this->sysCheckObj->errorInc();
        }
        $this->sysMessObj->messageout( $texter, $texter, $mess_txt, $tmpvarname, $globval, $error_flag );
    }
}

function test_xRoleRights($sql) {
    $test_key   = "xRoleRights";
    $test_name  = 'Role rights';
    $error_flag = '';
    $tmp_notes  = "[<a href=\"view.tmpl.php?t=RIGHT_IN_ROLE\" target=_help>show rights</a>]
               [<a href=\"rootsubs/init/rights_insert.php\" target=_help>Create all rights</a>]";
    $mess_txt   = 'EXIST';
    $temp_val   = "";

    $gquery="select count(*) from RIGHT_IN_ROLE";
    $sql->query($gquery);
    $sql->ReadRow();
    $temp_val=$sql->RowData[0];
    if (!$temp_val) {
        $mess_txt   = 'NO';
        $error_flag = 1;
    }
    $optmx=array("noglob"=>1);
    $this->sysMessObj->messageout( $test_key, $test_name,  $mess_txt, "", $temp_val, $error_flag, $tmp_notes, $optmx);
}

function test_xUserHaveRoles($sql) {
    $test_key = "xUserHaveRoles";
    $test_name  = 'Users have roles';
    $error_flag = '';
    $tmp_notes  = "[<a href=\"view.tmpl.php?t=USER_ROLES\" target=_help>show users and roles</a>]";
    $mess_txt   = 'YES';
    $temp_val   = "";
    $gquery="select count(distinct(DB_USER_ID)) from USER_ROLES";
    $sql->query($gquery);
    $sql->ReadRow();
    $temp_val=$sql->RowData[0];
    
    if (!$temp_val) {
        $mess_txt   = 'NO';
        $error_flag = 1;
        $tmp_notes  = "";
    }
    $optmx=array("noglob"=>1);
    $this->sysMessObj->messageout( $test_key, $test_name,  $mess_txt, "", $temp_val, $error_flag, $tmp_notes, $optmx);
    
}
	

}




/**
 * check function by name
 * can be configured by settings.syscheck.json
 * @author steffen
 *
 */
class sysCheckByName extends sysCheckByName_ABS {
    
  
    
    function _check( &$sql, $varname ) {
        switch ($varname) {
            
            case 'app.modulLog.dat':
                $this->sysCheckObj->test_app_modulLog();
                break;
            case 'app.upload_zip':
                $this->sysCheckObj->test_Upload_zip($sql);
                break;
                
            case 'data_path':
                $this->sysCheckObj->test_data_path($sql);
                break;
            case 'DbLoginMeth':
                $this->sysCheckObj->test_DbLoginMeth($sql);
                break;
            case 'DbLoginDeny':
                $this->sysCheckObj->test_DbLoginDeny($sql);
                break;
            case 'DbLoginIPOnly':
                $this->sysCheckObj->test_DbLoginIPOnly($sql);
                break;
            case 'security_write':
                $this->sysCheckObj->test_security_write($sql);
                break;
            case 'Db.encoding':
                $optx    = NULL;
                $keyNice = $varname;
                $valx    = $this->sysCheckObj->getFromGlob($sql, $varname);
                $error_flag = "";
                $mess_txt   = "OK";
                $allowEnc   = array('UTF-8','ISO-8859-1');
                $notes      = 'encoding of the database, allowed values: '.implode(', ',$allowEnc);
                
                if ($valx==NULL) {
                    $mess_txt  ="not set";
                    $error_flag=1;
                } else {
                    
                    if (!in_array($valx,$allowEnc)) {
                        $mess_txt  ="bad value";
                        $error_flag=1;
                        
                    }
                }
                $this->sysCheckObj->messageout( $keyNice, $mess_txt, $varname, $valx, $error_flag, $notes, $optx );
                break;
            case 'DbUserMessage':
                $this->sysCheckObj->test_DbUserMessage($sql);
                break;
            case 'exe.graphviz':
                $this->sysCheckObj->testExecutable($sql, "exe.graphviz", "graphviz executable", 0, "needed for substance networks");
                break;
            case 'exe.R':
                $this->sysCheckObj->testExecutable($sql, "exe.R",   "R package executable", 0, "used for experiments &gt; Statistical analysis");
                break;
            case 'htmlFrameTop.homeBut':
                $this->sysCheckObj->simpleTest($sql, "htmlFrameTop.homeBut", "Home Button URL");
                break;
            case 'img_convert':
                $this->sysCheckObj->test_img_convert($sql);
                break;
            case 'img_identify':
                $this->sysCheckObj->test_img_identify($sql);
                break;
            case 'img_path':
                $this->sysCheckObj->test_img_path($sql);
                break;
            case 'lab_path':
                $this->sysCheckObj->test_lab_path($sql);
                break;
            case 'magasin_serial':
                $this->sysCheckObj->test_magasin_serial($sql);
                break;
                
            case 'NTDPAL_EXE':
                $this->sysCheckObj->testExecutable($sql, "NTDPAL_EXE",   "Alignment Tool", 0, "substance: DNA alignments");
                break;
            case "pol.objRelease":
                $this->sysCheckObj->test_polObjRelease($sql);
                break;
            case 'o.PROJ.userhome':
                $this->sysCheckObj->test_o_PROJ_userhome($sql);
                break;
            case 'fModulLogC':
                $this->sysCheckObj->test_fModulLogC($sql);
                break;
                
            case 'app.advmod':
                $this->sysCheckObj->test_app_advmod($sql);
                break;
                
            case 'security_level':
                $this->sysCheckObj->test_security_level($sql);
                break;
            case 'work_path':
                $this->sysCheckObj->test_work_path($sql);
                break;
            case 'xDocVersControl':
                $this->sysCheckObj->test_xDocVersControl($sql);
                break;
                
            case 'xRoleRights':
                $this->sysCheckObj->test_xRoleRights($sql);
                break;
                
            case 'xUserHaveRoles':
                $this->sysCheckObj->test_xUserHaveRoles($sql);
                break;
            case 'xml_rpc.debug_dir':
                $this->sysCheckObj->test_xml_rpc_debug_dir($sql);
                break;
                
                    
            default:
                $this->sysCheckObj->messageout( '???', 'TEST?', $varname, '', -1, 'Admin: Test is not defined!' );
                
        }
    }

   


}
    

/**
 * non DB-tests
 * @author steffen
 *
 */
class syscheck_extra {
	
	function __construct(&$sysChMeta) {
		$this->sysChMeta = &$sysChMeta;
	}
	
	function doall(&$sqlo) {
		$this->sysChMeta->checkByName($sqlo, "fModulLogC");
	}
}
 
function this_httpcache($action   // [""], "delete"
    ) {
    
    
    $cnt = 0;
    $pathx = $_SESSION['globals']['http_cache_path'];    // !!! relative to rootsubs
    
    if ( $_SESSION['globals']['http_cache_path']!= "" ) { 
        if ($handle = opendir($pathx)) {
            while (false !== ($file = readdir($handle))) {
                if ($file != "." && $file != "..") { 
                    $cnt++;
                }
            }
            closedir($handle); 
        }
    }   
    
    return ($cnt);
} 


function this_funcCreaOpt( &$sql, &$sysMessObj ) {
	// FUNCTION: load edit.tmpl.php forwards
	
	
	$errcnt   = 0;
	$foundCnt = 0;
	$test_key = "xfCreaOpt";
	$infoName = "object creation option for";
	$infoNotes = "option for glob.obj.crea_wiz.php";
	
	$sqls   = "select NAME, VALUE from GLOBALS where NAME like 'o.%.fCreaOpt'";
    $sql->query($sqls);
    while ( $sql->ReadRow() ) {  
	
		$key   = $sql->RowData[0]; 
		$flag  = $sql->RowData[1]; 
		
		$tmparr  = explode(".",$key);
		$tmpname = $tmparr[1];
		$error_flag = '';
		$texter     = $infoName ." '". $tmpname."'";
		$mess_txt   = 'OK';
		
		if (!$flag) {
			$mess_txt   = 'NOT EXISTS';
			$error_flag = 1;
		}
		if ($error_flag)  $errcnt++;
		$sysMessObj->messageout( $test_key, $texter, $mess_txt, $key, $flag, $error_flag, $infoNotes );
		$foundCnt++;
		
	}
	
	
	if (!$foundCnt) {
		$sysMessObj->messageout( $test_key, $infoName, "not set", "o.[TABNAME].fCreaOpt", "", -2, $infoNotes );
	}
		
	return ($errcnt);
}

function this_readfiles( $pathx, $debug ) {
    $cnt = 0;
    
    if ($handle = opendir($pathx)) {
        while (false !== ($file = readdir($handle))) {
            if ($file != "." && $file != "..") { 
                if ($debug>1 )$size = $size + filesize($pathx."/".$file);
                $cnt++;
            }
        }
        closedir($handle); 
    }
    return array($cnt, $size);
}


function this_getFromGlob( &$sql, $keyx ) { 
    $sqls   = "select VALUE from GLOBALS where NAME='".$keyx."'";
    $sql->query("$sqls");
    $sql->ReadRow();   
    $valx = $sql->RowData[0];    
    return ($valx);
}