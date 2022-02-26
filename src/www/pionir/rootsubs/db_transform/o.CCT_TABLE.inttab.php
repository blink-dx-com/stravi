<?php
/*MODULE:  o.CCT_TABLE.inttab.php
  DESCR:   Repair CCT_TABLE, define INTERNAL tables
  AUTHOR:  qbi
  INPUT:   $id   (e.g. ARRAY_LAYOUT_ID)
  VERSION: 0.1 - 20020904
*/

extract($_REQUEST); 
session_start(); 


require_once ("reqnormal.inc"); // includes all normal *.inc files
require_once ('get_cache.inc');
require_once ('f.update.inc');

class oCCT_TABLEinttab {

function __construct() {
	// array("TABLE" => CNT)
	$this->internalTabs= array(
		"USER_PREF" => 1,
		"T_EXPORT" => 1,
		"SELECT_CHIP_IN_AP" => 1,
		"MODULE_PARAMETER" => 1,
		"MODULE_CONTEXT" => 1,
		"MODULE" => 1,
		"IDV" => 1,
		"ID" => 1,
		"H_VAL_INIT" => 1,
		"EXTRA_OBJ" => 1,
		"CCT_TAB_VIEW" => 1,
		"CCT_ACCESS_RIGHTS" => 1,
		"CCT_ACC_UP" => 1,
		"LINKV" => 1
	);
}

function repair(&$sql, &$sql2) {

	$cnt=0;
	$interncnt=0;
	$sqls = "select TABLE_NAME from CCT_TABLE order by TABLE_NAME";
	$sql2->query($sqls);
	while ( $sql2->ReadRow() ) {
		$tmptab = $sql2->RowData[0];
		if ( $this->internalTabs[$tmptab]>0 ) {
			$internal=1;
			echo "internal: $tmptab<br>";
			$interncnt++;
		} else  $internal=0;
		
		$argu=NULL;
		$argu["TABLE_NAME"]= $tmptab;
		$argu["INTERNAL"]  = $internal;
		gObjUpdate::update_row($sql, "CCT_TABLE", $argu);
		$cnt++;
	}
	
	echo "<br>";
	echo "$cnt tables touched.<br>";
	echo "$interncnt tables are internal now.<br>";
}

}

// --------------------------------------------------- 
global $error, $varcol;

$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] );
$sql2  = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();



$title       = "Repair CCT_TABLE, define INTERNAL tables";


$infoarr["title"] = $title;
$infoarr["form_type"]= "tool";

$infoarr["locrow"]= array( array('index.php', 'back' ) );


$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);
echo "<ul>";

$tablesubsObj = new oCCT_TABLEinttab();

if ( $_SESSION['sec']['appuser'] != "root" ) { // !$_SESSION['s_suflag'] 
     htmlErrorBox( "Error",   
     "Only root can execute this!",
     "For security reason it is not allowed for common users" );
     htmlFoot();
}

if ( !$go ) {
	echo "<form style=\"display:inline;\" method=\"post\"  name=\"editform\"  action=\"".$_SERVER['PHP_SELF']."?go=1\" >\n";
	echo "<input type=submit value=\"Repair now!\">\n";
	echo "</form>";
	htmlFoot();
}

echo "Repair now!<br><br>";
$tablesubsObj->repair($sql, $sql2);

echo 'refreshing cache ...<br>',"\n";
flush();
get_cache(1,0);
$error->reset();

htmlFoot("Ready.");


