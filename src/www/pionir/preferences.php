<?
/**
 * Set variables of user preferences $_SESSION['userGlob']
 * @package preferences.php
 * @swreq UREQ:GLOBAL
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param 
 * 	$variable or $var ( $_SESSION['userGlob'][$variable] )
	$val 	   ( new value)
    [$m]      ( the main serialized variable )
	[$act]  : special action: 
					"delfile": for $variable= g.sql_logging : delete log-file
	[$backurl] = urlencoded url to go back
 */

// extract($_REQUEST); 
session_start(); 


require_once ("reqnormal.inc");
require_once("javascript.inc");

class gPrefsSpecial {
	
function __construct() {
	$this->sqlo  = logon2( );
}

function action($act, $variable) {
	$found=0;
	echo "Action: $act<br>\n";
	if  ($act=='delfile' and $variable=='g.sql_logging') {
		$found=1;
		$this->do_delfile();
	}
	if (!$found) {
		htmlFoot('Error', 'Action: unknown.');
	}
}

function do_delfile() {
	$logx 		= &$this->sqlo->_sql_log;
	$fileinf    = $logx->_getLogFileName();
	$filename   = $fileinf['file'];
    $abs_dirname= $fileinf['path'];
	$filefull   = realpath($abs_dirname) . '/' . $filename;
	if ( file_exists($filefull) ) {
		echo 'delete sql-log-file ...<br>'."\n";
		unlink($filefull);
	} else {
		echo 'sql-log-file does not exist.<br>'."\n";
	}
}

}

// ------
		
$pagelib = new gHtmlHead();
$pagelib->PageHeadLight('set user preferences');

$variable = $_REQUEST['variable'];
$var      = $_REQUEST['var'];
$val      = $_REQUEST['val'];
$m 		  = $_REQUEST['m'];
$backurl  = $_REQUEST['backurl'];
$act      = $_REQUEST['act'];

if ($var!="") $variable=$var;

echo 'change _SESSION["userGlob"]: '.$variable.'='.htmlentities($val).' (mainvar: '.$m. ")<br>\n";

	
if ( $act!="" ) {
	$thelib = new gPrefsSpecial();
	$thelib->action($act, $variable);
	js__history_back2();
	exit;
}


if ($m!=""  ) {  
    $tmparr = unserialize($_SESSION['userGlob'][$m]);
    $tmparr[$variable] = $val;
    $_SESSION['userGlob'][$m] = serialize($tmparr);
} else {  
    $_SESSION['userGlob'][$variable]=$val; 
}

if ($backurl!="") {
	$backnow = urldecode($backurl);
	js__location_replace($backnow);
	return;
} 

js__history_back2();

$pagelib->htmlFoot();
