<?php
/**
 *  Switch ROID/WIID of a business object ON or OFF according to a 
 * @package ROID-flip.php
 * @swreq UREQ:xxxxxxxxxxxx
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com), Piet, CCT Jena
 * @param   $proj_id
 #        $go
 #  $_FILES['mapping_file']
 * @version $Header: trunk/src/www/pionir/rootsubs/ROID-flip.php 59 2018-11-21 09:04:09Z $
 */

extract($_REQUEST); 
session_start(); 


## Public libraries:
require_once('db_access.inc');
require_once('globals.inc');
require_once('func_head.inc');
require_once('access_check.inc');
require_once("object.info.inc");
require_once ('f.update.inc');


// Alias for Macs cursor:
$cursorObj1 = logon2( $_SERVER['PHP_SELF'] );
$cursorObj2 = logon2( $_SERVER['PHP_SELF'] );

// Global variables:
$testFlag  = 0;
$pageTitle = 'Set/remove ROID and WIID according to mapping file';

// User settings:
// $mapping_file = "ROID-mapping-cftr.csv";
$BO_new_user    = "substances";


// CGI variables:
if ( ! isset($action) or ! strlen($action) ) { $action = 'read only'; }
if ( ! isset($mapping_file) )                { $mapping_file = ""; }
if ( ! isset($go) ) $go=0;

if ( $testFlag ) 
{
  error_reporting (E_ALL);
}//End if



$title = $pageTitle;

$infoarr["title"] = $title;
$infoarr["form_type"]= "tool";

$infoarr["locrow"]= array( array('index.php', 'Administration' ) );

$pagelib = new gHtmlHead();
$pagelib->startPage($cursorObj1, $infoarr);

$infoObj = new objInfoC();
////////////////////////////////////////////////////////////////////////
////////////////   Session and user authentication

if ( $_SESSION['sec']['appuser'] != "root" ) { // !$_SESSION['s_suflag']
     htmlErrorBox( "Error",
     "Only root can execute this!",
     "For security reason it is not allowed for common users" );
     htmlFoot();
}


////# Connect to database (default database of the local session):

$varcol = & Varcols::get();




////////////////////////////////////////////////////////////////////////

// Read DB_USER mapping:
$sqls = "select * from db_user";
$cursorObj1->query($sqls);

$DB_USER_hash = array();
while( $cursorObj1->ReadArray() )
{ 
  $id   = $cursorObj1->RowData['DB_USER_ID'];
  $nick = $cursorObj1->RowData['NICK'];

  $DB_USER_hash['NICK'][$id]         = $nick;
  $DB_USER_hash['DB_USER_ID'][$nick] = $id;
}

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
// Upload mapping file if no filename was given as CGI parameter:
if (  !$go )
{ // Print from: ?>
	<ul>
	<form action="<?$_SERVER['PHP_SELF']?>"  method="POST" enctype="multipart/form-data">
	<input type="hidden" name="go" value="1">
	<?
	if ($mapping_file == '') {
		?>
		<p>Upload mapping file:<p>
		<input type="hidden" name="action" value="read only">
		<input type="file"   value="" SIZE=54 MAXLENGTH=10000 name="mapping_file" accept="text/*"><br>

		<?
	}
	echo '<input type="submit" name="Submit" value="  Start  ">';
	echo '</form></ul>';

	htmlFoot();
}//End if

