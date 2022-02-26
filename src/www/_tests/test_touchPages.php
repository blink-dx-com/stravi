<?php
/**
 * test: touch pages and analyse errors
 * $Header: trunk/src/www/_tests/www/test/test_touchPages.php 59 2018-11-21 09:04:09Z $
 * @package test_touchPages.php
 * @author  qbi
 * @version 1.0
 * @param string $go
 * @param array  parx["filex"] (filelist) 
 * @param int parx["cnt"] (0...)
 * @param int parx["close"] 0,1 : close window ?
 * @misc use $_REQUEST
 */

extract($_REQUEST); 
session_start(); 


require_once ("reqnormal.inc");

class Test_touchPages {
	
	private $closetime = 3000.0;

function __construct($go, &$parx, $scriptname, &$sqlo) {
	
	$this->sqlo = &$sqlo;
	$this->scriptname = $scriptname;
	$this->go = $go;
	$this->filex = $parx["filex"];
	$this->parx = $parx;
	if ( !$this->parx["cnt"] ) $this->parx["cnt"]=0;
	
	if ( $go==2 ) {
		$this->parx["close"] = $_SESSION['s_formState'][$this->scriptname]["close"];
	}
	
	
	$loginURL = $_SESSION['s_sessVars']["loginURL"];
	$loginURL_lev1 = dirname($loginURL);
	$this->baseurl = dirname($loginURL_lev1);

	// $this->baseurl ="http://tiberia2.clondiag.jena/~steffen/paertisanX";
	
}

function getFullUrl($fileshort) {
	return ($this->baseurl."/".$fileshort);
}

function manageForward() {
	
	
	$parx = $this->parx;
	
	$retval = 0;
		
	if (!sizeof( $_SESSION['s_formState'][$this->scriptname]["filearr"]) ) {
		return array(-1,"");
	}
	
	$this->fileArr = &$_SESSION['s_formState'][$this->scriptname]["filearr"];
	$lenarr = sizeof($this->fileArr);
	
	if ($parx["cnt"] < $lenarr) {
		$newcnt = $parx["cnt"] + 1;
		$meta   = "<META HTTP-EQUIV=\"Refresh\" CONTENT=\"2; URL=".$_SERVER['PHP_SELF'].
				"?parx[cnt]=".$newcnt."&go=2\">";
		$retval = 1;
	} else {
		$retval = 0;
	}
	
	return array($retval,$meta);
}

function formshow() {
	require_once ('func_form.inc');
	
	
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Give file list";
	$initarr["submittitle"] = "Submit";
	$initarr["tabwidth"]    = "AUTO";

	$hiddenarr = NULL;

	$formobj = new formc($initarr, $hiddenarr, 0);

	$fieldx = array ( 
		"title" => "Close?", 
		"name"  => "close",
		"object"=> "checkbox",
		"val"   => $this->parx["close"], 
		"notes" => "close windows after ".$this->closetime." msec?"
		 );
	$formobj->fieldOut( $fieldx );
	
	$fieldx = array ( 
		"title" => "Files", 
		"name"  => "filex",
		"object"=> "textarea",
		"val"   => $this->parx["filex"], 
		"inits" => array( "rows"=> 40, "cols"=> 80 ),
		"notes" => "list of files"
		 );
	$formobj->fieldOut( $fieldx );

	$formobj->close( TRUE );
}

function formshow2() {
	require_once ('func_form.inc');
	
	
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Start";
	$initarr["submittitle"] = "Submit";
	$initarr["tabwidth"]    = "AUTO";

	$hiddenarr = NULL;

	$formobj = new formc($initarr, $hiddenarr, 1);

	$formobj->close( TRUE );
}

function winopen( $url, $cnt ) {
	
	echo "\n".
	"<script language=\"JavaScript\"> \n".
	"	openwin(\"".$url."\", \"".$cnt."\"); \n".
	"</script>\n";

	
}

/**
 * get tablename and get FIRST object of table
 * @param unknown $basex
 * @return string $params
 */
function _get_OBJ_params($basex) {
	$sqlo = &$this->sqlo;
	
	$filename_arr = explode(".",$basex);
	$tablename = $filename_arr[1];
	if ($tablename==NULL) {
		throw new Exception('no tablename in "'.$basex.'" found.');
	}
	
	$tablename_UP = strtoupper($tablename);
	$pkname = PrimNameGet2($tablename_UP);
	if ($pkname==NULL) {
		throw new Exception('no PK found for table  "'.$tablename_UP.'" found.');
	}
	
	// get first OBJECT in TABLE
	$sqlsel = $pkname.' from '.$tablename_UP. ' where '.$pkname.'<10000 order by '.$pkname.' ASC';
	$sqlo->Quesel($sqlsel);
	$sqlo->ReadRow();
	$objid = $sqlo->RowData[0];
	
	$params = '?id='.$objid;
	return $params;
}

// ana one file
function _onefile($filename, $cnt) {
	
	$obj_detect_pattern='obj.';
	
	$fullurl = $this->baseurl."/".$filename;
	echo "FULL: ".$fullurl." ";
	
	$basex = basename($filename);
	$key= ".inc";
	$keylen = strlen($key);
	if ( substr($basex, strlen($basex)-$keylen) == $key ) {
		return;
	}
	
	$urlparams=NULL;
	if (substr($basex,0,strlen($obj_detect_pattern))==$obj_detect_pattern) {
		try {
			$urlparams = $this->_get_OBJ_params($basex);
		} catch (Exception $e) {
			$_SESSION['s_formState'][$this->scriptname]['err_arr'][]='_get_OBJ_params:'.$e->getMessage();
		}
		
	}
	
	$fullurl .= $urlparams;
	
	$this->winopen( $fullurl, $cnt );
	
	echo " o.k. <br>";
	$closetime = $this->closetime;
	
	if ( $this->parx["close"]>0 ) {		
		echo "\n".
		"<script language=\"JavaScript\"> \n".
		"	pause(".$closetime."); \n".
		"	winclose(); \n".
		"</script>\n";
	}
	
	ob_end_flush ( );
	while (@ob_end_flush()); // send all buffered output
}

/**
 * touch each script

 * @return string $ok
 */
function starttest() {
	
	$cnt=0;
	
	# $filearr = explode("\r", $this->filex);
	$filearr = preg_split("/[\n\r\t ]+/", $this->filex);
	# $filearr = split("[\n\r\t ]+", $this->filex);
	
	$_SESSION['s_formState'][$this->scriptname]["filearr"] = $filearr;
	$_SESSION['s_formState'][$this->scriptname]["close"]   = $this->parx["close"];
	
	$this->formshow2();
	/*
	foreach( $filearr as $dummy=>$filename) {
		$this->_onefile($filename, $cnt);
		$cnt++;
	}
	reset ($filearr); 
	*/
}

function forwardOne() {
	$cnt = $this->parx["cnt"];

	$fileshort = $this->fileArr[$cnt];
	echo "Line: ".($cnt+1). " <b>$fileshort</b><br>";
		
	$this-> _onefile($fileshort, $cnt);
}

}

