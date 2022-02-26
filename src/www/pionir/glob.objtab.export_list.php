<?php
/**
 * - export/import GUI for data for list of objects
   - TBD: for export: offer the option to produce a temporary download file on server, because for
		 large, time consuming data-export, the server connection is timed out
 * @package glob.objtab.export_list.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param   $tablename
  	  $go
 */ 
session_start(); 


require_once("db_access.inc");
require_once("globals.inc");
require_once("func_head.inc"); 
require_once("role.inc");

class objtab_expli_Gui {
	
function expform($tablename) {
	require_once ('func_form.inc');
	
	
	$initarr   = NULL;
	$initarr["action"]      = "glob.objtab.exp.php";
	$initarr["title"]       = "Advanced export parameters";
	$initarr["submittitle"] = "Export";
	$initarr["tabwidth"]    = "AUTO";

	$hiddenarr = NULL;
	$hiddenarr["t"]    = $tablename;

	$formobj = new formc($initarr, $hiddenarr, 0);

	//radio    => needs "inits" size=1 => conatins VALUE
					//							   >1 => array[VALUE] = TEXT for an radio field
					//		     "optx" => array ("rowbr"=>1) line break for radio elements
	
	$fieldx = array ( "title" => "Name or ID?", 
			"name"   => "view_opt[pureids]",   // ShowFK
			"object" => "radio", 
			"val"   => 0, 
			"inits" => array(0=>'name + ID', 1=>'only ID', 2=>'only name'), 
			"namex" => 1,
	 		"optx" => array ("rowbr"=>1),
			"notes" => "Export IDs instead of names of linked objects" );
	$formobj->fieldOut( $fieldx );
	
	
	
	
	$fieldx = array ( "title" => "Code column names", 
			"name"  => "view_opt[colCoNa]",
			"object" => "checkbox", "val"   => 0, "inits" => "1", "namex" => 1, "colspan"=>2,
			"notes" => "&nbsp;Column names: Show code-names (e.g. EXP_ID) instead of nice-names (e.g. 'exp id')" );
	$formobj->fieldOut( $fieldx );
	
	//OLD: "xls"=>"Excel 2003", 
	$fieldx = array ( "title" => "File format", 
			"name"   => "format",
			"object" => "select", 
	        "inits"  => array("xlsx"=>"Excel (xlsx)", "csv"=>"CSV", "xlsxml"=>"Microsoft XML Spreadsheet"), 
			"value"  => "xlsx",
			"namex"  => 1,
			"notes"  => "xlsx, csv, xml, ..." );
	$formobj->fieldOut( $fieldx );

	$formobj->close( TRUE );
	
}

function linkOut( 
		$url, 
		$text,
		$active=1
	) {
	// get link-text
	echo "<img src=\"images/greenball.gif\"> "; 
	if (!$active) $outtext = "<font color=gray>".$text."</font>";
	else $outtext = "<a href=\"".$url."\">".$text."</a>";
	$outtext .= '<br>'."\n";
	echo $outtext;
}

}

$sql  = logon2( $_SERVER['PHP_SELF'] );

$tablename=$_REQUEST['tablename'];
$go=$_REQUEST['go'];

$title= "Export/Import list wizard";
$infoarr=array();
$infoarr["title"] = $title;
$infoarr["form_type"]= "list";
$infoarr["obj_name"] = $tablename;  
$infoarr["obj_cnt"]  = 1;

$pagelib = new gHtmlHead();

$headarr = $pagelib->startPage($sql, $infoarr);

$mainlib = new objtab_expli_Gui();
 
echo "<ul>";
echo '<span style="font-size:1.2em; font-weight:bold;">EXPORT all selected objects/elements as ...</span>';
echo '<ul><br />'."\n";

echo "<img src=\"images/greenball.gif\"> <b><a href=\"glob.objtab.exp.php?t=".$tablename.
	"&format=xlsx\">Excel-file</a> </b>(Fastlink to produce *.xlsx (Excel) ... supported since May 2020)<br>";

		
//echo "<img src=\"images/greenball.gif\"> <a href=\"glob.objtab.expoCsv.php?t=".$tablename.
//		"\">special column CSV file</a><br>\n";
echo "<br />\n";

$mainlib->expform($tablename);
echo "<br>";

$role_right_name = "f.PaXml_export";
$role_right      = role_check($sql, $role_right_name);
$do_export = 1;

echo "<img src=\"images/greenball.gif\"> ";
if (!$role_right["execute"] && ($_SESSION['sec']['appuser']!="root")) {
	echo "<font color=gray>Paxml export: Info: you must have role right '$role_right_name'.</font><br>\n";
    $do_export=0;
}

if ( $do_export ) {
    $action_text = "Paxml export";      
    $tmp_info = $headarr["obj_cnt"];    
        
    if ( $tmp_info <= 0 ) {
       echo " $action_text: you must select objects.<br>\n";
    } else {
        echo "<a href=\"impexp/partisanxml/export.bulk.php?cct_table=".$tablename."&wheremem=1\">$action_text</a><br>\n"; 
    }
}

if ( glob_isAdmin() )  {
    echo "<img src=\"images/greenball.gif\"> <a href=\"rootsubs/glob.objtab.export.php?tablename=".$tablename. "&longsql=1\">pure SQL code</a><br>\n";
}  
echo "<img src=\"images/greenball.gif\"> <a href=\"glob.objtab.sattach_exp.php?tablename=".$tablename. "&longsql=1\">Attachments</a>".
	" <img src=\"images/icon.SATTACH.gif\"><br>\n";

echo "</ul>\n";
echo "<br />";
echo "<font size=+1><B>IMPORT objects/elements ...</B></font><ul><br>\n";

$tmpname = "CSV or Excel file";
$mainlib->linkOut( 'glob.objtab.import.php?tablename='.$tablename.'&parx[action]=insert', '<b>Insert</b> objects from '.$tmpname);
$mainlib->linkOut( 'glob.objtab.import.php?tablename='.$tablename.'&parx[action]=update', '<b>Update</b> existing objects from '.$tmpname);

$isActive=1;
if ( !glob_isAdmin() ) $isActive=0;
$mainlib->linkOut( 'glob.objtab.import2.php?tablename='.$tablename.'&parx[type]=insert', $tmpname.' II</a> (admin INSERT)', $isActive );
$mainlib->linkOut( 'glob.objtab.import2.php?tablename='.$tablename.'&parx[type]=move', $tmpname.' II</a> (admin move)', $isActive );

echo "<br>\n";
  

echo "</ul>";
echo "</ul>\n";
$pagelib->htmlFoot();
