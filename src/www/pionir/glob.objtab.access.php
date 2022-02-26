<?php
/**
 * - [AccessInfo] Access settings for selected objects
 * - change access for a selection of objects
 * @namespace core::
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param  
 *  $tablename or $t
  	$showit: must be set, if more than 1000 objects are selected
		 	0 : default
		    1 : show, even if too many object 
 */
session_start(); 


require_once ('reqnormal.inc');
require_once("sql_query_dyn.inc");
require_once("access_mani.inc");
require_once("glob.objtab.access.helper.inc");
require_once('access_info.inc');
require_once ( "javascript.inc" );

class fAccessListView {

function __construct($tablename) {
	$this->tablename=$tablename;
	$this->AccHelperLib=new fAccessHelper();
}

function access_show(&$o_rights) {
	$this->AccHelperLib->access_show($o_rights, false);
}

function tabshow2 (&$info_tab2) {
	$this->AccHelperLib->tabshow2 ($info_tab2);
}

function getGroupOfElem(
	&$sql,
	$cct_access_id, 
	&$tmp_group_id_arr, // return value: groups => rights on n business objects
	&$group_arr_rig     // return value: existing rights for the business objects
	) 
	{
	
    $right2colname = &$this->AccHelperLib->right_name2column_name;

    $error     = & ErrorHandler::get();
    $group_arr = array();
    $sql->query("SELECT user_group_id, select_right, update_right, delete_right, insert_right, entail_right ".
                " FROM cct_access_rights WHERE cct_access_id = $cct_access_id");
    if ($error->got(READONLY)) return;
    while ($sql->ReadArray()) {
        $group_id = $sql->RowData["USER_GROUP_ID"];
        if (empty($tmp_group_id_arr[$sql->RowData["USER_GROUP_ID"]]))
            $tmp_group_id_arr[$sql->RowData["USER_GROUP_ID"]] = 1; // not yet encountered group
        else
            $tmp_group_id_arr[$sql->RowData["USER_GROUP_ID"]]++; // group found again

        reset($right2colname);
        do {
            if (!isset($group_arr_rig[$group_id][key($right2colname)])) // group not yet encountered
                $group_arr_rig[$group_id][key($right2colname)] = $sql->RowData[current($right2colname)];
            elseif ($group_arr_rig[$group_id][key($right2colname)] != $sql->RowData[current($right2colname)])
                $group_arr_rig[$group_id][key($right2colname)] = -1; // rights for group not equal for all objects
        } while(next($right2colname));
		
		reset($right2colname);
    }
}

function showRelFuncs() {
	$tablename = $this->tablename;
	echo "<br>";
	echo "</ul>";
	echo "<hr noshade>";
	echo "Related functions:<ul>";
	echo "<img src=\"images/but.lock.in.gif\" hspace=2><a href=\"glob.objtab.acclock.php?t=".$tablename."\">Lock Objects</a><br>\n";
	echo "<img src=\"images/but.lock.reopen.gif\" hspace=2><a href=\"glob.objtab.accreopen.php?t=".$tablename."\">Reopen objects to me</a><br>\n";
	echo "<img src=\"images/but.search.gif\" hspace=2><a href=\"glob.objtab.access.sea.php?t=".$tablename."&act=withmod\">Search objects with MOD-rights</a><br>\n";
	if ( glob_isAdmin() ) {
		echo "<img src=\"0.gif\" widht=23 height=20 hspace=2><a href=\"glob.objtab.acc.touch.php?t=".$tablename."\">Touch objects</a><br>\n";
	}
	echo "</ul>\n";
	
}

function normalFoot() {
	$this->showRelFuncs();
	echo "</UL>";
	htmlFoot("<hr>");
}

}

$MAX_SHOW_CNT = 1000;
$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] );
$sql2  = logon2( $_SERVER['PHP_SELF'] );

$tablename=$_REQUEST['tablename'];
$t=$_REQUEST['t'];
$showit=$_REQUEST['showit'];

if ( $t!="" ) $tablename = $t;
$title = "[AccessInfo] Access settings for selected objects";
$title2 = "AccessInfo";
$userGroupCond = "&condclean=1&tableSCond=".urlencode("(SINGLE_USER is NULL or SINGLE_USER!=1)");

$infoarr=array();
$infoarr["help_url"] = "access_info.html";

$infoarr["title"] = $title;
$infoarr["title_sh"] = $title2;
$infoarr["form_type"]= "list";
$infoarr["obj_name"] = $tablename;
$infoarr["obj_cnt"]  = 1; 


