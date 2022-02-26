<?php
/**
 * cross search of table1 ($t) and $table2 ($t2)
 * @package glob.objtab.seaCross.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   $table1
        $table2
        $go
 * @version0 2008-04-01
 */
session_start(); 


require_once ("reqnormal.inc"); // includes all normal *.inc files
require_once ("f.sql_query.inc");	
require_once ("f.visu_list.inc"); 
require_once ( "javascript.inc" );
require_once ("visufuncs.inc");
require_once ("f.objview.inc");	
require_once ('glob.objtab.page.inc');
// --------------------------------------------------- 
global $error, $varcol;

class fObjCrossSearch {
	
var $table2src; // source of table2
	
function __construct($table1, $table2) {
	
	$this->table1 = $table1;
	$this->table2 = $table2;
	$this->crossknown = 
			array("EXP"=>array(
						"SAMPLE_IN_EXP"=>array("tabsrc"=>"CONCRETE_SUBST", "k1"=>"EXP_ID", "k2"=>"SAMPLE_CONCRETE_SUBST_ID") 
						)
			);
	if ( $this->table2 != "" ) {
		$crossinf2 = $this->crossknown[$table1][$table2];
		$this->table2src = $crossinf2["tabsrc"];
	}
}

function _cntShow(&$sqlo, $x, $table) {
	
	$visuLib = new visu_listC();
	$options = array("doCount"=>1);
	$answer = $visuLib->checkSelection( $sqlo, $table, $options);
	if ($answer[0]<0) {
		 $cnt = 0;
	} else {
		 $cnt = $answer[2];
	}
	
	$tablelink = $this->objViewLib->tableViewLink($table);
	$dataArr = array("table".$x, $tablelink , 
					 "<b>".$cnt."</b> selected elements" );
	
	$this->tabobj->table_row ($dataArr);
	return (0);
}

function info(&$sqlo) {
	
	$table1 = $this->table1;
	$table2 = $this->table2;
	
	$this->tabobj = new visufuncs();
	$headOpt = array( "title" => "Table info" );
	$headx  = array ("#", "Table", "selected elements");
	$this->tabobj->table_head($headx,   $headOpt);
	
	$this->objViewLib = new fObjViewC();
	
	$cnt1 = $this->_cntShow($sqlo, 1, $table1);
	if ($table2!="") { 
		$cnt2 = $this->_cntShow($sqlo, 2, $this->table2src);
	}
	
	$this->tabobj->table_close();
	echo "<br>";
}

function showtab2Sel(&$sqlo) {
	global $error;
	$FUNCNAME= "showtab2Sel";
	$crosstabs = $this->crossknown[$this->table1];
	if ($crosstabs==NULL) {
		 $error->set( $FUNCNAME, 1, "No cross table found." );
		 return;
	}
	echo "Possible crossing tables:<br><br>\n";
	foreach( $crosstabs as $tab2=>$tab2arr) {
		$selsrc = $tab2arr["tabsrc"];
		echo "- <b><a href=\"".$_SERVER['PHP_SELF']."?t=".$this->table1."&t2=".$tab2."&go=1\">".
				tablename_nice2($selsrc) ."</a></b>".
				"  (via ".tablename_nice2($tab2).")" .
				"<br>";
	}
	echo "<br>";
	reset ($crosstabs); 
}

function checktables( &$sqlo ) {
	global $error;
	$FUNCNAME= "checktables";
	
	$table1 = $this->table1;
	$table2 = $this->table2;
	
	$visuLib = new visu_listC();
	
	$options = array("doCount"=>1);
	$answer = $visuLib->checkSelection( $sqlo, $this->table2src, $options);
	if ($answer[0]<0) {
		 $error->set( $FUNCNAME, 1, "cross-table ".$table2.": search in '".$this->table2src."':".$answer[1] );
		 return;
	} 
	
}

function docrossing( &$sqlo ) {
	// do crossing now
	$table1 = $this->table1;
	$table2 = $this->table2;
	
	$crossinfo = $this->crossknown[$table1][$table2];
	
	// search in SRC of table2
	$sqlQuerLib2  = new fSqlQueryC($this->table2src);
	$sqlAfterTab2 = $sqlQuerLib2->get_sql_after(); 
	
	$cond = $crossinfo["k1"]."\n".
			 " in ( select EXP_ID from SAMPLE_IN_EXP where SAMPLE_CONCRETE_SUBST_ID in ( \n".
			 "         select x.CONCRETE_SUBST_ID from " . $sqlAfterTab2. "\n".
			 "      ) \n".	
 			 ")\n";
	
	$queryLib = new fSqlQueryC($table1);
	$infostr = "in cross table $table2";
	$queryLib->addCond ($cond,"AND",$infostr);
	$sqlAfterTab1 = $queryLib->get_sql_after(); 
	
	$primary_key = PrimNameGet2($table1);
	
	$sqls = "SELECT count(x.".$primary_key.") FROM ".$sqlAfterTab1;
	$sqlo->query($sqls);
	$sqlo->ReadRow();
	$w_cnt = $sqlo->RowData[0];
	echo "Result of cross-search: <b>$w_cnt</b> objects of ".tablename_nice2($table1)." <br>";
	echo "Info: this new selection is now ACTIVE.<br>";
	echo "<br><br>\n";
	
	$queryLib->queryRelase();
}

}

$error = & ErrorHandler::get();
$sqlo  = logon2( $_SERVER['PHP_SELF'] ); // URL-link for the first db-login; e.g. for object ($_SERVER['PHP_SELF']."?id=".$id)
$sqlo2 = logon2( );
if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();

$table1 = $_REQUEST["t"];
$table2 = $_REQUEST["t2"];
$go = $_REQUEST["go"];


		
$table1nice = tablename_nice2($table1);
$table2nice = tablename_nice2($table2);
if ($table2!="") { 
	$title       = "Cross search of table1 ($table1nice) and table2 ($table2nice)";
} else {
	$title       = "Cross search of table1 ($table1nice) and table2 (???)";
}
$infoarr=NULL;
$infoarr['title']    =$title;
$infoarr['title_sh']    ='Cross search';
$infoarr["form_type"]= "list";
$infoarr["sql_obj"]  = &$sqlo;
$infoarr["obj_name"] = $table1;
$infoarr["obj_cnt"]  = 1;          // show number of objects

$mainObj = new gObjTabPage($sqlo, $table1 );
$mainObj->showHead($sqlo, $infoarr);

echo "<ul>";
$mainObj->initCheck($sqlo);
$mainlib = new fObjCrossSearch($table1, $table2);


if ( !$go) {
	$mainlib->showtab2Sel($sqlo);
	$error->printAll();
	htmlFoot("<hr>");
}

if ($table2=="") {
	htmlFoot("Error", "Give table2.");
} 

$mainlib->info($sqlo);

$mainlib->checktables($sqlo);
if ( $error->printAll() ) {
	htmlFoot();
}

$mainlib->docrossing($sqlo);
if ( $error->printAll() ) {
	htmlFoot();
}

$stopopt = array(1, "getting info");
$url="view.tmpl.php?t=".$table1;
js__location_replace($url, "list view of ".$table1, $stopopt);

htmlFoot("<hr>");