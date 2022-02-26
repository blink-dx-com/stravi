<?php
/**
 * Create a ROID mapping file for all business objects in a given project.
 * @package ROID-map.php
 * @author  piet
 * @version 1.0
 * 
 */

extract($_REQUEST); 
session_start(); 


// Public libraries:
require_once('db_access.inc');
require_once('globals.inc');
require_once ('func_head.inc');
require_once ('access_check.inc');
require_once("object.info.inc");


$cursorObj1 =  logon2( $_SERVER['PHP_SELF'] );
$cursorObj2 =  logon2( $_SERVER['PHP_SELF'] );

// Global variables:
$testFlag  = 0;
$pageTitle = 'Create a ROID mapping file for all business objects in current project';

// CGI parameters:
if ( ! isset($PROJ_ID) ) { $PROJ_ID = 0; }

// User settings:
$rel_dirname  = $_SESSION['globals']['http_cache_path'];
$loginPATH    = $_SESSION['s_sessVars']['loginPATH'];


$mappingFile_dir  = realpath($loginPATH . "/" . $rel_dirname);
$mappingFile_name = "ROIDmap-".$_SESSION['sec']['dbuser']."_".$_SESSION['sec']['db']."-PROJ_".$PROJ_ID.".csv";

$mappingFile_URLdir = $_SESSION['s_sessVars']['loginURL'] . "/" . $rel_dirname;
$mappingFile_URL    = "$mappingFile_URLdir/$mappingFile_name";


if ( $testFlag ) 
{ //$ERR = open_logfile();
  error_reporting (E_ALL);
}//End if


$title = $pageTitle;

$infoarr["title"] = $title;
$infoarr["form_type"]= "tool";

$infoarr["locrow"]= array( array('index.php', 'Administration' ) );

$pagelib = new gHtmlHead();
$pagelib->startPage($cursorObj1, $infoarr);


////////////////////////////////////////////////////////////////////////
////////////////   Session and user authentication

if ( $_SESSION['sec']['appuser'] != "root" ) { // !$_SESSION['s_suflag']
     htmlErrorBox( "Error",
     "Only root can execute this!",
     "For security reason it is not allowed for common users" );
     htmlFoot();
}


////# Connect to database (default database of the current session):





////////////////////////////////////////////////////////////////////////

// Read DB_USER mapping:
$sqls = "select * from db_user";
$cursorObj1->query($sqls);

$DB_USER_hash = array();
while( $cursorObj1->ReadArray() )
{ $id   = $cursorObj1->RowData['DB_USER_ID'];
  $nick = $cursorObj1->RowData['NICK'];

  $DB_USER_hash['NICK'][$id]         = $nick;
  $DB_USER_hash['DB_USER_ID'][$nick] = $id;
}//End while

////////////////////////////////////////////////////////////////////////

// Read H_WIID mapping:
$sqls = "select * from h_wiid";
$cursorObj1->query($sqls);

$H_WIID_hash = array();
while( $cursorObj1->ReadArray() )
{ $id   = $cursorObj1->RowData['WIID'];
  $NAME = $cursorObj1->RowData['NAME'];

  $H_WIID_hash['NAME'][$id]   = $NAME;
  $H_WIID_hash['WIID'][$NAME] = $id;
}//End while

////////////////////////////////////////////////////////////////////////
$infoObj = new objInfoC();
// Get NAME of root project:

$sqls = "select NAME from PROJ where PROJ_ID=$PROJ_ID";
$cursorObj1->query($sqls);
$cursorObj1->ReadRow();
$PROJ_NAME = $cursorObj1->RowData[0];

////////////////////////////////////////////////////////////////////////
// Print current settings:

print "<p>Current settings:\n";
print "<pre>\n";
print "Magasin serial of current database: ".$_SESSION['globals']['magasin_serial']."\n";
print "Root project ID:   '".$PROJ_ID."'\n";
print "Root project NAME: '".$PROJ_NAME."'\n";

$filename_abs = "$mappingFile_dir/$mappingFile_name";
print "Mapping file:      '".$filename_abs."'\n";
print "</pre>\n";
print "<hr noshade size=1>\n";

// Open mapping file:
$FH = fopen($filename_abs,"w");
if ( !$FH )
{ 
	htmlFoot("Error","Cannot open file '$filename_abs' for writing. Program exited.");

}//End if

print("ROID mapping file opened for writing.<br>\n"); 

