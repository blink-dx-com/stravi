<?php
/**
 * search content inside docs
 * @package obj.link.searchDoc.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   
 *    int $go
 *    $parx[]
  			"searchtxt"		search text
			"casesense"	    match case?
			"breakFirst"	Break after first match in document
			"showMisFiles" = 0|1
 */ 
session_start(); 


require_once ("reqnormal.inc"); // includes all normal *.inc files
require_once ("sql_query_dyn.inc");
require_once ("o.LINK.subs.inc");

function this_formshow($parx) {
	require_once ('func_form.inc');
	
	
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Give search text";
	$initarr["submittitle"] = "Search";
	$initarr["tabwidth"]    = "AUTO";

	$hiddenarr = NULL;

	$formobj = new formc($initarr, $hiddenarr, 0);

	$fieldx = array ( "title" => "Search text", "name"  => "searchtxt",
			"object" => "text",
			"val"   => $parx["searchtxt"],
			"notes" => "common string" );
	$formobj->fieldOut( $fieldx );
	
	$fieldx = array ( "title" => "Case sensitive", "name"  => "casesense",
			"object" => "checkbox", "inits" => 1,
			"val"   => $parx["casesense"],
			"notes" => "match case?" );
	$formobj->fieldOut( $fieldx );
	
	$fieldx = array ( "title" => "Break after 1. match", "name"  => "breakFirst",
			"object" => "checkbox", "inits" => 1,
			"val"   => $parx["breakFirst"],
			"notes" => "Break after first match in document?" );
	$formobj->fieldOut( $fieldx );
	
	$fieldx = array ( "title" => "Show also unmatched", "name"  => "showMisFiles",
			"object" => "checkbox", "inits" => 1,
			"val"   => $parx["showMisFiles"],
			"notes" => "Show also umatched documents" );
	$formobj->fieldOut( $fieldx );

	$formobj->close( TRUE );

}

function this_infoOut( $objid, $objcnt, $lineno, $foundcnt, $tmpname, $buffer, $bad=NULL) {

	if (strlen($buffer)>200) $buffer = substr($buffer,0,200)."...";
	$outbuf = htmlspecialchars($buffer);
	
	$bgcolor= "#EFEFEF";
	if ( !$foundcnt ) $bgcolor= "#EFEFD0";
	if ( $bad ) {
		 $bgcolor= "#FFC0C0";
		 $outbuf = "<font color=red>no match</font>";
		 $lineno = "";
	}
	echo "<tr bgcolor=$bgcolor>";
	if ( !$foundcnt ) echo "<td>$objcnt</td><td><a href=\"edit.tmpl.php?t=LINK&id=".$objid."\">".$tmpname."</a></td>";
	else echo "<td colspan=2>&nbsp;</td>";
	
	echo "<td>".$lineno."</td><td>".$outbuf."</td></tr>\n" ;
	while (@ob_end_flush()); // send all buffered output
	
}
function this_infoOutEnd($found) {

}

function this_search_str( $objid, $tmpname, $objcnt, 
	$parx, // "breakFirst" => "1" => break after first found
	$opt = NULL	
	) {
	$filename = linkpath_get( $objid );
	if ( !file_exists($filename) ) {
		return;
	} 
	$searchtxt  = $parx["searchtxt"];
	$searchcase = $parx["casesense"];
	$found = 0;
	$lineno= 0;
	$handle = fopen($filename, "r");
	
	
	while (!feof($handle)) {
		$buffer = fgets($handle, 16384);
		if ($searchcase) $seres = strstr($buffer, $searchtxt);
		else $seres = stristr($buffer, $searchtxt);
		
		if ( $seres != NULL) {
			if (!$found) $bufferFirst = $buffer;		
			this_infoOut( $objid, $objcnt, $lineno, $found, $tmpname, $buffer);
			
			$found++;
			if ($parx["breakFirst"]=="1") {
				break;
			}
		}
		$lineno++;
	}
	
	fclose($handle);
	
	if ( !$found AND ($parx["showMisFiles"]==1) ) {
		$badflag = 1;
		this_infoOut( $objid, $objcnt, $lineno, $found, $tmpname, "", $badflag);
		
	}
	
	return array($found, $bufferFirst);
}

global $error, $varcol;

$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] );
//$sql2  = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();

$parx = $_REQUEST["parx"];

$tablename = "LINK";
$tablenice ="document";


$title       = "Search document content";

$infoarr=array();
$infoarr["title"] = $title;
$infoarr["form_type"]= "list";
$infoarr["obj_name"] = $tablename;
$infoarr["obj_cnt"]  = 1;          // show number of objects


$pagelib = new gHtmlHead();
$headarr = $pagelib->startPage($sql, $infoarr);
echo "<ul>";
ob_end_flush ( );

gHtmlMisc::func_hist("obj.link.searchDoc", $title, $_SERVER['PHP_SELF'] );
 
$t_rights = tableAccessCheck( $sql, $tablename );
if ( $t_rights["write"] != 1 ) {
	tableAccessMsg( $tablenice, "write" );
	htmlFoot();
}
$sqlopt=array();
$sqlopt["order"] = 1;
$sqlAfter  = get_selection_as_sql( $tablename, $sqlopt);


$stopReason = "";
$tmp_info   = $_SESSION['s_tabSearchCond'][$tablename]["info"];
if ($tmp_info=="") $stopReason = "No elements selected.";
if ($headarr["obj_cnt"] <= 0) $stopReason = "No elements selected.";
if ($stopReason!="") {
    htmlFoot("Attention", $stopReason." Please select elements of '".$tablenice."'!");
}

// $nicename_table = tablename_nice2($tablename);
this_formshow($parx);

if (!$go OR $parx["searchtxt"]=="") {
	htmlFoot();
}

echo "<font size=+1><b><font color=gray>Search now for:</b></font> <b>".htmlspecialchars($parx["searchtxt"])."</b></font>";
if ($parx["breakFirst"]) echo ", break after first match";
if ($parx["showMisFiles"]) echo ", show also unmatched documents";

echo "<br>\n";


$primary_key = "LINK_ID";

//$sql->query("SELECT count(x.".$primary_key.") FROM ".$sqlAfter);
//$sql->ReadRow();
//$w_cnt = $sql->RowData[0];
$foundcnt=0;

echo "<table border=0>";
$sqlsLoop = "SELECT x.".$primary_key.", x.NAME FROM ".$sqlAfter;
$sql->query($sqlsLoop);
$objcnt=1;
while ( $sql->ReadRow() ) {
    $tmpid   = $sql->RowData[0];
	$tmpname = $sql->RowData[1];
	
	list($found, $buffer) = this_search_str($tmpid, $tmpname, $objcnt, $parx);
	if ($found) {
		$foundcnt++;
	}
	$objcnt++;
	
}
echo "</table>";

echo "<hr>";
echo "Result: <B>$foundcnt</B> file matches.<br>";

$pagelib->htmlFoot();