$pagelib = new gHtmlHead();
$pagelib->_PageHead ( $infoarr["title"],  $infoarr );

?>
<script language="JavaScript">
<!--
function newGroupGo() {
    group_id = document.editform.new_group_id.value;
    if (group_id=="") {
        alert ('Please select a group first!');
        return;
    } 
    location.href='glob.objtab.access_mod.php?tablename=<?=$tablename?>&group_id=' + group_id;
}

function openAccWin( desttable, group_id ) {
    url_name="glob.objtab.access.show_rights.php?tablename="+desttable+"&group_id="+group_id;
    InfoWin = window.open( url_name, desttable, "scrollbars=yes,status=yes,width=650,height=500");
    InfoWin.focus();
}


//-->
</script>
<?

$owopt = array("cond"=>$userGroupCond);
js__openwin($owopt);
js__linkto();
js__inputRemote();
js__openproj();

 
$headarr = $pagelib->_startBody($sql, $infoarr);

$mainlib = new fAccessListView($tablename);

echo "<ul>";

if ( empty($tablename) ) htmlFoot("ERROR", "Please give a tablename!");
if ( !cct_access_has2($tablename) ) {

    echo "<font color=red>INFO:</font> No access information available, because the table type is not 'business object'!<br>";
    $mothertable = mothertable_get2($tablename);
    if ( $mothertable != "" ) {
        $mothertable_nice = tablename_nice2($mothertable); 
        echo "<font color=green>TIP:</font> This are associated elements owned of the object '".$mothertable_nice."'.<br>\n";
        echo "Select this object '".$mothertable_nice."' to get/change access properties.<br>\n";
    }    
    echo "</body></html>\n";
    return (-1);
}

$tmp_info  = $_SESSION['s_tabSearchCond'][$tablename]["info"];
$sqlAfter  = get_selection_as_sql($tablename);
if ( empty($tmp_info) ) {
    htmlFoot("Attention", "Please go back and select elements from the list!");
}

$isTabAdmin = role_admin_check ( $sql, $tablename );
// get selected objects
$sql->query("SELECT x.cct_access_id, a.db_user_id FROM $sqlAfter");
if ($error->printLast()) htmlFoot();

$cnt           = $headarr["obj_cnt"]; // number of objects
$group_arr     = array();
$group_arr_rig = array();
$users         = array();

if ($cnt > $MAX_SHOW_CNT AND !$showit) {
	htmlInfoBox( "Info: Many objects", "You selected more than $MAX_SHOW_CNT objects.<br>The access analysis can take a lot of time.<br><br>".
	   "<a href=\"".$_SERVER['PHP_SELF']."?tablename=".$tablename."&showit=1\"><b>&gt;&gt; continue the \"access info view\" anyway </B></a><br><br>".
	   "<a href=\"glob.objtab.accdel.php?tablename=".$tablename."&groupall=1\">&gt;&gt; <img src=\"images/but13.del.gif\" border=0> delete ALL group-right-entries from the selected objects</a>"
	   , "", "INFO" );
	$mainlib->normalFoot();
}

while ($sql->ReadRow()) {
    if (array_key_exists($sql->RowData[1], $users))
        $users[$sql->RowData[1]]++; // found object belonging to a already known user
    else
        $users[$sql->RowData[1]] = 1; // first object belonging to this user found 

    $mainlib->getGroupOfElem($sql2, $sql->RowData[0], $group_arr, $group_arr_rig);
    if ($error->printLast()) htmlFoot();
    $cnt++;
}

// print_r_pre($users);
// print_r_pre($group_arr);
// print_r_pre($group_arr_rig);

$info_tab2=array();
// $info_tab2[]=array(" Condition"]       = $tmp_info;
// $info_tab2[]=array("Selected objects" , "<b>".$cnt."</b>");

$tmptxt = "<img src=\"images/icon.DB_USER.gif\"> Owners of objects:";
if ($_SESSION['sec']['appuser']=="root") $tmptxt  .= " &nbsp;&nbsp;[<a href=\"glob.objtab.trackgui.php?tablename=".$tablename."\">Change the owner!</a> ... in CCT_ACCESS]";
$info_tab2[]=array("headline241177"   , $tmptxt );

reset($users);
do {
    $sql->query("SELECT nick FROM db_user WHERE db_user_id = ".key($users));
    if ($error->printLast()) htmlFoot();
    $sql->ReadRow();
	$tmpkey = (key($users) == $_SESSION['sec']['db_user_id']) ? "<B>".$sql->RowData[0]."</B>" : $sql->RowData[0];
	
    $info_tab2[]=array( $tmpkey , current($users));
} while (next($users));
// tabshow ($info_tab2[]);


