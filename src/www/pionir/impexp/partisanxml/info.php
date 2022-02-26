<?php
/*MODULE: info.php
  DESCR:  info about paxml modules
  AUTHOR: qbi
  INPUT:  
  VERSION: 0.1 - 20050925
*/



extract($_REQUEST); 
session_start(); 


require_once ("reqnormal.inc"); // includes all normal *.inc files
require_once ("visufuncs.inc");

function this_news() {
	htmlInfoBox( "PAXML News", "", "open", "CALM" );
	?>
	<ul>
	<li>2008-04-11 Support of export/import of attachments (on business objects)</li>
	</ul>
	<?
	htmlInfoBox( "", "", "close" );
}

function this_eximFlagInfo( &$sql ) {
	$vopt = array ("icon"=>"ic.del.gif");
	
	echo "<h2>List of EXPORT-denied tables/columns </h2><br>\n";

	echo "\"Export denied\" is defined by the EXIM-flag.<br><br>\n";
	
	$tabobj = new visufuncs();
	$headOpt = array( "title" => "Tables, denied for export", "headNoShow" => 1
	 		          );
	$headx  = array("Table");
	
	$tabobj->table_head($headx, $headOpt);
	
	$stmt = "SELECT table_name, exim FROM cct_table WHERE exim > 0 order by NICE_NAME";
	$sql->query($stmt);
	while ( $sql->ReadRow() ) {
		$denyTab = $sql->RowData[0];
		$nicename = tablename_nice2($denyTab);
		$dataArr  = array("<a href=\"../../edit.tmpl.php?t=CCT_TABLE&id=".$denyTab."\">".$nicename."</a>");
		$tabobj->table_row ($dataArr);
	}
	$tabobj->table_close();
	
	echo "<br><br>";
	
	
	$tabobj = new visufuncs();
	$headOpt = array( "title" => "Columns, denied for export");
	$headx  = array("Table", "Column");
	$lasttable = "";
	$tabobj->table_head($headx, $headOpt);
	$stmt = "SELECT table_name, column_name, exim FROM cct_column WHERE exim>0 order by table_name";
	$sql->query($stmt);
	while ( $sql->ReadRow() ) {
		$denyTab = $sql->RowData[0];
		$colx = $sql->RowData[1];
		$nicename = tablename_nice2($denyTab);
		$colnice  = columnname_nice2($denyTab, $colx);
		if ($lasttable == $denyTab) $nicename="";
		$dataArr  = array($nicename, 
			"<a href=\"../../edit.tmpl.php?t=CCT_COLUMN&id=".$denyTab."&primasid[1]=$colx\">".$colnice."</a>"
			);
		$tabobj->table_row ($dataArr);
		$lasttable = $denyTab;
	}
	
	$tabobj->table_close();
	
	
}

// --------------------------------------------------- 
global $error, $varcol;

$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();


$title       = "PAXML info";

#$infoarr['help_url'] = 'o.EXAMPLE.htm';
$infoarr=array();
$infoarr["title"] = $title;
$infoarr["form_type"]= "tool";

$infoarr["locrow"]= array( array('../../home.php', 'home' ) );
$infoarr['icon']     = '../../images/ic.paxml.png';




$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);

echo "<ul>";
echo "<i>Info: This tool informs about special PAXML settings in this database.</i><br><br>\n";

this_eximFlagInfo($sql);

echo "<br>";
this_news();

htmlFoot();