// go==1
// Move uploaded file if any:
  if( isset($_FILES['mapping_file']) && is_uploaded_file($_FILES['mapping_file']['tmp_name']) )
  { $mapping_file = "/tmp/ROIDmap-uploaded.".session_id().".csv";
    copy($_FILES['mapping_file']['tmp_name'], $mapping_file);
  }//End if


  // Print variable setting:
  print "<pre>\n";
  print "Parameters:\n";
  print "  Magasin serial of local database: ".$_SESSION['globals']['magasin_serial']."\n";
  print "  Mapping file:   '".realpath($mapping_file)."'\n";
  print "  Kind of action: '".$action."'\n";


  // Read list of ABSTRACT_SUBST_ID from file:

  $FH = fopen($mapping_file,"r");
  if ( !$FH ) {
  	htmlFoot("Error", "Cannot open file '$mapping_file' for reading.\n Program exited.\n");
  }

  print "\n"; 
  print "Reading ROID mapping from file:\n"; 
  print "  "; system("ls -ld $mapping_file");
  if ( is_link($mapping_file) ) { print "  "; system("ls -ld ".realpath($mapping_file)); }
  print "\n";

  $lineNum = 0;
  $datalines            = array();
  $magasin_serial_count = array();
  while( !feof ($FH) )
  { 
    $line = fgets($FH, 16384); // Get line of length
    if ( $line )
    { 
      $lineNum++;

	  // Remove newline and CR, but NO tab or blanks:
	  // (\n  newline, \r  carriage return)
	  $line = preg_replace("/[\r\n]*$/",'',$line);

	  // first line is header:
	  if ( $lineNum == 1 )
      { $columnNames = explode("\t", $line);
	    $columnNum   = sizeof($columnNames);
        continue;
      }//End if

	  $datafields   = explode("\t", $line);
	  $datafieldNum = sizeof($datafields);
	  if ( $datafieldNum != $columnNum )
	  { print "Error:  Line $lineNum has inappropriate number of data fields.\n";
	    exit; 
	  }//End if

      $dataArray = array();
      for( $i=0;$i<$columnNum;$i++ )
      { $dataArray[$columnNames[$i]] = $datafields[$i]; }

      // Parse WIID_NAME:
      preg_match('/^http:\/\/www\.clondiag\.com\/magasin\/\?db=(\d+)$/',$dataArray['WIID_NAME'],$matches);
      if ( isset($matches[1]) ) { $dataArray['WIID_serial'] = (int) $matches[1]; }
      else                      { $dataArray['WIID_serial'] = 0; }

      $datalines[] = $dataArray;
      if ( ! isset($magasin_serial_count[$dataArray['magasin_serial']]) ) { $magasin_serial_count[$dataArray['magasin_serial']] = 0;};
      $magasin_serial_count[$dataArray['magasin_serial']]++;

    }//End if
  }//End while

  print(sizeof($datalines)." business object mappings read from file.\n");

  // Check magasin serials found in file.
  // Restraints:  All magasin serial in file must be the same.
  $magasin_serial_error = '';
  if( sizeof($magasin_serial_count) == 1 )
  { foreach( $magasin_serial_count as $magasin_serial => $count ) { /* do nothing */; }

    if( ! is_int($magasin_serial) )
    { $magasin_serial_error = "No valid magasin_serial found."; }
  }//End if
  elseif( sizeof($magasin_serial_count) > 1 )
  { $magasin_serial_error = "Several different magasin_serial found in file."; }
  else
  { $magasin_serial_error = "No valid magasin_serial found";  }


  if ( $magasin_serial_error )
  { print "ERROR:  $magasin_serial_error\n";
    print_r($magasin_serial_count);
    print "Program exited.\n";
    exit;
  }//End if

  print "\nROID mapping file was generated on local database (db=$magasin_serial).\n";
  print "</pre><hr noshade size=\"1\">\n";

  ////////////////////////////////////////////////////////////////////////
  // Analyze mapping read from file.
  // Case 1:   file was written on local database
  if ( $magasin_serial == $_SESSION['globals']['magasin_serial'] )
  { print "<p>List of BO in local database which originate from foreign database:\n";

    print "<table>\n";
    print "<tr bgcolor=\"#cccccc\">";
    print "<td>TABLE_NAME</td>";
    print "<td>ID</td>";
    print "<td>NAME</td>";
    print "<td>user</td>";
    print "<td>local&nbsp;ROID</td>";
    print "<td>local&nbsp;WIID_NAME</td>";
    print "<td>file&nbsp;ROID</td>";
    print "<td>file&nbsp;WIID_serial</td>";
    print "</tr>\n";

    $BO_foreign_count = 0;
    foreach( $datalines as $index => $subhash )
    { // Skip business objects which have no foreign WIID_serial:
      
	  if ( $subhash['WIID_serial'] == 0 ) { continue; }
      if ( $subhash['WIID_serial'] == $_SESSION['globals']['magasin_serial'] ) { continue; }

	  $bo_hash = $infoObj-> getParamsRaw ( $cursorObj1, $subhash['TABLE_NAME'],$subhash['ID'] );

      // Update BO if action is specified:
      if ( $action == "setROID" )
      {
        $bo_hash['access']['DB_USER_ID'] = $DB_USER_hash['DB_USER_ID']['root'];
        $bo_hash['access']['ROID']       = $subhash['ROID'];
        $bo_hash['access']['WIID']       = $H_WIID_hash['WIID'][$subhash['WIID_NAME']];

      }//End if
      elseif ( $action == "removeROID" )
      {

        $bo_hash['access']['DB_USER_ID'] = $DB_USER_hash['DB_USER_ID'][$BO_new_user];
        $bo_hash['access']['ROID']       = 'NULL';
        $bo_hash['access']['WIID']       = 'NULL';

      }//End elseif

	  if ( $action =="setROID" OR  $action =="removeROID" ) {

	  		$tmpval["CCT_ACCESS_ID"] = $bo_hash["vals"]["CCT_ACCESS_ID"];
			$tmpval["DB_USER_ID"] = $bo_hash['access']['DB_USER_ID'];
			$tmpval["ROID"] 	  = $bo_hash['access']['ROID'];
			$tmpval["WIID"]       = $bo_hash['access']['WIID'];
			gObjUpdate::update_row( $cursorObj2 , "CCT_ACCESS", $tmpval );
	  }

      $BO_foreign_count++;

      if ( $bo_hash['access']['ROID'] )
      { $ROID_local      = $bo_hash['access']['ROID'];
        $WIID_NAME_local = $H_WIID_hash['NAME'][$bo_hash['access']['WIID']];
      }//End else
      else
      { $ROID_local      = "<br>";
        $WIID_NAME_local = "<br>";
      }//End else

      print "<tr bgcolor=\"#eeeeee\">";
      print "<td>".$subhash['TABLE_NAME']."</td>";
      print "<td>".$subhash['ID']."</td>";
      print "<td>".$bo_hash['vals']['NAME']."</td>";
      print "<td>".$DB_USER_hash['NICK'][$bo_hash['access']['DB_USER_ID']]."&nbsp;[".$bo_hash['access']['DB_USER_ID']."]</td>";
      print "<td>$ROID_local</td>";
      print "<td>$WIID_NAME_local</td>";
      print "<td>".$subhash['ROID']."</td>";
      print "<td>".$subhash['WIID_serial']."</td>";
      print "</tr>\n";

    }//End foreach

    print "</table>\n";
    print "<p>Number of foreign BO in file:&nbsp;&nbsp;$BO_foreign_count";

    print "<p><table><tr bgcolor=\"#e6e6e6\">";
    print "<td valign=\"baseline\"><b>Action:&nbsp;&nbsp;</b></td>";
  
    print "<td><a href=\"".$_SERVER['PHP_SELF']."?action=setROID&mapping_file=$mapping_file&go=1\">Set local ROID to foreign ROID</a>";
    print "<br>new user:&nbsp;&nbsp;root&nbsp;[".$DB_USER_hash['DB_USER_ID']['root']."]</td>";

    print "<td><a href=\"".$_SERVER['PHP_SELF']."?action=removeROID&mapping_file=$mapping_file&go=1\">Remove local ROID</a>";
    print "<br>new user:&nbsp;&nbsp;$BO_new_user&nbsp;[".$DB_USER_hash['DB_USER_ID'][$BO_new_user]."]</td>";
    print "</tr></table>";

  }//End if


  // Analyze mapping read from file.
  // Case 2:   file was created on foreign database
  elseif( $magasin_serial != $_SESSION['globals']['magasin_serial'] )
  { print "<p>List of BO in foreign database which originate from local database:";

    print "<table>\n";
    print "<tr bgcolor=\"#cccccc\">";
    print "<td>TABLE_NAME</td>";
    print "<td>local&nbsp;ID</td>";
    print "<td>NAME</td>";
    print "<td>user</td>";
    print "<td>local&nbsp;ROID</td>";
    print "<td>local&nbsp;WIID_NAME</td>";
    print "<td>foreign&nbsp;ROID</td>";
    print "</tr>\n";

    // Check if $WIID_NAME_foreign exists in H_WIID:
    $WIID_NAME_foreign = "http://www.clondiag.com/magasin/?db=$magasin_serial";
    if ( $action == "setROID" )
    { if ( isset($H_WIID_hash['WIID'][$WIID_NAME_foreign]) )
      { $WIID_foreign = $H_WIID_hash['WIID'][$WIID_NAME_foreign]; }
      else
      { $WIID_foreign = insert_H_WIID($cursorObj1,$WIID_NAME_foreign); }
    }//End if


    $BO_foreign_count = 0;
    foreach( $datalines as $index => $subhash )
    { // Skip business objects which have not been created on local database:
      if ( $subhash['WIID_serial'] != $_SESSION['globals']['magasin_serial'] ) { continue; }

	  $bo_hash = $infoObj-> getParamsRaw ( $cursorObj1, $subhash['TABLE_NAME'], $subhash['ROID']);

      // Update BO if action is specified:
      if ( $action == "setROID" )
      {
        $bo_hash['access']['DB_USER_ID'] = $DB_USER_hash['DB_USER_ID']['root'];
        $bo_hash['access']['ROID']       = $subhash['ID'];
        $bo_hash['access']['WIID']       = $WIID_foreign;

      }//End if
      elseif ( $action == "removeROID" )
      {
        $bo_hash['access']['DB_USER_ID'] = $DB_USER_hash['DB_USER_ID'][$BO_new_user];
        $bo_hash['access']['ROID']       = 'NULL';
        $bo_hash['access']['WIID']       = 'NULL';

      }//End elseif

	  if ( $action =="setROID" OR  $action =="removeROID" ) {

	  		$tmpval["CCT_ACCESS_ID"] = $bo_hash["vals"]["CCT_ACCESS_ID"];
			$tmpval["DB_USER_ID"] = $bo_hash['access']['DB_USER_ID'];
			$tmpval["ROID"] 	  = $bo_hash['access']['ROID'];
			$tmpval["WIID"]       = $bo_hash['access']['WIID'];
			gObjUpdate::update_row( $cursorObj2 , "CCT_ACCESS", $tmpval );
	  }


      $BO_foreign_count++;

      if ( $bo_hash['access']['ROID'] )
      { $ROID_local      = $bo_hash['access']['ROID'];
        $WIID_NAME_local = $H_WIID_hash['NAME'][$bo_hash['access']['WIID']];
      }//End else
      else
      { $ROID_local      = "<br>";
        $WIID_NAME_local = "<br>";
      }//End else

      print "<tr bgcolor=\"#eeeeee\">";
      print "<td>".$subhash['TABLE_NAME']."</td>";
      print "<td>".$subhash['ROID']."</td>";
      print "<td>".$bo_hash['vals']['NAME']."</td>";
      print "<td>".$DB_USER_hash['NICK'][$bo_hash['access']['DB_USER_ID']]."&nbsp;[".$bo_hash['access']['DB_USER_ID']."]</td>";
      print "<td>$ROID_local</td>";
      print "<td>$WIID_NAME_local</td>";
      print "<td>".$subhash['ID']."</td>";
      print "</tr>\n";

    }//End foreach

    print "</table>\n";
    print "<p>Number of foreign BO in file:&nbsp;&nbsp;$BO_foreign_count";

    print "<p><table><tr bgcolor=\"#e6e6e6\">";
    print "<td valign=\"baseline\"><b>Action:&nbsp;&nbsp;</b></td>";

    print "<td>new local WIID:&nbsp;&nbsp;$WIID_NAME_foreign";
    print "<br><a href=\"".$_SERVER['PHP_SELF']."?action=setROID&mapping_file=$mapping_file&go=1\">Set local ROID to foreign ROID</a>";
    print "<br>new user:&nbsp;&nbsp;root&nbsp;[".$DB_USER_hash['DB_USER_ID']['root']."]</td>";

    print "<td>new local WIID:&nbsp;&nbsp;''";
    print "<br><a href=\"".$_SERVER['PHP_SELF']."?action=removeROID&mapping_file=$mapping_file&go=1\">Remove local ROID</a>";
    print "<br>new user:&nbsp;&nbsp;$BO_new_user&nbsp;[".$DB_USER_hash['DB_USER_ID'][$BO_new_user]."]</td>";
    print "</tr></table>";

  }//End else





////////////////////////////////////////////////////////////////////////
?>
<hr noshade size=1>
<?

// Open html page:
htmlFoot();

// Exit PHP script:
exit;
////////////////////////////////////////////////////////////////////////

////////////// Functions

function insert_H_WIID(&$cursorObj,$NAME)
{ 
	$sqls = "(NAME) values (" . $cursorObj->addQuotes($NAME) . ")";
	return $cursorObj->queryInsert("H_WIID", $sqls,'WIID');
}//End function


////////////////////////////////////////////////////////////////////////
#eof
?>
