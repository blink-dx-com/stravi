<?php
/**
 * search rights for the selected objects with mod-rights 
 * @package glob.objtab.access.sea.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   $t -- tablename
		 $act : ["withmod"] -- with mod-rights
 */
session_start(); 


require_once ("reqnormal.inc"); // includes all normal *.inc files
require_once ("f.sql_query.inc");

class gAccSeaC {
	
function __construct($t, $act) {
	$this->tablename = $t;
	$this->act=$act;
	$this->sqlQuerLib = new fSqlQueryC($this->tablename);
	
	$this->actmodes = array( "withmod"=> "Search objects with MOD-rights" );
}

function _f_withmod(&$sqlo) {
	global $error;
	$FUNCNAME= "_f_withmod";
	
	$sqlAdd = "x.CCT_ACCESS_ID in (".
				"select CCT_ACCESS_ID from CCT_ACCESS_RIGHTS where INSERT_RIGHT=1 OR UPDATE_RIGHT=1 OR DELETE_RIGHT=1 OR  ENTAIL_RIGHT=1".
				")";
	$this->sqlQuerLib->addCond($sqlAdd, "AND", "with MOD-rights");
	$sqlsAfter = $this->sqlQuerLib->get_sql_after(); 
	$sqls = "select count(1) from ".$sqlsAfter;
	$retval = $sqlo->query($sqls);
	if (!$retval OR $error->Got(READONLY))  {
		$error->set( $FUNCNAME, 1, "SQL-query is wrong: sqlsAfter: ". $sqlsAfter);
		return;
	}
	$sqlo->ReadRow();
	$retid = $sqlo->RowData[0];
	echo "<b>$retid</b> objects found with MOD-rights.<br>\n";
	echo "... release as CURRENT condition.<br>\n";
	
	$this->sqlQuerLib->queryRelase();
}

function search( &$sqlo ) {
	
	echo "Mode: <b>".$this->actmodes[$this->act]."</b> (".$this->act.")<br><br>";
	
	switch  ($this->act) {
		case "withmod":
			$this->_f_withmod($sqlo);
			break;
		default:
			htmlFoot("Error", "Mode unknown");
	}
}

}		
		
// --------------------------------------------------- 
global $error, $varcol;

$act 		= $_REQUEST["act"];
$tablename  = $_REQUEST["t"];

$error = & ErrorHandler::get();
$sqlo  = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();

$tablename			 = $t;

$title       		 = "Search access rights in selection";
$infoarr = NULL; 
$infoarr["title"] = $title;
$infoarr["scriptID"] = "";
$infoarr["form_type"]= "list";

$infoarr["obj_name"] = $tablename;
$infoarr["obj_cnt"]  = 1; 
$infoarr["locrow"]   = array( array("glob.objtab.access.php?t=".$tablename, "AccessInfo") );

//$infoarr['help_url'] = "o.EXAMPLE.htm";

$pagelib = new gHtmlHead();
$pagelib->startPage($sqlo, $infoarr);
echo "<ul>\n";

$mainlib = new gAccSeaC($tablename, $act);
$mainlib->search( $sqlo );
$error->printAll();
 
htmlFoot("<hr>");