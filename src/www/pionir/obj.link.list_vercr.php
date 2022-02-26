<?php
/**
 * activate version control
 * @package obj.link.list_vercr.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   int $go
 */
session_start(); 


require_once ("reqnormal.inc"); // includes all normal *.inc files
require_once ("sql_query_dyn.inc");
require_once ("f.visu_list.inc");
require_once("o.LINK.versctrl.inc");

class oLINK_versActivList{

function oLINK_versActivList() {
	$this->versionObj = new oLINKversC();
}

function form0(&$sql, $sql2, $sqlAfter) {
	require_once ('func_form.inc');
	
	
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Activate version control";
	$initarr["submittitle"] = "Activate!";
	$initarr["tabwidth"]    = "AUTO";

	$hiddenarr = NULL;

	$formobj = new formc($initarr, $hiddenarr, 0);

	$formobj->close( TRUE );
	
	$this->docsActivate ($sql, $sql2, $sqlAfter, 0);
}

function form1(&$sql, &$sql2, $sqlAfter) {
	$this->docsActivate ($sql, $sql2, $sqlAfter, 1);
}

function docsActivate (&$sql, &$sql2, $sqlAfter, $go) {
	// analyze and activate
	global $error;
	
	$sqlsLoop = "SELECT x.LINK_ID, x.NAME FROM ".$sqlAfter;
	$sql2->query($sqlsLoop);
	$wascnt= 0;
	$cnt   = 0;
	while ( $sql2->ReadRow() ) {
	
		$nowerror = "";
		$tmpid   = $sql2->RowData[0];
		$tmpname = $sql2->RowData[1];
		
		if ( $this->versionObj->isUnderControl($sql, $tmpid) ) {
			// already under control ...
			$wascnt++;
		} else {
			
			do {
				$o_rights = access_check($sql, "LINK", $tmpid);
				if ( !$o_rights["write"]) {
					$nowerror = "no write permission on this document!";
					break;
				}
				
				$pathori  = linkpath_get( $tmpid );
			
				if ( !file_exists($pathori) ) {
					$nowerror = "has no file attachment!";
					break;
				}
	
				if ( $go ) {
					$dummy = NULL;
					$this->versionObj->create( $sql, $tmpid, $dummy, 1);
					if ($error->Got(READONLY))  {
						$error->set("form1", 1, "Activation of document [ID:$tmpid] failed.");
						return;
					}
				}
			} while (0);
		}
		
		if ($nowerror!="") echo "<font color=red><b>Error:</b></font> [ID:$tmpid] '$tmpname' : ".$nowerror."<br>\n"; 
		$cnt++;
	}
	echo "<br>";
	echo "Docs analyzed: <b>$cnt</b><br>";
	echo "Already under control: <b>$wascnt</b><br>";
}

}

// --------------------------------------------------- 
global $error, $varcol;

$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] );
$sql2  = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();
$tablename="LINK";

$go = $_REQUEST["go"];

$title       = "Activate version control for selected documents";

#$infoarr['help_url'] = 'o.EXAMPLE.htm';
$infoarr=array();
$infoarr["title"] = $title;
$infoarr["form_type"]= "list";
$infoarr["obj_name"] = $tablename;
$infoarr["obj_cnt"]  = 1;  
 

$pagelib = new gHtmlHead();
$headarr = $pagelib->startPage($sql, $infoarr);
echo "<ul>";

// ----------------------------------------------------------------
// SPECIALS for LIST SELECTION
// ----------------------------------------------------------------
$listVisuObj = new visu_listC();
$tablenice   = tablename_nice2($tablename);
$sqlopt=array();
$sqlopt["order"] = 1;
$sqlAfter  = get_selection_as_sql( $tablename, $sqlopt);

// check TABLE selection
$copt = array ("elemNum" => $headarr["obj_cnt"] ); // prevent double SQL counting
list ($stopFlag, $stopReason)= $listVisuObj->checkSelection( $sql, $tablename, $copt );
if ( $stopFlag<0 ) {
    htmlFoot("Attention", $stopReason." Please select elements of '".$tablenice."'!");
}

$activeObj = new oLINK_versActivList();

if ( !$activeObj->versionObj->isPossible() ) {
	 htmlFoot("Attention", "No version system implemented. Please ask your admin!");
}

echo "<B><font size=+1 color=#606060>";
if ( !$go )   echo "1. Prepare Activation of version control";
if ( $go==1 ) echo "2. Activate version control ";
echo "</font>";
echo "</b><br><br>\n";

if ( !$go ) {
	$activeObj->form0($sql, $sql2, $sqlAfter);
}

if ( $go ) {
	$activeObj->form1($sql, $sql2, $sqlAfter);
}

$error->printAll();
htmlFoot("<hr>");

