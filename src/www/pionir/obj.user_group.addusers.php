<?php
/**
* Add/Delete selected users to a group
* - you can use this tool if:- you are root
* - you are group admin
* - you have insert rights for this group
* 
* @package obj.user_group.addusers.php
* @swreq 
* @author  Steffen Kube (steffen@blink-dx.com)
* @param $id (of user_group)
  		 $action = ["add"], "del"
  		 $go: 0,1
*/

session_start(); 


require_once ("reqnormal.inc"); 
require_once ("sql_query_dyn.inc");
require_once ("func_form.inc");
require_once 'o.USER_GROUP.subs.inc';

function _this_form1($title, $submit, $id, $action) {
    
    // <form method="post" action="echo $_SERVER['PHP_SELF']?go=2&id=<?echo $i&action=<?echo $action" > 
    
    $initarr   = array();
    $initarr["title"]       = $title; // title of form
    $initarr["submittitle"] = $submit; // title of submit button
    $initarr["tabwidth"]    = "AUTO";   // table-width: AUTO
    
    $hiddenarr = NULL; 
    $hiddenarr["id"]     = $id; 
    $hiddenarr["action"] = $action; 
    
  
    $formobj = new formc($initarr, $hiddenarr, 1);
    
    $cl_opt=array("noBackButton"=> 1);
    $formobj->close( TRUE , $cl_opt); // close form, sshow submit-button
}

// ---------------------------

global $error;
$error = & ErrorHandler::get();

$sql  = logon2( $_SERVER['PHP_SELF'] );
$sql2 = logon2( $_SERVER['PHP_SELF'] );

$id  = $_REQUEST['id'];
$action  = $_REQUEST['action'];
$go  = $_REQUEST['go'];

$title = "Add users to group";
if ($action == "del"){
	$title = "Remove users from group";
}


$infoarr=array();
$infoarr["title"] = $title;
$infoarr["form_type"]= "obj";
$infoarr["obj_name"] = "USER_GROUP";
$infoarr["obj_id"]   = $id;
$infoarr["show_name"]= 1;
$infoarr["checkid"]=1;


$pagelib = new gHtmlHead();

$pagelib->startPage($sql, $infoarr);

echo "<UL>";


$sqls_main="select DB_USER_ID from USER_GROUP where USER_GROUP_ID=". $id;
$sql->query($sqls_main);
$sql->ReadRow();
$admin_id = $sql->RowData[0];


if ( $_SESSION['sec']['appuser']!="root" ) {
    
    $insert_has=0;
    $o_rights = access_check($sql, 'USER_GROUP', $id);
    if ( $o_rights["insert"] ) $insert_has=1;
    
    if ($admin_id!=$_SESSION['sec']['db_user_id'] and !$insert_has ) {
    
    	htmlInfoBox( "Denied", 'You must be group admin or need "insert"-rights for this group to use this function!', "", "ERROR" );
    	htmlFoot();
    }
}

$tablename ="DB_USER"; 
$sel_info  = $_SESSION['s_tabSearchCond'][$tablename]["info"];

if ($sel_info=="") {
	htmlInfoBox( "Please select users", "<center>No users selected.<br><br>[<b><a href=\"view.tmpl.php?t=DB_USER\">SELECT users</a></B>]", "", "INFO" );
	htmlFoot(); 
}

$selcond_ori = get_selection_as_sql( $tablename );
$sqls 		 = "select count(*) from " . $selcond_ori;
$sql->query("$sqls");
$sql->ReadRow();
$num_users   = $sql->RowData[0];

echo "<font color=gray>Number of selected users:</font> <B>$num_users</B>".
	"&nbsp;&nbsp; [<a href=\"view.tmpl.php?t=DB_USER\">select other users</a>]<br><br>";

if (!$num_users) htmlFoot("INFO", "Please select <a href=\"view.tmpl.php?t=DB_USER\">users</a> first!"); 
	
if ($go<2) {
    _this_form1($title, $title.' NOW!', $id, $action);
    echo "<br>";
}

echo '<span style="color=gray">Selected users:</style><ul><br>';

$group_mod_lib = new oUSER_GROUP_mod($sql, $id);
$pagelib->chkErrStop();

$sqls = "select x.db_user_id, x.nick from " . $selcond_ori;
$sql->query($sqls);
while ( $sql->ReadRow() ) {
	
	$thisUserInGroup = False;
	$db_user_tmp = $sql->RowData[0];
	$nick 		 = $sql->RowData[1];	
	echo "<img src=\"images/icon.DB_USER.gif\"> <B>".$nick."</B> ";

	$sqls = "select db_user_id from DB_USER_IN_GROUP where USER_GROUP_ID=".$id. " AND db_user_id=".$db_user_tmp;
	$sql2->query($sqls);
	if ( $sql2->ReadRow() ) {
		$thisUserInGroup = True;
	} 
	
	if ($action == "del") {	// DELETE
		if ( !$thisUserInGroup ) {
			echo " not in group.";
		} else {
			if ( $go==2 ) {
			    $group_mod_lib->remove_user($sql2, $db_user_tmp);
			    
			    if ($error->Got(READONLY))  {
    			    $group_mod_lib->close();
    			    $pagelib->chkErrStop();
			    }
				echo " removed from group.";
			}
		}
	} else { 	// add
		if ($thisUserInGroup) {
			echo " already in group.";
		} else {
			if ( $go==2 ) {
			    $group_mod_lib->add_user($sql2, $db_user_tmp);
			    if ($error->Got(READONLY))  {
			        $group_mod_lib->close();
			        $pagelib->chkErrStop();
			    }
				echo '<span style="color=green">added.</style>';
			}
		}
	}
	echo "<br>\n";

}

if ($go==2) {
    $group_mod_lib->close($sql);
}

echo "</ul></ul>\n";

htmlFoot();
