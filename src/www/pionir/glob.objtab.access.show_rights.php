<?php
/**
 * show the rights for the selected objects
 * @package glob.objtab.access.show_rights.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param    $tablename
         $group_id
		 [$optall] = 0,1 -- no group_id needed
		
 * @version0 2002-09-19
 */

session_start(); 


require_once ('reqnormal.inc');
require_once("sql_query_dyn.inc");
require_once("access_mani.inc");
require_once("glob.objtab.access.helper.inc");
require_once ("visufuncs.inc");

class fAccessLiShow {
	
function __construct($tablename, $group_id) {
	$this->tablename=$tablename;
	$this->group_id = $group_id;
	$this->AccHelperLib=new fAccessHelper();
	
	$sqlopt["order"] = 1;
	$this->sqlAfter  = get_selection_as_sql( $tablename, $sqlopt);
}

function getRig2colname() {
	return ($this->AccHelperLib->right_name2column_name);
}

function rigtabshow (&$info_tab2) {
	$this->AccHelperLib->tabshow2 ($info_tab2);
}

function showGroup(&$sql) {
	global $error;
	
	$group_id = $this->group_id;
	
	$sql->query("SELECT name FROM user_group WHERE user_group_id = ".$group_id);
	if ($error->printLast()) htmlFoot();
	if (!$sql->ReadRow()) {
		htmlFoot("ERROR", "Group ID:$group_id not found!");
	} else {
		$infox["groupname"] = $sql->RowData[0];
		$info_tab2[] = array(" Selected group", "<img src=\"images/icon.USER_GROUP.gif\"> <b>".$infox["groupname"]."</b>");
	}
	$this->rigtabshow ($info_tab2); 
}

function showAll( &$sql, &$sql2) {
	global $error;
	
	$rowCnt   = 0;
	$MAX_SHOW = 1000;
	
	$groupstor=NULL;
	
	$tablename=$this->tablename;
	$most_imp_col = importantNameGet2($tablename);
	$prim_key_col = PrimNameGet2($tablename);
	
	$tabobj = new visufuncs();
	$headOpt = array( "title" => "Details for group-access-rights of objects");
	$headx  = array ("ID", "Name", "Group-Rights");
	$tabobj->table_head($headx,   $headOpt);
	
	$sql2->query("SELECT x.cct_access_id, x.$most_imp_col, x.$prim_key_col FROM ".$this->sqlAfter);
	if ($error->printLast()) htmlFoot();
	$objHasAcc = 0;
	$loopcnt =0;
	
	while ($sql2->ReadRow()) {
	
		$rights = array();
		$loopcnt++;
		if ( $loopcnt>$MAX_SHOW ) {
			$dataArr = array("...", "...", "<font color=red>WARNING</font>: Too many ($loopcnt) objects selected for this function. STOP.");
			$tabobj->table_row ($dataArr);
			break;
		}
		
		$sql->query( "SELECT user_group_id as grid,
					select_right as r, 
	 				update_right as w, 
	  				delete_right as d, 
	   				insert_right as i,
					entail_right as e".
					" FROM cct_access_rights WHERE cct_access_id = ".$sql2->RowData[0].
					" order by user_group_id"
					);
		
		$objid   = $sql2->RowData[2];
		$objname = $sql2->RowData[1];
		
		$storeRig=NULL;
		while ($sql->ReadArray()) {	
			$storeRig[] = $sql->RowData;
		}
		
		$righttext = "";
		
		if ( sizeof($storeRig) ) {
		
			$objHasAcc++;
			
			$tmpkomma  = "";
			
			
			foreach( $storeRig as $key=>$valarr) {
				
				$grpid = $valarr["GRID"];
				
				// get name
				if ( $groupstor[$grpid] ) {
					$grname = $groupstor[$grpid];
				} else {
					if ( sizeof($groupstor)<40 ) {
						$groupstor[$grpid] = obj_nice_name ( $sql, "USER_GROUP", $grpid );
						$grname = $groupstor[$grpid];
					} else {
						$grname = obj_nice_name ( $sql, "USER_GROUP", $grpid );
					}
				}
				
				$oneGrpTxt="";
				foreach( $valarr as $key2=>$val) {
					if ($val==1)  $oneGrpTxt .=$key2;
					else $oneGrpTxt .="-";
				}
				reset($valarr);
				
				$onegrpout = "<b>".$grname."</b>: ".$oneGrpTxt;
				$righttext .= $tmpkomma . $onegrpout;
				$tmpkomma = ", ";
				
			}
			reset ($storeRig); 
		}
		
		$dataArr = array($objid, $objname, $righttext);
		
		$tabobj->table_row ($dataArr);
	}
	
	$tabobj->table_close();
	
	echo "<br>";
	echo "<b>$objHasAcc</b> objects have access-rights.<br>";
}

}