$info_tab2[]= array(
		"headline241177" ,
		"<img src=\"images/icon.USER_GROUP.gif\">".
		" Groups which have access to the objects:".
		"&nbsp;&nbsp;&nbsp;[<a href=\"glob.objtab.access.show_rights.php?tablename=".$tablename."&optall=1\">show ALL right-details</a>]");
reset ($group_arr);
$group_names = array(); // array: user_group.id => user_group.name
if (count($group_arr)) {// only if objects are in groups
    do {
        $sql->query("SELECT name FROM user_group WHERE user_group_id = ".key($group_arr));
        $sql->ReadRow();
        $info_tab2[]=array($sql->RowData[0] , current($group_arr));
        $group_names[key($group_arr)] = $sql->RowData[0];
    } while (next($group_arr));
}
if (empty($info_tab2)) {
    $info_tab2[]= array("none","&nbsp;");
}

$mainlib->tabshow2 ($info_tab2);

unset($info_tab2);
reset ($group_arr);
echo "<P>";
echo "<form name='xform' ACTION='glob.objtab.accdel.php?tablename=$tablename' METHOD='POST'>\n";
echo "<table cellspacing='0' cellpadding='1' border='0'>\n";
echo "<tr><td bgcolor='#999999'>\n";
echo "<table cellspacing='0' cellpadding='4' bgcolor='#ffffff' width='100%' border='0'>\n";
echo "<tr bgcolor='#DDDDDD'><td colspan=2>";
echo "<input type=submit value=\"Delete rights\">";
echo "</td><th>Group</th><th>Objects</th>";
echo "<th>Read</th><th>Write</th><th>Delete</th><th>Insert</th><th>Entail</th><th>&nbsp;</th></tr>\n";
$tmpGrpCnt=0;
if (count($group_arr)) {
    do {
        $tmp_group_id = key($group_arr);
        $o_rights     = &$group_arr_rig[$tmp_group_id];
        echo "<tr><td><input type=checkbox name=grpid[".$tmp_group_id."] value=1></td><td>";
        echo "<a href='glob.objtab.access_mod.php?tablename=$tablename&group_id=$tmp_group_id'>";
        echo "<img src='images/but.modify.gif' border='0' TITLE='modify'></a>";
        echo "</td><td>".$group_names[$tmp_group_id]."</td><th>".$group_arr[$tmp_group_id]."</th>";
        $mainlib->access_show($o_rights);
        echo "<td><a href=\"glob.objtab.access.show_rights.php?tablename=".$tablename."&group_id=".$tmp_group_id."\">show</a></td>";
        echo "</tr>\n";
		$tmpGrpCnt++;
    } while (next($group_arr));
}

// row for new group
echo "</form>\n";
echo "<form name='editform' ACTION='".$_SERVER['PHP_SELF']."?tablename=$tablename' METHOD='POST'>\n";
?>
<tr>
<td>&nbsp;</td><td><a href="javascript:newGroupGo()"><img src="images/but.modify.gif" border=0></a></td>
<td colspan="2" nowrap>
<?
$jsFormLib = new gJS_edit();
$fopt = array();
$answer = $jsFormLib->getAll('USER_GROUP', 'new_group_id', '', '                  none                  ',  0, $fopt);
echo $answer;
?>
</td>
<td colspan="6">&lt;--- select a new group here</td>
</tr>
</table>
</tr></td></table>
</form>
<?

echo "<font size=-1><a href=\"glob.objtab.accdel.php?tablename=".$tablename."&groupall=1\"><img src=\"images/but13.del.gif\" border=0>".
   " Delete right-entries of ALL groups</a> (faster than selecting groups)</font><br>\n";
	  
$rinopt = array("isTabAdmin"=>$isTabAdmin);
$fAccInfoLib = new fAcessInfoC($tablename);
$fAccInfoLib->rightsInfo($sql, $rinopt);

echo "<br><br>";
htmlInfoBox( "Legend", "", "open", "CALM" );
?>
<img src="images/but.checked.gif"> - right is switched <i>on</i> for all selected objects<br>
<img src="images/but.checkno.gif"> - right is switched </i>off</i> for all selected objects<br>
<img src="images/but.checkgray.gif"> - for some selected objects right is switched <i>on</i> for others <i>off</i><br>
<font color=gray>(only the owner and groups having entail-right can manipulate rights)</font><br>
<?
htmlInfoBox( "", "", "close" );

$mainlib->normalFoot();
