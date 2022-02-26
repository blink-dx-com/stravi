<?php
/**
 * Show statistics of objects in a project (including sub-projects)
 * @package obj.proj.substat.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param  $cacheid   (Proj-ID of cache, contains links to projects)
  		   $oriprojid
 */
session_start(); 


function oneTabAna(&$sql, $cacheProj) {
	$sqls_name = "";
	$tablename = "PROJ";
	$table_nice = tablename_nice2($tablename);
	
	$tabobj = new visufuncs();
	$headOpt = array( "title" => "Number of objects in sub-".$table_nice."s");
	$headx  = array ("Table", "Number");
	$tabobj->table_head($headx,   $headOpt);
	
	$tabcntsum = 0;
	
	// $sqls =  "select count(1) from PROJ_HAS_ELEM where PROJ_ID=".$cacheProj." AND TABLE_NAME='PROJ'";
	$sql_sel_subProjs = " from T_PROJ_ELEM where T_PROJ_ID=".$cacheProj." AND TABLENAME='PROJ'";
	$sqls =  "select count(1)". $sql_sel_subProjs;
	$sql->query($sqls);
	$sql->ReadRow();
	$projcnt = $sql->RowData[0];
	echo "<img src=\"images/icon.PROJ.gif\"> <font color=gray> Number of sub-".$table_nice.":</font> ".$projcnt."<br>\n";
	
	$sqls_cacheProjs = "select OBJID".$sql_sel_subProjs;
	$sqls_cacheObjs= "select count(TABLE_NAME), max(TABLE_NAME) from PROJ_HAS_ELEM where PROJ_ID in (".$sqls_cacheProjs.") group by TABLE_NAME";
		
	$sql->query($sqls_cacheObjs);
	while ($sql->ReadRow() ) {
		$tabcnt = $sql->RowData[0];
		$tabname = $sql->RowData[1];
		$tablenice =  tablename_nice2($tabname);
		if ($tabname=="PROJ")  $tablenice = $table_nice." (links)";
		$dataArr = array("<img src=\"images/icon.".$tabname.".gif\"> ".$tablenice, $tabcnt);
		$tabcntsum = $tabcntsum + $tabcnt;
		$tabobj->table_row ($dataArr);
	}
	
	$dataArr = array("<b>TOTAL:</b> ","<b>".$tabcntsum."</b>");
	$daopt=array("bgcolor" => "#E0E0FF");
	$tabobj->table_row ($dataArr, $daopt);
	$tabobj->table_close();
	
	// go to PROJ_HAS_ELEM of projects
	$sqls_cacheObjLinks= "PROJ_ID in (".$sqls_cacheProjs.")";
	$urlCond = urlencode($sqls_cacheObjLinks);
 	echo "[<a href=\"view.tmpl.php?t=PROJ_HAS_ELEM&condclean=1&tableSCond=$urlCond\"><font color=gray>advanced list: ".$table_nice." has BO</font></A>]<br>";
	

}

require_once ("reqnormal.inc"); // includes all normal *.inc files
require_once ("visufuncs.inc");
// --------------------------------------------------- 
global $error, $varcol;


$cacheid 		= $_REQUEST['cacheid'];
$oriprojid 		= $_REQUEST['oriprojid'];

$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] );
// $sql2  = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();
$tablename = "PROJ";
$table_nice = tablename_nice2($tablename);

$title       = "Show statistics of objects in a ".$table_nice." (including sub-".$table_nice."s)";

#$infoarr['help_url'] = 'o.EXAMPLE.htm';
$infoarr=array();
$infoarr["title"] = $title;
$infoarr["form_type"]= "obj";
$infoarr["obj_name"] = $tablename;
$infoarr["obj_id"]   = $oriprojid;
$infoarr["show_name"]= 1;



$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);

#$sc_scriptname = "scriptID";
if (!$cacheid) htmlFoot("Error", "Need the ID of the Cache-".$table_nice);

echo "[<a href=\"edit.tmpl.php?t=T_PROJ&id=".$cacheid."\">Cache-".$table_nice."</a>]<br>";
echo "<ul>";

oneTabAna( $sql, $cacheid);
 
htmlFoot();