$columnNames = array();
$columnNames[] = 'TABLE_NAME';
$columnNames[] = 'NAME';
$columnNames[] = 'magasin_serial';
$columnNames[] = 'ID';
$columnNames[] = 'ROID';
$columnNames[] = 'WIID_NAME';


// Write table header:
fwrite($FH,join("\t",$columnNames)."\n");



$lineNum   = 0;
$datalines = array();
fwrite($FH,"");


if ( $PROJ_ID < 1 )
{ print "<pre>ERROR:  No valid PROJ_ID supplied.\n";
  print "   PROJ_ID: '$PROJ_ID'\n";
  print "   Program exited.\n";
  exit;
}//End if


// Store mother project in list of bo:
$bo_list = array();
$bo_list[] = array( 'table_name' => 'PROJ', 'prim_key' => $PROJ_ID );


$sqls = "select * from PROJ_HAS_ELEM where PROJ_ID=$PROJ_ID";
$cursorObj1->query($sqls);

$BO_count         = 0;
$BO_skipped_count = 0;
$BO_written_count = 0;
while( $cursorObj1->ReadArray() )
{ // Some aliases for columns in cursor:

  $TABLE_NAME = &$cursorObj1->RowData['TABLE_NAME'];
  $PRIM_KEY   = &$cursorObj1->RowData['PRIM_KEY'];

  $BO_count++;

  // Skip links to other projects (NO recursion):
  if ( $TABLE_NAME == 'PROJ' ) { $BO_skipped_count++; continue; }

  // Store BO identifier in list of hashes:
  $bo_list[] = array( 'table_name' => $TABLE_NAME, 'prim_key' => $PRIM_KEY );

}//End while


foreach( $bo_list as $index => $sub_hash )
{ // Retrieve business object:

  $bo_hash = $infoObj->getParamsRaw ( $cursorObj2, $sub_hash['table_name'], $sub_hash['prim_key'] );

  if( ! $bo_hash['vals']['CCT_ACCESS_ID'] )
  { print "<pre>ERROR:  Failed to retrieve business object.\n";
    print "   table name = '{$sub_hash['table_name']}'\n";
    print "           id = '{$sub_hash['prim_key']}'\n";
    print "Program exited.\n";
    exit;
  }//End if


  // Map access data of business object to columns:
  $dataArray = array();
  $dataArray['TABLE_NAME']     = $sub_hash['table_name'];
  $dataArray['NAME']           = $bo_hash ['vals']['NAME'];
  $dataArray['magasin_serial'] = $_SESSION['globals'] ['magasin_serial'];
  $dataArray['ID']             = $sub_hash['prim_key'];

  if ( $bo_hash['access']['ROID'] )
  { 
  	$dataArray['ROID']      = $bo_hash['access']['ROID'];
    $dataArray['WIID_NAME'] = $H_WIID_hash['NAME'][$bo_hash['access']['WIID']];
  }//End if
  else
  { $dataArray['ROID']      = '';
    $dataArray['WIID_NAME'] = '';
  }//End else


  // Write data line (transform hash into list):
  $dataList = array();
  foreach( $columnNames as $index => $columnName )
  { if ( isset($dataArray[$columnName]) ) { $dataList[] = $dataArray[$columnName]; }
    else								  { $dataList[] = ''; }
  }//End if


  fwrite($FH,join("\t",$dataList)."\n");
  $BO_written_count++;

}//End while


print("  Root project contains $BO_count business objects.<br>\n");
print("  $BO_skipped_count links to projects skipped.<br>\n");
print("  $BO_written_count business objects written to ROID mapping file.<br>\n");
print("ROID mapping file closed.<br>\n");


print "<hr noshade size=1>";
$anker = "<a href=\"$mappingFile_URL\">$mappingFile_name</a>";
print "<p>Download mapping file to your local disk:&nbsp; $anker\n";


print "<hr noshade size=1>";
$anker = "<a href=\"ROID-flip.php?mapping_file=$filename_abs\">Set/remove &nbsp;ROID/WOID&nbsp;</a>";
print "Step&nbsp;2:&nbsp;&nbsp$anker";

////////////////////////////////////////////////////////////////////////
?>
<hr noshade size=1>
<font size=-1>Script name:&nbsp;&nbsp;<? echo $_SERVER['PHP_SELF']; ?></font>
<?

// Open html page:
htmlFoot();

// Exit PHP script:
exit;
////////////////////////////////////////////////////////////////////////
#eof
?>
