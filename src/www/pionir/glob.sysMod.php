<?php
/**
 * modify SYSTEM parameters
 * @package glob.sysMod.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param  $testid   (e.g. "object modification log")
		   $parx  :  params of test
					 e.g. "val"
		   $go    : 0,1
 * @version0 2008-04-01
 */
session_start(); 

require_once ("reqnormal.inc"); // includes all normal *.inc files
require_once ('f.msgboxes.inc'); 


class gSysModC {
	
function __construct( $testid, $parx, $go ) {
	$this->testid = $testid;
	$this->parx   = $parx;
	$this->go = $go;
	
	$this->actarr = array (
		"CCT_ACCESS_UPI" => array("inf"=>"ENABLE object modification log"), 
		"http_cache_clean"=> array("inf"=>"clean Temp files")
						  );
}

function info() {
	echo "Action: <b>".$this->actarr[$this->testid]["inf"]."</b> [key: ".$this->testid."] <br>\n";
	if (sizeof($this->parx) ) {
		echo "Parameters: <b>";
		echo glob_array2String($this->parx);
		echo "</b><br>";
	}
	echo "<br>";
}

function f_http_cache_clean() {
    
    
    $cnt   = 0;
    $pathx = $_SESSION['globals']['http_cache_path'];    // !!! relative to rootsubs
    echo "Directory : '$pathx' <br>";
    
    if ( $pathx != "" ) { 
        
        if ($handle = opendir($pathx)) {

            while (false !== ($file = readdir($handle))) {
                if ($file != "." && $file != "..") { 
                    if ($action=="delete") unlink ($pathx . "/" .$file);
                    $cnt++;
                }
            }

            closedir($handle); 
        }
    }   
    echo "<B>$cnt</B> files deleted.<br>\n";
    return ($cnt);
} 

function f_CCT_ACCESS_UPI( &$sqlo ) {
	global $error;
	$FUNCNAME= "f_CCT_ACCESS_UPI";
   
	$trigAct = "";
	$inval   = $this->parx["val"];
	if ($inval=="ON")   $trigAct="ENABLE";
	if ($inval=="OFF")  $trigAct="DISABLE";
	if ($inval=="AUDIT")$trigAct="AUDIT";
	
	if ($trigAct=="")  {
		$error->set( $FUNCNAME, 1, "input-parameter not valid" );
		return;
	}
	
	/*
	 * TODO: check DB-Version for Trigger:
	 * 
	 * TRIGGER: CCT_ACCESS_UPI  -> 1.0.4.1
	 * TRIGGER: CCT_ACCESS_UPI2 -> 1.0.4.9
	 */
	
	//enable trigger for audit-objects only
	if ($trigAct=="AUDIT")  {
		//disable all-log trigger
		$sqls = "ALTER TABLE CCT_ACCESS disable trigger CCT_ACCESS_UPI"; //{disable|enable}
		$sqlo->Query($sqls);
		
		//enable trigger for audit objects only 
		$sqls = "ALTER TABLE CCT_ACCESS ENABLE trigger CCT_ACCESS_UPI2 "; //{disable|enable}
		$sqlo->Query($sqls);
		return;
	}
	
	else {
		//disable trigger for audit objects only 
		$sqls = "ALTER TABLE CCT_ACCESS disable trigger CCT_ACCESS_UPI2"; //{disable|enable}
		$sqlo->Query($sqls);
		
		//set the all-log trigger
		$sqls = "ALTER TABLE CCT_ACCESS ".$trigAct." trigger CCT_ACCESS_UPI "; //{disable|enable}
		$sqlo->Query($sqls);
		return;
	}
}

function doit ( &$sqlo ) {

	
    switch ( $this->testid ) {
        case "http_cache_clean":
            $this->f_http_cache_clean();
            break;
		case "CCT_ACCESS_UPI":
            $this->f_CCT_ACCESS_UPI( $sqlo );
            break;
			
        default: htmlErrorBox( "Error", "Action not found");
            break;
    }

} 

}

// Open the database connection
global $error, $varcol;

$error = & ErrorHandler::get();
$sqlo  = logon2( ); // URL-link for the first db-login; e.g. for object ($_SERVER['PHP_SELF']."?id=".$id)
if ($error->printLast()) htmlFoot();

$title = "Modify SYSTEM parameters";
$infoarr=array();
$infoarr["help_url"] = "o.GLOBALS.html";

$infoarr["title"] = $title;
$infoarr["form_type"]= "tool";
$infoarr["icon"]     = "images/f.glob.syscheck.gif";
$infoarr["locrow"]   =  array( array("glob.syscheck.php", "System check") );

$pagelib = new gHtmlHead();
$pagelib->startPage($sqlo, $infoarr);
echo "<ul>\n";

if ($_REQUEST["testid"]=="") htmlFoot("Error", "Give a test-ID"); 

if ( !glob_isAdmin() ) {
     htmlErrorBox( "Error",  "Only root can execute this!",
     "For security reason it is not allowed for common users" );
     htmlFoot();
}

$mainlib = new gSysModC( $_REQUEST["testid"],  $_REQUEST["parx"], $_REQUEST["go"] );
$mainlib->info();
$mainlib->doit ( $sqlo );
if ( !$error->printAll() ) {
	cMsgbox::showBox("ok", "Action successful.");
}
   
htmlFoot("<hr>");