<?php
/**
 * view/modify access-rights for one obejct
 * - for business objects: set access rights
 * - support also recursive object touch for PROJECTs
 * - for ASSOC elements: get access info
 * QM-things: if object is:
	  + LOCKED AND 
	  + $_SESSION['globals']["security_write"] AND 
	  + user is 'table admin' AND 
 	  + CCT_ACCLOG for this object, 
	  
	  => then:
		- tell user to REOPEN, need 'QC-admin-flag ???'
		- add 'reopen' status to object
	    - change the right
 * @package glob.obj.access.php
 * @swreq UREQ:0000951: show/edit access matrix, show audit trail 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @modtest EXISTS for modify
 * @param $t   tablename
	      $id       identifier of primary key (OLD:prim_id)
          id2   TBD: really used?         primary key 2 id[1]
          id3   TBD: really used?         primary key 3 id[2]
	      [new_groupid]    id of newly added group
		  [read_grp[], write_grp[], delete_grp[], insert_grp[], entail_grp[]] ... arrays containing group_ids for the rights
		  [go]             do writing operation
          [proj_recursive] set rights recursivly for project
          [proj_recursive_method] 'add'/'set' set rights or add rights for proj_recursive
		  [specialAct]  : "", "reopen"
		  [myGroupAct]  : 0,1 : [DEPRECATED ]myGroup given
		  [showProblems]: 0,1 show audit log problems
  */

session_start(); 


// require_once ('reqnormal.inc');
require_once 'subs/glob.obj.superhead.inc';
require_once ('o.CCT_ACCLOG.gui.inc');
require_once ('f.help.inc');
require_once ("javascript.inc" );

require_once ('glob.obj.access.sub.inc');

$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] );
$sql2  = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot();

$tablename=$_REQUEST['t'];
$prim_id  =$_REQUEST['id'];
//$id2  =$_REQUEST['id2'];
//$id3  =$_REQUEST['id3'];
$new_groupid  =$_REQUEST['new_groupid'];
$go  =$_REQUEST['go'];
$proj_recursive =$_REQUEST['proj_recursive'];
$proj_recursive_method =$_REQUEST['proj_recursive_method'];
$specialAct   =$_REQUEST['specialAct'];
// $myGroupAct   =$_REQUEST['myGroupAct'];
$showProblems =$_REQUEST['showProblems'];

$read_grp=$_REQUEST['read_grp'];
$write_grp=$_REQUEST['write_grp'];
$delete_grp=$_REQUEST['delete_grp'];
$insert_grp=$_REQUEST['insert_grp'];
$entail_grp=$_REQUEST['entail_grp'];

$title =  "Access information of object";
//$userGroupCond  = "&condclean=1&tableSCond=".urlencode("(SINGLE_USER is NULL or SINGLE_USER!=1)");
//$tablename_nice = empty($tablename) ? 'object': tablename_nice2($tablename);
//$pk_arr         = primary_keys_get2($tablename);

$gui_lib = new glob_obj_superhead($tablename, $prim_id);
$gui_lib->page_open($sql, '0perm');


if ( $_SESSION["userGlob"]["g.debugLevel"]>0 ) {
    $mainPageDebug = $_SESSION["userGlob"]["g.debugLevel"];
} else $mainPageDebug=0;

$mainSubLib = new gObjAccessSub ( $sql, $tablename, $prim_id);
// $mainSubLib->showNavTab();

?>
<style>
.hoverTable tr:hover {
     background-color: #DDDDff;
}
</style>
<?php

echo "<ul>";

if (empty($prim_id ) || empty($tablename)) {
  htmlFoot("ERROR", "Please give table name and ID!");
}

if (!access_reading_allowed($sql, $tablename, $prim_id)) {
  htmlFoot('ALERT', 'You are not allowed to read this object!');
}

$isTabAdmin = 0;
$mainSubLib->setInitVars($sql);
$assoc_is      = $mainSubLib->assoc_is;
$basetable     = $mainSubLib->basetable;
$basetable_nice= $mainSubLib->basetable_nice;
// $mothertable   = $mainSubLib->mothertable;
$mothertable_nice= $mainSubLib->mothertable_nice;

$isTabAdmin = role_admin_check ( $sql, $basetable );

if (empty($go)) $go = 0;
if (empty($proj_recursive)) $proj_recursive = 0;
if (empty($proj_recursive_method)) $proj_recursive_method = "";
if ($proj_recursive == 1) {
  if (strcmp($tablename, 'PROJ'))
	htmlFoot('ALERT', 'proj_recursive is only allowed for projects.');
  if (empty($proj_recursive_method))
	htmlFoot('ERROR', 'You have to select add or set telling how to handle rights.');
  if ($proj_recursive_method!='add' and $proj_recursive_method!='set')
	htmlFoot('ALERT', 'Detected invalid value for proj_recursive_method.');
}

