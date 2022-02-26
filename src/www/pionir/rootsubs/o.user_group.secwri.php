<?php
/*MODULE: o.user_group.secwri.php
  DESCR:  User-group related for 'security_write' changes
  AUTHOR: qbi
  VERSION: 0.1 - 20051010
*/

extract($_REQUEST); 
session_start(); 


require_once ("reqnormal.inc"); // includes all normal *.inc files
require_once ("visufuncs.inc");
require_once ("insert.inc");
require_once ("access_mani.inc");


// --------------------------------------------------- 
global $error, $varcol;

function this_headout($coltxt, $opter) {
	echo "<font color=#000050 size=+2>". $coltxt."</font>".$opter;
	echo "</b><br><br>\n";
}

function this_form1( $go ) {
	require_once ('func_form.inc');
	
	
	switch ( $go ) {
		case 0:
			$headtxt = "Create single user groups";
			break;
		case 1:
			$headtxt = "Add cct_access_rights entries for new groups";
			break;
	}
	
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = $headtxt;
	$initarr["submittitle"] = "Next &gt;&gt;";
	$initarr["tabwidth"]    = "AUTO";

	$hiddenarr = NULL;
	$hiddenarr["tablename"]     = $tablename;

	$formobj = new formc($initarr, $hiddenarr, $go);


	$formobj->close( TRUE );
}

function this_go1grpcrea( &$sql, &$sql2 ) {

	$sql2->query("select db_user_id, nick from DB_USER where LOGIN_DENY=0 order by nick");
	$cnt=0;
	$addcnt=0;
	while ($sql2->ReadRow()) {
		$dbuserid 	 = $sql2->RowData[0];
		$dbuser_nick = $sql2->RowData[1];
		
		$sql->query("select 1 from USER_GROUP where name='".$dbuser_nick."'");
		if ( !$sql->ReadRow() ) {
			// group not exists
			$argu = NULL;
			$argu["NAME"] = $dbuser_nick;
			$argu["SINGLE_USER"] = 1;
			$argu["DB_USER_ID"]  = $dbuserid;
			$dbgrp_id = insert_row($sql, "USER_GROUP", $argu);
			
			if (!$dbgrp_id) {
				echo "<font color=red>Error:</font> error during creation of group for user '$dbuser_nick'<br>";
				break;
			}
			$argux = NULL;
			$argux["USER_GROUP_ID"] = $dbgrp_id;
			$argux["DB_USER_ID"]    = $dbuserid;
			$retval = insert_row($sql, "DB_USER_IN_GROUP", $argux);
			
			$addcnt++;
			
		} else {
 			echo "<font color=#606000>WARNING:</font> group '$dbuser_nick' already exists.<br>\n";
		}
		$cnt++;
	}
	echo "<b>$addcnt</b> groups created.";
}

function this_acc_write(
	&$sql, 
	$cct_access_id, 
	$group_id, 
	$colstr,
	$valstr
	) {
	global $error;
	
	$colstr = "cct_access_id, user_group_id ".$colstr;
	$valstr = "$cct_access_id, $group_id ". $valstr;
	
	$sql->Query("select 1 from cct_access_rights WHERE cct_access_id = $cct_access_id AND user_group_id = $group_id");	
	if ( !$sql->ReadRow() ) {
		$retval = $sql->Insertx ( "CCT_ACCESS_RIGHTS" , $colstr, $valstr);
		if ($error->got(READONLY)) return;	
		return 1;
	}
}

function this_cctaccess_do(&$sql, &$sql2, $sqls, &$grouparr) {

	$transArr = array(
		"read"  => "select",
		"write" => "update",
		"insert"=> "insert",
		"delete"=> "delete",
		"entail"=> "entail"
		);
	$cnt    = 0;
	$addcnt = 0;
	$rightsin = NULL;
	$cnt10  = 0;
	$rights   = access_getInsRights();
	
	while (@ob_end_flush()); // send all buffered output
				
	$cols="";
	$vals="";
	// prepare SQL-String
	
	foreach( $rights as $key=>$val) {
		$cols .= ", ". $key."_right";
		$vals .= ", 1";
	}
	reset ($rights); 
	
	$sql2->query($sqls);
	
	while ($sql2->ReadRow()) {
		$cnt++;
		$acc_id 	 = $sql2->RowData[0];
		$acc_dbuser  = $sql2->RowData[1];
		$acc_tab     = $sql2->RowData[2];
		
		$newgrpid = $grouparr[$acc_dbuser];
		if ( $newgrpid>0 ) {
			$accAdded = this_acc_write($sql, $acc_id, $newgrpid, $cols, $vals);
			// access_write($sql, $acc_id, $rightsin, $newgrpid);
			if ($accAdded) $addcnt++;
		}
		
		
		if ((float)$cnt*0.001 == (int)((float)$cnt*0.001)) {
			echo "*";
			ob_end_flush ();
			$cnt10++;
			if ($cnt10>10) {
				echo " $cnt<br>";
				while (@ob_end_flush()); // send all buffered output
				$cnt10=0;
			}
		}
		
	}
	echo "<br>";

	$header = NULL;
	$infox = array( "Analysed entries"=> $cnt, "Added entries"=>$addcnt);
	$optxc = array( "showid" => 1);
	visufuncs::table_out( $header,  $infox, $optxc);
	echo "<br>";
}

function this_mainout($text) {
	echo "<b>".$text."</b><br>";
}

