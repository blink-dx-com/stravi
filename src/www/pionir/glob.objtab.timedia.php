<?php
/**
 * show diagram of time distribution
 * @package glob.objtab.timedia.php
 * @namespace core::gui::objtab
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param $tablename
  		  $parx : see gui/glob.objtab.timedia.inc
 */
session_start(); 


require_once ('reqnormal.inc');
require_once ('gui/glob.objtab.timedia.inc');
require_once ('f.progressBar.inc');


$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] );
$sql2  = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();

$tablename= $_REQUEST["tablename"];
$parx = $_REQUEST["parx"];

$flushLib = new fProgressBar( 1000 );


$title       = '[ObjectTimeGraph] Show creation time of selection objects in a diagram';
$infoarr=array();
$infoarr["title"]    = $title;
$infoarr["title_sh"] = "ObjectTimeGraph";
$infoarr["form_type"]= "list";
$infoarr["obj_name"] = $tablename;
$infoarr["obj_cnt"]  = 1;          // show number of objects
$infoarr["javascript"] = $flushLib->getJS(); 

$cssstr = $flushLib->getCss() ;
$cssstr .=  "\n" ."table.xdataTab td  { font-size:0.8em;  }\n".
		    "table.xdataTab table  { padding-right=10px; padding-left=10px; }".
			"";

$infoarr["css"] = $cssstr;

$pagelib = new gHtmlHead();
$headarr = $pagelib->startPage($sql, $infoarr);
echo "<ul>\n";

if ($tablename=="") htmlFoot("Error", "No tablename given"); 
$isbo = cct_access_has2($tablename);

$othertables = array( "CCT_ACC_UP"=>1 , "CCT_ACC_UP_VIEW"=>1 );
if ( !$isbo AND !isset($othertables[$tablename]) ) htmlFoot("Error", "Only business objects and some system-tables supported."); 


$mainlib = new gObjtabTimedia($tablename, $parx);
$mainlib->setProgBarLib($flushLib);

$sqlQuerLib = new fSqlQueryC($tablename);
$sqlopt["order"] = 0;
$sqlAfter = $sqlQuerLib->get_sql_after($sqlopt); 
$mainlib->setQuery($sqlAfter);

$tablenice = tablename_nice2($tablename);

gHtmlMisc::func_hist("glob.objtab.timedia", $title, $_SERVER['PHP_SELF']."?tablename=".$tablename );

$stopReason = "";
$tmp_info   = $_SESSION['s_tabSearchCond'][$tablename]["info"];
if ($tmp_info=="") $stopReason = "No elements selected.";
if ($headarr["obj_cnt"] <= 0) $stopReason = "No elements selected.";
if ($stopReason!="") {
    htmlFoot("Attention", $stopReason." Please select elements of '".$tablenice."'!");
}

$mainlib->prepare($sql);

$mainlib->formshow($sql);

if (!$go) htmlFoot();

if ($parx["duration"]<1) {
	htmlFoot("Error", "Please select the scaling!");
}

$mainlib->doloop ( $sql, $sql2 ); 

htmlFoot("</ul><hr>");
