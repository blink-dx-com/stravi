<?php
/**
 * Analyse continuity of IDs of a selection of objects
 * @package glob.objtab.idAna.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   $tablename
 * @version0 2008-04-01
 */
session_start(); 

require_once ("reqnormal.inc");         // includes all normal *.inc files
require_once ('glob.objtab.page.inc');
require_once ("visufuncs.inc");

class gObjTabIdAna {

function __construct(&$listPageLib, $tablename) {
	$this->tablename = $tablename;
	$this->sqlAfter  = $listPageLib->getSqlAfter();
	$this->barMaxLen = 300;
	$this->barDays   = 400;
	$this->primary_key = PrimNameGet2($tablename);
	
	$this->debug = 0;
}

function rowout(&$sql, $lastid, $tmpname, $diff) {

	$barwidth = intval( $diff/ $this->barDays * $this->barMaxLen);	
	if ($barwidth<0) 	$barwidth=0;
	if ($barwidth>$this->barMaxLen) $barwidth = $this->barMaxLen;
	$dategraph  = "<img src=\"images/point.gif\" height=3 width=".$barwidth.">";
	
	$accinfo = access_data_get( $sql, $this->tablename, $this->primary_key, $lastid);
	
	
	$dataArr = array($lastid, $tmpname, $accinfo["crea_date"] , $diff, $dategraph);
	$this->tabobj->table_row ($dataArr);
	
}

function doit( &$sql, &$sql2 ) {

	$MAXDIFF=1;
	
	$tablename = $this->tablename;
	
	
	$nameCol     = importantNameGet2($tablename);
	
	$this->tabobj = new visufuncs();
	$headOpt = array( "title" => "Statistics" );
	$headx  = array ("ID", "Name", "Crea_date",  "Diff", "Graph");
	$this->tabobj->table_head($headx,   $headOpt);

	
	$sqlsLoop = "SELECT x.".$this->primary_key.", x.".$nameCol." FROM ".$this->sqlAfter;
	$sql2->query($sqlsLoop);
	$cnt=0;
	$lastid = 0;
	while ( $sql2->ReadRow() ) {
		$tmpid   = $sql2->RowData[0];
		$tmpname = $sql2->RowData[1];
		if (!$cnt) {
			$firstobjid = $tmpid;
			$lastid     = $tmpid;
			
			$this->rowout($sql, $tmpid, $tmpname,0);
		}
		
		$diff = $tmpid - $lastid;
		
		if ($diff>$MAXDIFF OR ($this->debug)) {
			$this->rowout($sql,$lastid, $lastname, $diff);
			
		}
		
		$lastid   = $tmpid;
		$lastname = $tmpname;
		$cnt++;
	}
	
	$this->rowout($sql, $lastid, $tmpname, 0);
	
	$this->tabobj->table_close();
}

}


// --------------------------------------------------- 
global $error, $varcol;

$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] ); // URL-link for the first db-login; e.g. for object ($_SERVER['PHP_SELF']."?id=".$id)
$sql2  = logon2( );
if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();

$tablename = $_REQUEST["tablename"];

$title       = "Analyse continuity of IDs of a selection of objects";
$infoarr=array();
$infoarr["back_url"] = "view.tmpl.php?t=".$tablename;
$infoarr["back_txt"] = "list view";
$infoarr["sql_obj"]  = &$sql;
$infoarr["obj_name"] = $tablename;
$infoarr["obj_cnt"]  = 1;          // show number of objects
$infoarr["title"]    = $title;   

$listPageLib = new gObjTabPage($sql, $tablename );
$headopt=array();
$listPageLib->showHead($sql, $infoarr, $headopt);
$listPageLib->initCheck($sql);

$mainLib = new gObjTabIdAna($listPageLib, $tablename);

echo "<ul>";

$mainLib->doit( $sql, $sql2 );

htmlFoot();