function this_go2cctaccess(&$sql, &$sql2, $dateunxProto) {

	// get all groups
	$grouparr = NULL;
	$sql2->query("select db_user_id, nick from DB_USER where LOGIN_DENY=0 order by nick");
	$cnt	=0;
	this_mainout( "--- Start time: ".date("h:m")." ...");
	this_mainout( "--- Select single user groups ...");
	while ($sql2->ReadRow()) {
	
		$dbuserid 	 = $sql2->RowData[0];
		$dbuser_nick = $sql2->RowData[1];
		
		if ($dbuser_nick== "root") continue; // no need for root ... 
		
		$sql->query("select USER_GROUP_ID from USER_GROUP where SINGLE_USER=1 AND NAME='".$dbuser_nick."'");
		$sql->ReadRow();
		$grp_id = $sql->RowData[0];
		
		if ($grp_id) {
			$grouparr[$dbuserid] = $grp_id;
		} else {
			echo "- Info: user '<B>$dbuser_nick</B>': no single user group.<br>";
		}
	}
	echo "<br>";
	this_mainout( "--- ".sizeof($grouparr)." single user groups detected");
	if ( !sizeof($grouparr) ) {
		 htmlErrorBox("Error", "No single user groups found", "Please create USER_GROUPs with SINGLE_USER=1."  );
		 return;
	}
	ob_end_flush ( );

	if ( $_SESSION["userGlob"]["g.debugLevel"]>0 ) {
		$tmpDebug = "where cct_access_id<400";
	}
	#$tmpDebug2 = " AND cct_access_id>516301";
	#$tmpDebug = "where cct_access_id>500000 AND cct_access_id<501000";
	#$tmpDebug2 = " AND cct_access_id>500000 AND cct_access_id<501000";
	if ($tmpDebug!="") {
		this_mainout( "--- DEBUG-condition: ".$tmpDebug."");
	} 
	
	
	this_mainout( "--- Add cct_access_right entries ...");
	// FUNCTION: add ONE entry per BO into CCT_ACCESS_RIGHTS for new groups
	$sqls ="select cct_access_id, DB_USER_ID, table_name, crea_date from cct_access ".$tmpDebug." order by cct_access_id";
	this_cctaccess_do($sql, $sql2, $sqls, $grouparr);

	
	this_mainout( "--- End time: ".date("h:m")." ...");
	// specials for protocols ???
		
}

$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] );
$sql2  = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();


$title       = "Admin: transform to ObjectHiProtectPolicy : convert groups/rights";

$infoarr['help_url'] = 'access_info.html';
$infoarr["title"]    = $title;
$infoarr["title_sh"] = 'transform to ObjectHiProtectPolicy';
$infoarr["form_type"]= "tool";
$infoarr["locrow"]= array( array('rootFuncs.php', 'rootFuncs' ) );


if ( !$go ) $go=0;
$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);
ob_end_flush ();
echo "<ul>";
$dateunxProto = time() - 3600 * 24 * 363 * 1; // 2 years before
$date_human =  date ("Y-m-d", $dateunxProto);

if ( !$go) {
	echo "<B>This tool updates database data, to activate the 'ObjectHiProtectPolicy' (OHIPPO) mode.</B><br><br>";
	echo "1. Check the system for 'security_write' features<br>";
	echo "2. Insert personal groups for all users (only active onces, not 'root')<br>";
	echo "3. For all entries in CCT_ACCESS: Add one CCT_ACCESS_RIGHT for the personal group of user X to CCT_ACCESS entry with owner X<br>";
	echo "&nbsp;&nbsp;&nbsp;&nbsp;- remove all rights from protocols (concrete) which were created before $date_human.<br>";
	echo "INFO: DEBUG-mode possible for function this_go2cctaccess()<br>";
	echo "<br>";
}

$opter = " [<a href=\"".$_SERVER['PHP_SELF']."\">Start again</a>]";
switch ( $go ) {
	case 0:
		$headtxt = "1. Prepare DB changes";
		$opter = "";
		break;
	case 1:
		$headtxt = "2. Create single user groups";
		break;
	case 2:
		$headtxt = "3. Add cct_access_rights entries for new groups";
		break;
}
this_headout($headtxt, $opter);


if ( $_SESSION['sec']['appuser']!="root" ) {
	echo "Sorry, you must be root.";
	return 0;
}

$infox[] = array("security_write",$_SESSION['globals']["security_write"],"");


$sql->query("select count(1) from DB_USER");
$sql->ReadRow();
$infox[] = array("Number of users", $sql->RowData[0],"");

$sql->query("select count(1) from USER_GROUP where SINGLE_USER=1");
$sql->ReadRow();
$infox[] = array("Number of single user groups", $sql->RowData[0],"");

$sql->query("select count(1) from cct_access");
$sql->ReadRow();
$acccnt = $sql->RowData[0];
$minuteFact = 60.0/250000.0; // fact = 1h / 250000 accesses
$timeneed = (int)($acccnt * $minuteFact);
$infox[] = array("Number of CCT_ACCESS", $acccnt, "Approximated conversion time: <B>".$timeneed. "</B> minutes");

$header = array("What", "Number", "Notes");
$optxc  = array( );
visufuncs::table_out( 
		$header,  // can be NULL
		$infox,   // array of values or just the value
     	$optxc);
echo "<br>";


if (!$go) {
	this_form1( $go );
	htmlFoot("<hr>");
}

if ( $go==1 ) {
	this_go1grpcrea($sql, $sql2);
	this_form1( $go );
	htmlFoot("<hr>");
}

if ( $go==2 ) {
	this_go2cctaccess($sql, $sql2, $dateunxProto);
	htmlFoot("<hr>");
}



htmlFoot();


