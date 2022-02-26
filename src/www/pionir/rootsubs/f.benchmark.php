<?php
/*MODULE:  f.benchmark.php
  DESCR:   do a system benchmark
  AUTHOR:  qbi
  INPUT:   $go
  VERSION: 0.1 - 20060918
*/

extract($_REQUEST); 
session_start(); 


require_once ("reqnormal.inc"); // includes all normal *.inc files
require_once ('func_form.inc');
require_once ("visufuncs.inc");

class fBenchC {

function __construct() {
	
	
	$this->dbuserid = $_SESSION['sec']['db_user_id'];
	$this->loopMax  = 5000;
	$this->timeArr  = NULL;
	$this->refTime = NULL;
	$this->refTime["insert"] = "7.0 ... 8.6";  // 8.6678
	$this->refTime["select"] = 0.109 ; // 0.1098 0.1084
	$this->MAX_POINTS = 100;
}

function get_CPU_info() {

	$filename = "/proc/cpuinfo";
	$keywords = array("model name"=>1, "cpu MHz"=>1);
	
	if ( !file_exists($filename) )  return;
	
	$infoout = "";
	$strarr = file ( $filename );
	if (!sizeof($strarr)) return;
	
	$tmpkomma = "";
	foreach( $strarr as $dummy=>$val) {
		
		$infoarr = explode(":",$val);
		$key = trim($infoarr[0]);
		if ( $keywords[$key]!="" ) {
			$infoout .= $tmpkomma .$key.": ". trim($infoarr[1]);
			$tmpkomma = "; ";
		}
	}
	reset ($strarr); 
	
	return($infoout);
}

function stamp($keyword) {

	//$timenow = time();
	list($usec, $sec) = explode(" ", microtime()); 
	$this->timeArr[$keyword] = $sec + $usec;
}

function insertx( &$sql ) {

	$loopCnt=0;
	while ( $loopCnt < $this->loopMax ) {
		$colstr = "DB_USER_ID, TABLE_NAME, COLUMN_NAME, VALUE";
		$valstr = $this->dbuserid.", 'TEST', 'TEST', '".$loopCnt."'";
		$sql->Insertx ( "T_EXPORT", $colstr, $valstr);
		$loopCnt++;
	}

}

function select( &$sql ) {

	$sqls = "select * from T_EXPORT where DB_USER_ID=".$this->dbuserid." order by VALUE";
	$sql->query($sqls);
	while ( $sql->ReadRow() ) {	
		$retid = $sql->RowData[0];
		$loopCnt++;
	}
	
}

function delete_this(&$sql) {
	
	$sqls = "DB_USER_ID=".$this->dbuserid;
	$sql->deletex('T_EXPORT', $sqls);
}

function calcDiff($keyfrom, $keyto) {
	$val = $this->timeArr[$keyto] - $this->timeArr[$keyfrom];
	$val = sprintf("%.4f", $val);
	return ($val);
}

function analyseOnetime($key, $notes, $useRef=0) {

	if ($useRef) { // use the reference system
		$systemArr = &$this->refTime;
	} else {
		$systemArr = &$this->anatime;
	}
	$facttmp   = $systemArr[$key] / $this->refTime[$key];
	$img_width =  floor($facttmp * $this->MAX_POINTS);
	$bargraph  = '<img src="../images/point.gif" height="2" width="'.$img_width.'">';
	$retvals   = array ($notes." : ".$key, $systemArr[$key], $bargraph);
	return ($retvals);
}

function analyseTime() {

	$this->anatime = NULL;
	$this->anatime["insert"] = $this->calcDiff("insert_start", "insert_stop");
	$this->anatime["select"] = $this->calcDiff("select_start", "select_stop");;

	
	$tabobj = new visufuncs();
	$dataArr= NULL;
	$dataarr = NULL;
	$dataarr[] = $this->analyseOnetime("insert", "this system");
	$dataarr[] = $this->analyseOnetime("select", "this system");
	
	$dataarr[] = $this->analyseOnetime("insert", "ref system", 1);
	$dataarr[] = $this->analyseOnetime("select", "ref system", 1);
	
	
	$headOpt = array( "title" => "Benchmark Summary");
	$headx   = array ("action", "time (sec)", "bargraph");
	$tabobj->table_out2( $headx, $dataarr,  $headOpt) ;
	
	// print_r ( $this->timeArr);
	
}

function fullBench(&$sql) {

	
	echo "Insert/select objects: <b>".$this->loopMax."</b><br>";

	$this->delete_this($sql);
	
	$this->stamp("start");
	
	$this->stamp("insert_start");
	$this->insertx( $sql );
	$this->stamp("insert_stop");
	
	$this->stamp("select_start");
	$this->select( $sql );
	$this->stamp("select_stop");
	
	$this->delete_this($sql);
	
	$this->analyseTime();
}

}
// --------------------------------------------------- 
global $error, $varcol;

$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF']."?id=".$id ); // give the URL-link for the first db-login
$sql2  = logon2( );
if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();



$title       = "System benchmark test";

$infoarr			 = NULL;

$infoarr["title"] = $title;
$infoarr["form_type"]= "tool";

$infoarr["locrow"]= array( array('rootFuncs.php', 'back' ) );
$infoarr["help_url"] ="super_user_functions.html";

// $infoarr["version"] = '1.0';	// version of script


$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);
echo "<ul>\n";

//$sc_scriptname = "scriptID"; 
if ( $_SESSION['sec']['appuser'] != "root" ) { // !$_SESSION['s_suflag'] 
     htmlErrorBox( "Error",   
     "Only root can execute this!",
     "For security reason it is not allowed for common users" );
     htmlFoot();
}

if ( !$go ) {
	echo "Notes: This is a benchmark test for database functions. It uses the SQL-commands INSERT and SELECT (using table T_EXPORT)<br><br>\n";
}
$mainObj = new fBenchC();

$cpuinfo = $mainObj->get_CPU_info();

echo "<b>This-CPU info:</b> ".$cpuinfo."<br>\n";
echo "<b>REF-CPU info:</b> model name: Intel(R) Xeon(R) CPU 3060 @ 2.40GHz; cpu MHz: 2394.061; model name: Intel(R) Xeon(R) CPU 3060 @ 2.40GHz; cpu MHz: 2394.061";
echo "<br><br>";

//if ( !$go ) {
	
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Start a benchmark test";
	$initarr["submittitle"] = "Start";
	$initarr["tabwidth"]    = "AUTO";

	$hiddenarr = NULL;

	$formobj = new formc($initarr, $hiddenarr, 0);
	$formobj->close( TRUE );
if ( !$go ) {
	htmlFoot("<hr>");
}
//}

echo "<br><b>Start the benchmark</b><br><br>";



$mainObj->fullBench($sql);

htmlFoot("<hr>");