// --------------------------------------------------- 
global $error, $varcol;

$error = & ErrorHandler::get();
$sqlo  = logon2( $_SERVER['PHP_SELF'] ); // URL-link for the first db-login; e.g. for object ($_SERVER['PHP_SELF']."?id=".$id)
if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();

ob_implicit_flush  (TRUE);

$parx = $_REQUEST["parx"];
$go   = $_REQUEST["go"];
$meta = NULL;
$scriptname = "test_touchPages";

$mainlib = new Test_touchPages( $_REQUEST["go"], $_REQUEST["parx"], $scriptname, $sqlo);

if ( $go>1 ) {
	list($metaexist,$meta) = $mainlib->manageForward();
}


$title				 = "Touch pages";

$infoarr			 = NULL;
$infoarr["scriptID"] = $scriptname;
$infoarr["title"]    = $title;
$infoarr["form_type"]= "tool"; // "tool", "list"
$infoarr["locrow"] = array( array("index.php", "Unittests") );
if ( $meta !="" )  $infoarr["headIn"] = $meta;

$pagelib = new gHtmlHead();
$headarr = $pagelib->startPage($sqlo, $infoarr);
?>
<script language="JavaScript">
<!--
var InfoWin = null;

function openwin( url, cnt ) {

    url_name= url;
    InfoWin = window.open( url_name, "TESTWIN"+cnt, "scrollbars=yes,status=yes,width=650,height=500");
    InfoWin.focus();
    document.write ("INFOWIN:" + InfoWin + "<br>" );
    
}

function winclose( ) {
    
   InfoWin.close();
}

function pause(millisecondi)
   {
       var now = new Date();
       var exitTime = now.getTime() + millisecondi;
   
       while(true)
       {
           now = new Date();
           if(now.getTime() > exitTime) return;
       }
   }


</script>
<?php
		
echo "GO:$go metaexists:$metaexist close:".$mainlib->parx["close"]." ";
echo "[<a href=\"".$_SERVER['PHP_SELF']."\">Start again</a>]<br>";


if ( !$go ) {

	echo 'Former Errors:<br>';
	print_r($_SESSION['s_formState'][$scriptname]['err_arr']);
	echo "<br>----<br>\n";

	$mainlib->formshow();
	$_SESSION['s_formState'][$scriptname]['err_arr']=NULL;
	htmlFoot();
}

if ( $go==2 ) {
	if ($metaexist>=1) $mainlib->forwardOne();
	if ($metaexist<0) {
		htmlFoot("Error", "No array found.");
	}
	htmlFoot("<hr>");
}


if ( $_REQUEST["parx"]["filex"]=="" ) {
	htmlFoot("Error", "Need files list");
}

$mainlib->starttest();
htmlFoot("<hr>");