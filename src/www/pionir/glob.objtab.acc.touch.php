<?php
/**
 * touch a list of objects
 * @package glob.objtab.acc.touch.php
 * @author  qbi
 * @version 1.0
 * @param string $t tablename
 * @param int $go 0,1,2
 * @param array $parx
 */
session_start(); 


require_once ("reqnormal.inc"); // includes all normal *.inc files
require_once ("sql_query_dyn.inc");
require_once ("f.visu_list.inc");     // sub function LIST visualization
require_once ('glob.objtab.page.inc');
require_once ("f.sql_query.inc");	
require_once 'glob.obj.touch.inc';

class gObjtabTouch {
	
function __construct($tablename, $go, $parx) {
	$this->tablename = $tablename;
	$this->go = $go;
	$this->parx=$parx;
	
	$sqlQuerLib = new fSqlQueryC($tablename);
	$sqlopt["order"] = 1;
	$this->sqlAfter = $sqlQuerLib->get_sql_after($sqlopt); 
}

function form1() {
	require_once ('func_form.inc');
	
	
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Give touch string";
	$initarr["submittitle"] = "Submit";
	$initarr["tabwidth"]    = "AUTO";

	$hiddenarr = NULL;
	$hiddenarr["t"]     = $this->tablename;

	$formobj = new formc($initarr, $hiddenarr, 0);

	$fieldx = array ( 
		"title" => "Touch-string", 
		"name"  => "touchstr",
		"object"=> "text",
		"val"   => $this->parx["touchstr"], 
		"notes" => "the touch JSON-string"
		 );
	$formobj->fieldOut( $fieldx );

	$formobj->close( TRUE );
}

function doit(&$sqlo, &$sqlo2) {
    global $error;
    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
    
   
    
	$reason    = $this->parx["touchstr"];
	echo "Touch now: ".htmlspecialchars($reason)."<br>";
	$reason_dict=json_decode($reason,TRUE);
	if (!is_array($reason_dict)) {
	    $error->set( $FUNCNAME, 1, 'Input-string is not JSON!' );
	    return;
	}
	if (empty($reason_dict)) {
	    $error->set( $FUNCNAME, 1, 'Input-string is not JSON!' );
	    return;
	}
	
	$tablename = $this->tablename;
	$primary_key = PrimNameGet2($this->tablename);
	$sqlsLoop = "SELECT x.".$primary_key." FROM ".$this->sqlAfter;
	$sqlo2->query($sqlsLoop);
	$cnt=0;
	while ( $sqlo2->ReadRow() ) {
		$objid = $sqlo2->RowData[0];
		globObjTouch::touch( $sqlo, $tablename, $objid, $reason_dict );
		$cnt++;
		if ( ($cnt/200)== intval($cnt/200) ) echo "*";
	}
	echo "<br>";
	echo "Touched objects: <b>$cnt</b><br>";
}
	
}

// --------------------------------------------------- 
global $error;

$error = & ErrorHandler::get();
$sqlo  = logon2( $_SERVER['PHP_SELF'] ); // URL-link for the first db-login; e.g. for object ($_SERVER['PHP_SELF']."?id=".$id)
$sqlo2 = logon2( );
if ($error->printLast()) htmlFoot();
// $varcol = & Varcols::get();

$tablename=$_REQUEST["t"];
$go=$_REQUEST["go"];
$parx=$_REQUEST["parx"];

//$i_tableNiceName 	 = tablename_nice2($tablename);

$title       		 = "Touch a selection of objects";
$infoarr			 = NULL;
$infoarr["title"]    = $title;     
$infoarr["scriptID"] = "glob.objtab.acc.touch";
$infoarr["obj_cnt"]  = 1;          // show number of objects


$mainObj = new gObjTabPage($sqlo, $tablename );
$mainObj->showHead($sqlo, $infoarr);
$mainObj->initCheck($sqlo);
echo "<ul>\n";

if ( !glob_isAdmin() ) {
     htmlErrorBox( "Error",   
     "Only root can execute this!",
     "For security reason it is not allowed for common users" );
     htmlFoot();
}

$mainlib = new gObjtabTouch($tablename, $go, $parx);

if ( !$go ) {
	$mainlib->form1();
	htmlFoot("<hr>");
}

if ($parx["touchstr"]=="") {
	htmlFoot("Error", "Please give the touch-text");
} 

$mainlib->doit($sqlo, $sqlo2);
$error->printAll();

htmlFoot("<hr>");