$tablename  = $_REQUEST["tablename"];
$group_id	= $_REQUEST["group_id"];
$optall 	= $_REQUEST["optall"];

		
if ( $optall>0 ) {
	$infotitle = "(full details)";
} else {
	$infotitle = "for ONE group";
}

$title    = "List of access settings for selected objects ".$infotitle;
$title_sh = "List of settings ".$infotitle;

$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] );
$sql2  = logon2( $_SERVER['PHP_SELF'] );
$infoarr=array();
$infoarr["help_url"] = "access_info.html";
$infoarr["title"]    = $title;
$infoarr["title_sh"] = $title_sh;
$infoarr["form_type"]= "list";
$infoarr["obj_name"] = $tablename;
$infoarr["obj_cnt"]  = 1; 
$infoarr["locrow"]   = array( array("glob.objtab.access.php?t=".$tablename, "AccessInfo") );
 
$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);

$mainlib = new fAccessLiShow($tablename, $group_id);

$rig2colname = $mainlib->getRig2colname();

echo "<ul>";
$infox = NULL;
if (empty($tablename)) htmlFoot("ERROR", "Please give me the tablename!");
if (!cct_access_has2($tablename)) htmlFoot("INFO", "No access information available, because the object is not a business object!");

if ( $optall>0 ) {
	// 
} else {
	if (empty($group_id)) htmlFoot("ERROR", "Please give me the group_id!");
}


$tmp_info     = $_SESSION['s_tabSearchCond'][$tablename]["info"];
$most_imp_col = importantNameGet2($tablename);
$prim_key_col = PrimNameGet2($tablename);
if (empty($tmp_info)) {
  htmlFoot("Attention", "Please go back and select elements from the list!");
}

$bgcolor       = "#e0e0e0";
$users         = array();
$info_tab2     = array();
// $info_tab2[" Condition"] = $tmp_info;

if ( $group_id ) {
	$mainlib->showGroup($sql);
}

if ($optall>0) {
	$mainlib->showAll($sql, $sql2);
	htmlFoot("<hr>");
}

echo '<table cellspacing="1" border=0>';
echo '<tr bgcolor="#DDDDDD"><th>'.columnname_nice2($tablename, $prim_key_col).'</th>';
echo '<th>'.columnname_nice2($tablename, $most_imp_col).'</th>';
echo '<th>Owner</th>';
do {
  echo '<th>'.key($rig2colname).'</th>';
} while(next($rig2colname));

echo '</tr>';


// get selected objects
$rowCnt = 0;
$sql->query("SELECT x.cct_access_id, x.$most_imp_col, x.$prim_key_col FROM ".$mainlib->sqlAfter);
if ($error->printLast()) htmlFoot();
while ($sql->ReadArray()) {
  
  $rights = array();
  
  $sql2->query("SELECT select_right, update_right, delete_right, insert_right, entail_right ".
			   " FROM cct_access_rights WHERE cct_access_id = ".$sql->RowData["CCT_ACCESS_ID"]." AND user_group_id = $group_id");
  if ($error->printLast()) htmlFoot();
  if ($sql2->ReadArray()) {
  
  	$tmp_user_id = 0;
	reset($rig2colname);
	do {
	  $rights[key($rig2colname)] = $sql2->RowData[current($rig2colname)];
	} while(next($rig2colname));
	
	if ($sql->RowData["CCT_ACCESS_ID"]) {
		$sql2->query("SELECT db_user_id from cct_access where cct_access_id=".$sql->RowData["CCT_ACCESS_ID"]);
		$sql2->ReadRow();
		$tmp_user_id = $sql2->RowData[0];
	}
	 
	if ($tmp_user_id AND !array_key_exists($tmp_user_id, $users)) {
	  $sql2->query("SELECT nick FROM db_user WHERE db_user_id = ".$tmp_user_id);
	  if ($error->printLast()) htmlFoot();
	  $sql2->ReadRow();
	  $users[$tmp_user_id] = $sql2->RowData[0];
	}

	$bgcolor = ($bgcolor == "#f0f0f0") ? "#e0e0e0" : "#f0f0f0";

	echo '<tr bgcolor="'.$bgcolor.'"><td>'.$sql->RowData[$prim_key_col].'</td>';
	echo '<td>'.$sql->RowData[$most_imp_col].'</td>';
	echo '<td>'.$users[$tmp_user_id].'</td>';

	foreach( $rights as $dummy=>$right_val)
	  echo '<th><img src="images/but.'.($right_val ? "checked" : "checkno").'.gif"></th>';
	
	$rowCnt++;
  }
}
echo "</table>";

echo "<br><font size=+1><B>$rowCnt</B></font> objects give access to this group.<br>";
echo "</ul>";
htmlFoot();

