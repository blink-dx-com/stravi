<?php
/**
 * delete access for a selection of objects for a selection of groups
 * @package glob.objtab.accdel.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   $tablename
  		  $grpid[group_id] = 1
		  $groupall = 0,1 (if 1 : delete all access rights
		  $go 0,1
 * @version0 2005-10-18
 */
session_start(); 


require_once ('reqnormal.inc');
require_once("sql_query_dyn.inc");
require_once("access_mani.inc");
require_once("f.visu_list.inc");
require_once ("visufuncs.inc");

function this_form($tablename, $grpid, $groupall) {
	require_once ('func_form.inc');
	
	
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "<img src=\"images/ic.del.gif\">&nbsp;&nbsp; Accept action&nbsp;&nbsp;&nbsp;&nbsp;";
	$initarr["submittitle"] = "Remove now";
	$initarr["tabwidth"]    = "AUTO";

	$hiddenarr = NULL;
	$hiddenarr["tablename"]     = $tablename;
	
	if ($groupall) {
		$hiddenarr["groupall"] = 1;
	} else {
		foreach( $grpid as $tGroupid=>$val) {
			$hiddenarr["grpid[".$tGroupid."]"] = 1;
		}
		reset ($grpid); 
	}
	$formobj = new formc($initarr, $hiddenarr, 0);

	$formobj->close( TRUE );
}

global $error, $varcol;

$error      = & ErrorHandler::get();
$sql        = logon2( $_SERVER['PHP_SELF'] );
$sql2       = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot();
$varcol     = & Varcols::get();

$tablename=$_REQUEST['tablename'];
$go=$_REQUEST['go'];
$grpid=$_REQUEST['grpid'];
$groupall=$_REQUEST['groupall'];

$grouptable = "USER_GROUP";
$infox      = NULL;
$listVisuObj= new visu_listC();
$tablenice = tablename_nice2($tablename);

$title       = "Delete group rights for selected objects";
$title_sh    = "Delete group rights";
$infoarr=array();
$infoarr['help_url'] = 'access_info.html';
$infoarr["title"] 	 = $title;
$infoarr["title_sh"] = $title_sh;
$infoarr["form_type"]= "list";
$infoarr["obj_name"] = $tablename;
$infoarr["obj_cnt"]  = 1;          // show number of objects
$infoarr["locrow"]   = array( array("glob.objtab.access.php?t=".$tablename, "AccessInfo") );


$pagelib = new gHtmlHead();
$headarr = $pagelib->startPage($sql, $infoarr);
$primary_key = PrimNameGet2($tablename);
echo "<ul>\n";

if ($tablename=="") 			  htmlFoot("ERROR", "No table selected");
if (!cct_access_has2($tablename)) htmlFoot("ERROR", "Objects are not business objects!");

// $this->AccHelperLib = new fAccessHelper();

$infox["groupNum"] = sizeof($grpid);
if (!$infox["groupNum"] AND !$groupall) {
	 htmlFoot("Error", " No groups selected");
}
$sqlopt=array();
$sqlopt["order"] = 1;
$sqlAfter  = get_selection_as_sql( $tablename, $sqlopt);

// check TABLE selection
$copt = array ("elemNum" => $headarr["obj_cnt"] ); // prevent double SQL counting
list ($stopFlag, $stopReason)= $listVisuObj->checkSelection( $sql, $tablename, $copt );
if ( $stopFlag<0 ) {
    htmlFoot("Attention", $stopReason." Please select elements of '".$tablenice."'!");
}


$tmpkomma = "";
if ($groupall>0) {
	echo "<br>";
	htmlInfoBox( "Warning", "This tool removes <b>ALL</b> group-access-entries from the selected objects.", "", "WARN" );

} else {
	echo "<font color=gray>Delete rights for groups <img src=\"images/icon.USER_GROUP.gif\">: &nbsp;&nbsp;</font>";
	foreach( $grpid as $tGroupid=>$val) {
		$tname = obj_nice_name    ( $sql, $grouptable, $tGroupid);
		echo $tmpkomma."<b>".$tname."</B>";
		$tmpkomma = ", ";
	}
	reset ($grpid); 
}
echo "<br>\n";


echo "<br><B><font size=+1 color=#606060>";
if ( !$go )   echo "1. Prepare remove";
if ( $go==1 ) echo "2. Do remove";
echo "</font></b><br><br>\n";

if ( !$go ) {
	this_form($tablename, $grpid, $groupall);
	htmlFoot();
}


// select objects, than get groups
$infox["ok_cnt"]   = 0;
$infox["deny_cnt"] = 0;

$sqlsLoop = "SELECT x.".$primary_key.", x.cct_access_id FROM ".$sqlAfter;
$sql2->query($sqlsLoop);
$loopcount = 0;
$showStepShow  = 100;
if ($headarr["obj_cnt"]>2000)  $showStepShow  = 500;
$showStepShowF = 1.0 / $showStepShow;
$showStepShowBr = 0;
	
while ( $sql2->ReadRow() ) {

    $tobjid = $sql2->RowData[0];
	$taccid = $sql2->RowData[1];
	$change_allow = access__allowed_to_chmod($sql, $taccid, $tablename);
	
	if ($change_allow=="entail" OR $change_allow=="yes" ) {
		if ($go) {
			if ($groupall>0) {
				// dell-all
				access_delAll( $sql, $taccid);
			} else {
				foreach( $grpid as $tGroupid=>$val) {
					access_del_group( $sql, $taccid, $tGroupid );
				}
				reset ($grpid); 
			}
			$infox["ok_cnt"]++;
		}
	} else {
		$infox["deny_cnt"]++;
	}
	
	// page output for long process
	if ( round($loopcount * $showStepShowF) == ($loopcount * $showStepShowF) ) {
		echo "*";  
		$showStepShowBr++;
		if ( $showStepShowBr>20 ) {
			$showStepShowBr = 0;
			echo "<br>\n";
			ob_end_flush ();
		} 
	}
	$loopcount++;
}

echo "<br><font color=green>... ready</font>\n";
echo "<br><br>\n";

$header = NULL;
$tmparr = NULL;
$tmparr[] = array("Analysed objects" , $loopcount);
$tmparr[] = array("Accepted objects" , $infox["ok_cnt"]);
if ($infox["deny_cnt"]>0) $tmparr[] = array("Denied objects" , "<font color=red>".$infox["deny_cnt"]."</font>");

$opt = array("title"  => "Statistics");
visufuncs::table_out( $header,  $tmparr,  	$opt );

htmlFoot("<hr>");