$cct_access_id = $mainSubLib->get_cct_access_id($sql);

$o_rights  = access_check ( $sql, $basetable, $prim_id, $cct_access_id );
if ($error->printLast()) htmlFoot();

$t_rights = tableAccessCheck($sql, $basetable);

$mainSubLib->checkAuditState($sql);

if ($go == 1 ) { // set rights now!
	if ( $specialAct=="reopen" ) {
		$urlnew='p.php?mod=DEF/g.obj.reopen&t='.$basetable.'&id='.$prim_id;
		js__location_replace( $urlnew, "go to reopen-tool" );
		exit;
	} else {
		$doForward = $mainSubLib->changeit($sql,$sql2,$o_rights,
			$read_grp,
			$write_grp,
			$delete_grp,
			$insert_grp,
			$entail_grp,
			$new_groupid,
			$proj_recursive,
			$proj_recursive_method
			);
			
		if ( $error->printAll() ) {
			htmlFoot();
		}
	}
	
	echo "<br />\n";
	$urlnew = $_SERVER['PHP_SELF'] . "?t=".$tablename."&id=".$prim_id;    
	
	if ($mainPageDebug>0) {
		$actionLog = $mainSubLib->getActionLog();
		htmlInfoBox( "Debug Action log", "", "open", "CALM" );
		echo '<pre>';
		echo implode("\n", $actionLog);
		echo '</pre>';
		htmlInfoBox( "", "", "close" );
	}
	
	if ($doForward) {	
		js__location_replace( $urlnew, "reload access-page" );
		exit;
	} else {
		echo ' [<b><a href="'.$urlnew.'">back to access page</a></b>]';
		htmlFoot('<hr>');
	}
}

$o_rights = access_check($sql, $basetable, $prim_id, $cct_access_id); // recheck if user removed some of his own rights

if ($assoc_is) {
	htmlInfoBox( "Info:  No access info for feature elements.", 
		"This is a feature list element of '<B>".$mothertable_nice."</B>'. You can only modify the rights on the mother ".
            "object.", "", "WARN" );
	htmlFoot();
}

echo "<B>Access permissions";

if ($assoc_is) {
    echo " of $mothertable_nice";
}
echo "</B>";
echo "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;[<a href=\"obj.db_user.manage_users_groups.php\">My default object creation settings =&gt;</a>]";

echo " &nbsp;&nbsp;&nbsp; ".fHelpC::link_show('access_info.html', "help", array("object" =>"icon"));
echo "<blockquote>\n";


$mainSubLib->mainForm( $sql, $sql2, $o_rights, $t_rights );

$rinopt = NULL;
$rinopt["accCheckRed"] = $mainSubLib->infox["accCheckRed"];
if ($isTabAdmin) $rinopt["isTabAdmin"] =1; 
$fAccInfoLib = new fAcessInfoC($basetable);
$fAccInfoLib->rightsInfo($sql, $rinopt);
echo "</blockquote>\n";

// test, if one right is not set
$mainSubLib->show_missRoleRight( $t_rights, $basetable_nice);

$accLogLib = new oAccLogGuiC();
$accLogLib->setObject( $sql, $tablename, $prim_id );

echo "<B>Object audit trail</B>";

// old tool was: obj.cct_acclog.add.php
$helpLib = new fHelpC();
$hlpopt  = array("object"=>'icon');
$helpText = $helpLib->link_show("o.CCT_ACCLOG.html", "help", $hlpopt);

echo '&nbsp;&nbsp;&nbsp;';
echo ' <a href="glob.obj.acclock.php?t='.$tablename.'&id='.$prim_id.'"><img src="images/but.lock.do.gif">'.
	 ' Add audit entry</a> | &nbsp;&nbsp;';
if (!$showProblems) 
	echo ' [<a href="'.$_SERVER['PHP_SELF'].'?t='.$tablename.'&id='.$prim_id.'&showProblems=1">show more</a>]&nbsp;&nbsp; ';
echo $helpText;

	
if ($accLogLib->actionsExist($sql) ) {
	echo "<blockquote>\n";
	$logOpt = array();
	if ($showProblems>0) $logOpt=array('shProblems'=>1);
	$accLogLib->showLogTable($sql, $sql2, $logOpt);
	echo "<br>";
	echo "</blockquote>\n";
} else {
	echo "<br><br>";
}


htmlFoot("<br>\n<hr size=1 noshade>");
