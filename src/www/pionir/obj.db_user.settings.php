<?
/**
 * set global user defined preferences: $_SESSION['userGlob'] of this user
 * @package obj.db_user.settings.php
 * @swreq SREQ:0002566: o.DB_USER > manage user settings 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param 
 *       [$backtablename]
		 [$seltab] 0,1 - select a table
		 $mode : 
		   ['default'], 
		    'groups'
		 $setti
		 $go
 * @version $Header: trunk/src/www/pionir/obj.db_user.settings.php 59 2018-11-21 09:04:09Z $
 */


// extract($_REQUEST); 
session_start(); 

require_once ('reqnormal.inc');
require_once("subs/f.prefsgui.inc");
require_once("func_form.inc");
require_once("o.DB_USER.subs2.inc");
require_once("o.DB_USER.subs.inc");
require_once 'gui/o.DB_USER_rights_show.inc';

class fMainPrefsGui{
	
    function htmlInfoOpen( $headtext ) {
    	
    	$color1 = "#606060";
    	$color2 = "#EFEFEF";
    	$color3 = "#FFFFFF";
    	
    	print "<table border=0 cellspacing=1 cellpadding=0 bgcolor=$color1><tr><td>";
           
    	print "  <table border=0 cellspacing=0 cellpadding=2 bgcolor=$color1 width=100%><tr><td>";  
    	print "    <font color=".$color3.">&nbsp;<B>".$headtext."</B></font></td>";
    	print "    </tr></table>\n";
        print "  <table border=0 cellspacing=0 cellpadding=5 bgcolor=$color2 width=100%><tr><td>";
    	
    	echo "<table cellspacing=0 cellpadding=0 border=0>"; // form
    }
    
    function htmlInfoClose(  ) {
    	echo "</table>"; // form
    	print "</td></tr></table>\n";
    	print "</td></tr></table>\n";
    }
    
    function prefNameShow( $text, 
    	$option=NULL // "bgcolor"
    				 // "headline"
    	) {
    	$bgcolor = "";
    	if ($option["headline"]==1) {
    		echo "<tr".$bgcolor."><td colspan=3>".$text."</td></tr>\n";
    		return;
    	} 
    	if ($option["bgcolor"]!="") $bgcolor= " bgcolor=\"".$option["bgcolor"]."\"";
    	echo "<tr".$bgcolor."><td><font color=gray>".$text.":&nbsp;</font></td>\n";
    }  
    function prefClose( $info="" ) {
    	echo "<td>&nbsp;<I>".$info."</I></td></tr>\n";
    }
    function empty_line($text) { 
     //bgcolor=#D0D0D0 
    	echo "<tr height=7><td colspan=3></td></tr>\n";
        echo "<tr bgcolor=#DFDFDF height=20><td colspan=3>&nbsp;&nbsp;&nbsp;<b><font color=gray>".$text."</font></b></td></tr>\n"; // <hr NOSHADE size=3>
    }
    
    function form() {
        
        global $error;
        
        $this->htmlInfoOpen( "Main application settings" );
        ?>
        <form  method="post" action="<?echo $_SERVER['PHP_SELF']?>?go=1" > 	
        <?	
        
        	
        $this->prefNameShow( "Full login info" );
        $checked="";
        if  ($_SESSION['userGlob']["g.headerloginfo"]) $checked="checked";
        echo "<td><input type=checkbox name=setti[g.headerloginfo] value=1 $checked > with DB name </td>";		
        $this->prefClose ("Shows extended login info in the header frame");
         
        /*
         * DEPRECATED: default: show forever
        $this->prefNameShow( "Quick search" );
        $checked="";
        if  ($_SESSION['userGlob']["g.headquicks"]) $checked="checked";
        echo "<td><input type=checkbox name=setti[g.headquicks] value=1 $checked > show</td>";		
        $this->prefClose ("Shows a quick search form in the header frame");
        */
               
        $this->prefNameShow( "Start page" );
        $checked="";
        $var = "g.appStartPage";
        if  ($_SESSION['userGlob'][$var] == "project") $checked="checked";
        echo "<td><input type=checkbox name=setti[".$var."] value=1 $checked > start with project </td>";		
        $this->prefClose ("Application starts with [theme park] or project");
        
//         $this->prefNameShow( "show GUI-Plugin-Code" );
        
//         $checked="";
//         $var     = "g.plugin";
        
//         $varcont = NULL;
//         if ($_SESSION['userGlob'][$var]!=NULL) $varcont = unserialize($_SESSION['userGlob'][$var]);
//         $subvar  = 'shMod'; // show tool name
//         if  ( $varcont[$subvar]>0 ) $checked="checked";
//         echo '<td><input type=checkbox name="setti['.$var.']['.$subvar.']" value=1 '.$checked.' ></td>';		
//         $this->prefClose ("including Plugin-Version (if exists)");
        
        
//         $this->prefNameShow( "QC specials" );
//         $checked="";
//         $var = "g.appQC";
//         if  ($_SESSION['userGlob'][$var] == "1") $checked="checked";
//         echo "<td><input type=checkbox name=setti[".$var."] value=1 $checked ></td>";		
//         $this->prefClose ("Special QC checks / views");
        
        $this->prefNameShow( "personal emails" );
        $checked="";
        $forwardto = "nick";
        $var = "g.emailOpt";
        $tempArr  = unserialize($_SESSION['userGlob'][$var]);
        $wantEmailTmp = $tempArr['send'];
        $forwardEmailTmp = $tempArr['forward'];
        if ($tempArr['forwardto'] != "" ) $forwardto = $tempArr['forwardto'];
        if  ($wantEmailTmp != -1) $checked1="checked";
        if  ($forwardEmailTmp == 1) $checked2="checked";
        echo "<td><input type=checkbox name=setti[".$var."1] value=1 $checked1 >receive</td>";
        $this->prefClose ("receive emails from the system?");
        
        echo "<td></td><td><input type=checkbox name=setti[".$var."2] value=1 $checked2 >forward to
        <input type=textfield name=setti[".$var."3] value =".$forwardto." size = '8'</td>";		
        $this->prefClose ("insert nickname of other user");
        
        $this->empty_line("Design");
        
        $this->prefNameShow( "Show user icon" );
        $checked="";
        $var = "g.headerUserIcon";
        if  ($_SESSION['userGlob'][$var] > 0) $checked="checked";
        echo "<td><input type=checkbox name=setti[".$var."] value=1 $checked > show?</td>";		
        $this->prefClose ("Show your portrait in the header frame");  
        
        $this->prefNameShow( "Design theme" );
        $checked="";
        $var = "g.appDesign";
        $fxoption = NULL;
        $preselected = $_SESSION['userGlob'][$var];
        if ($preselected=="") $preselected=1; 
        $feld=array(  1=>"blue", 2=>"brown", 3=>"green", 6=>"violett", 7=>"purple", 4=>"toasten", 5=>"girls" );
        $tmpvar = formc::selectFget( "setti[".$var."]", $feld, $preselected, $fxoption); 
        echo "<td>".$tmpvar."</td>";		
        $this->prefClose ("Choose your design theme");  
        
        $this->prefNameShow( "Object history" );
        $checked="";
        $varSer = "g.histlist";
        $tmparr = unserialize($_SESSION['userGlob'][$varSer]);
        $varx   = $tmparr["somode"];
        $varXname = $varSer."."."somode";
        if ($varx=="") $varx="time";
        
        $preselected = $varx;
        $fxoption=NULL;
        $feld=array( "time"=>"[by time]",  "type"=>"by type" );
        $tmpvar = formc::selectFget( "setti[".$varXname."]", $feld, $preselected, $fxoption);
        echo "<td>".$tmpvar."</td>";
        	
        $this->prefClose ("Choose sort method."); 
        
        
        $this->empty_line("Debugging");
        $pfOption = NULL;
        // $pfOption["bgcolor"] = "#F8F8F8";
        $this->prefNameShow( "Debug level", $pfOption );
        echo "<td><input name=setti[g.debugLevel] value=\"" .$_SESSION['userGlob']["g.debugLevel"]. "\" size=4> </td>";
        $this->prefClose ("Level of debugging [0..5] (0: no debug info)");
        
        $this->prefNameShow( "Debug Keys", $pfOption );
        echo "<td><input name=setti[g.debugKey] value=\"" .$_SESSION['userGlob']["g.debugKey"]. "\"> </td>";
        $this->prefClose ("Module specific debug keys (comma separated)");
        
        $this->prefNameShow( "SQL logging", $pfOption );
        
        
        /**
         * allow SQL logging only for root or user with flag "su" (super user)
         */
        $sql_logging_allow=0; // default
        if ($_SESSION['sec']['appuser']=="root" OR $_SESSION['s_suflag']) $sql_logging_allow=1;
        
        $err_stack_mem=NULL;
        
        if ($sql_logging_allow) {
        	$checked="";
        	if  ($_SESSION['userGlob']["g.sql_logging"]) $checked="checked";
        	echo "<td><input type=checkbox name=setti[g.sql_logging] value=1 $checked > on </td>";
        	
        	$extras_info=NULL;
        	$log    = & SQL_log::getHandle();
        	
        	if (!$log->log_file_exists())  {
        	    $extras_info=' <b>WARNING: Log-file not exists.</b>';
        	} 
        	if ($error->Got(READONLY))  {
        	    $err_stack_mem = $error->getAllNoReset();
        	    $error->reset();
        	}
        	
        	$log_info = $log->_getLogFileName();
        	$filex=$log_info['file'];
        	$SQL_URL = 'f_workfile_down.php?file='.$filex;

        	$this->prefClose ("Enable SQL query logging".
        		' [<a href="preferences.php?act=delfile&var=g.sql_logging">Delete log-file</a>]'.
        		' [<a href="'.$SQL_URL.'" target="_sql_log">Show log-file</a>]'. $extras_info);
        } else {
        	echo "<td>-</td>";
        	$this->prefClose ('Allowed for admin or user with flag "super user"');
        }
        
        
        $pfOption = array("headline"=>1);
        $this->prefNameShow( "&nbsp;", $pfOption);
        
        // bgcolor=#CFCFFF height=30
        echo "<tr ><td>&nbsp;</td><td><input type=\"submit\" value=\"Submit\" class='yButton'> </td><td>&nbsp;</td></tr>\n";
        
        // this_empty_line();
        
        echo "</form>\n";
        $this->htmlInfoClose( );
        
        if (isset($err_stack_mem)) {
            echo "<br>";
            $error->setAll($err_stack_mem);
            $error->printAll();
        }
    }
    
    function save(&$setti) {
        
        $infox=array();
        
        echo '<span style="color:green">update preferences...</span><br>';
//         DEPRECATED
//         $tmpvar = "g.appQC";
//         unset ($_SESSION['userGlob'][$tmpvar]);
//         if ( $setti[$tmpvar]>0 ) $_SESSION['userGlob'][$tmpvar]=$setti[$tmpvar];
        
        $tmpvar = "g.headerloginfo";
        unset ($_SESSION['userGlob'][$tmpvar]);
        if ( $setti[$tmpvar]>0 ) $_SESSION['userGlob'][$tmpvar]=$setti[$tmpvar];
        
        $_SESSION['userGlob']["g.headquicks"]="";
        if ( $setti["g.headquicks"]>0 ) $_SESSION['userGlob']["g.headquicks"]=$setti["g.headquicks"];
        
        $_SESSION['userGlob']["g.debugLevel"]="";
        if ( ($setti["g.debugLevel"]>-1) && ($setti["g.debugLevel"]<10) ) $_SESSION['userGlob']["g.debugLevel"]=$setti["g.debugLevel"];
        
        unset ( $_SESSION['userGlob']["g.debugKey"] );
        if ( $setti["g.debugKey"]!="" ) $_SESSION['userGlob']["g.debugKey"] = $setti["g.debugKey"];
        
        $sql_logging_allow=0; // default: do not allow SQL-logging
        if ($_SESSION['sec']['appuser']=="root" OR $_SESSION['s_suflag']) $sql_logging_allow=1;
        $_SESSION['userGlob']["g.sql_logging"]=NULL;
        if ($sql_logging_allow) {
            if ( $setti["g.sql_logging"]>0 ) $_SESSION['userGlob']["g.sql_logging"]=1;
        }
        
//         $var     = "g.plugin";
//         $varcont = unserialize($_SESSION['userGlob'][$var]);
//         $subvar  = 'shMod'; // show tool name
//         $varcont[$subvar]=NULL;
//         if ( $setti[$var][$subvar]>0 ) $varcont[$subvar]=1;
//         $_SESSION['userGlob'][$var]=serialize($varcont);
        
        
        $sendvalue = 1;
        $forwardvalue = -1;
        $forwardto = "";
        if ( !$setti["g.emailOpt1"]) $sendvalue = -1;
        if ( $setti["g.emailOpt2"]) $forwardvalue = 1;
        $forwardto = $setti["g.emailOpt3"];
        $_SESSION['userGlob']["g.emailOpt"]=serialize(array('send'=>$sendvalue, 'forward'=>$forwardvalue, 'forwardto' => $forwardto));
        
        $var = "g.appStartPage";
        $_SESSION['userGlob'][$var] = "";
        if ( $setti[$var] ) $_SESSION['userGlob'][$var]="project";
        
        $var = "g.appDesign";
        $tmpLastState = $_SESSION['userGlob'][$var];
        $_SESSION['userGlob'][$var] = "";
        if ( $setti[$var]!="" ) $_SESSION['userGlob'][$var]=$setti[$var];
        if ($tmpLastState != $setti[$var]) $infox["gotoMainFrame"] = 1;
        
        $varSer = "g.histlist";
        $tmparr = unserialize($_SESSION['userGlob'][$varSer]);
        $varx     = $tmparr["somode"];
        $varXname = $varSer."."."somode";
        $newvar   = $setti[$varXname];
        $tmparr["somode"] = $newvar;
        $_SESSION['userGlob'][$varSer] = serialize($tmparr);
        
        $var = "g.headerUserIcon";
        $showIcLastState = $_SESSION['userGlob'][$var];
        $_SESSION['userGlob'][$var] = "";
        if ($showIcLastState != $setti[$var]) $infox["gotoMainFrame"] = 1; // size of frame is changing !
        if ( $setti[$var]>0 ) {
            $_SESSION['userGlob'][$var]="1";
        }
        
        
        echo '<script language="JavaScript">'."\n";
        echo '<!--'."\n";
        
        echo 'if ( parent.oben != null ) { '."\n"; // check if exists
        if ( $infox["gotoMainFrame"] ) {
            echo '  parent.location.href="main.fr.php";'."\n";
        } else {
            echo '	parent.oben.location.href="header.php";'."\n";
        }
        echo '}'."\n";
        
        echo ' //-->'."\n";
        echo '</script>'."\n";
        
        
    }

}

/**
 * show settings for the user, defined by the admins
 * @author steffen
 *
 */
class this_roles_Groups {
	
	function __construct($userid) {
		$this->userid=$userid;
	}
	
	function _show_list($list, $table=NULL) {
		$retlist="";
	    if (empty($list)) return;
	    $tmpkomma = "";
		
	    foreach ($list as $id => $tmpgroup) {
			$urltxt = "<a href=\"edit.tmpl.php?t=".$table."&id=$id\">".
				'<img src="images/icon.'.$table.'.gif" border=0 hspace=4>'.
				$tmpgroup."</a>";
			$retlist .= $tmpkomma . $urltxt;
			$tmpkomma = "<br />";
	    }
	    
		return ($retlist);
	}
	
	function _featTabOut($iconx, $key, $actions, $notes) {
		
		echo '<tr bgcolor=#EFEFEF valign=top>';
		echo '<td><img src="images/'.$iconx.'" border=0 hspace=3 vspace=2></td>'.
			 "<td><B>".$key."</B></td>";
		echo "<td>".$notes."</td>";
		echo "<td>";
		echo $actions;
	    echo "</td>";
	    echo "</tr>\n";
	}
	
	function showTable(&$sqlo) {
	    
	    echo '<h2>Your role/group profile</h2>';
		
		$id=$this->userid;
		$home_proj_id = oDB_USER_sub2::userHomeProjGet ($sqlo, $id);
		$user_groups  = oDB_USER_sub2::groupListGet($sqlo, $id);
		$user_roles   = oDB_USER_sub2::roleListGet ($sqlo, $id);
		$myGroup = access_getPersGrp ($sqlo, $id);
		$sqlo->query("select nick from db_user where DB_USER_ID=".$id);
		$sqlo->ReadRow();
		$userNick = $sqlo->RowData[0];
		
		echo "<table border=0 cellpadding=1 cellspacing=1  bgcolor=#D0D0FF>\n";
		
		$tmpInfo=array();
		$iamuser = array($id=>$userNick.' [ID:'.$id.']');
	    $tmpInfo["notes"]  = $this->_show_list($iamuser, "DB_USER"); 
	    $this->_featTabOut("icon.DB_USER.gif", "User object", '', $tmpInfo["notes"] );
		
		// HOME PROJECT
		$tmpInfo=array();
		if ( $home_proj_id ) {
	      	$tmpInfo["notes"]  = "<a href=\"edit.tmpl.php?tablename=PROJ&id=".$home_proj_id."\">home project</a>";
	    } 
	    $this->_featTabOut("icon.PROJ.gif", "Home project", '', $tmpInfo["notes"] );
	    
		// USER ROLES
		$tmpInfo=array();
		if ( !empty($user_roles) ) {
			$tmpInfo["actions"] = "<a href=\"obj.db_user.show_all_rights.php?user_id=".$id."\">Summary</a>";
			$tmpInfo["notes"]   = $this->_show_list($user_roles, "ROLE");
		}
	    $this->_featTabOut("icon.USER_ROLES.gif","Roles",'', $tmpInfo["notes"] );
	    
		// USER GROUP
	    $tmpInfo=array();
		if (!empty($user_groups)) {
			if ($myGroup) unset($user_groups[$myGroup]);
			$tmpInfo["notes"]  = $this->_show_list($user_groups, "USER_GROUP");
		} 
	    $this->_featTabOut("icon.USER_GROUP.gif", "Member of groups", $tmpInfo["actions"] , $tmpInfo["notes"] );
	    
		// personal group
		
		$tmpInfo = array();
		if ($myGroup>0) {
			
			
			$personal_group=array($myGroup=>$userNick);
			$tmpInfo["notes"]  = $this->_show_list($personal_group, "USER_GROUP");
		}
		$this->_featTabOut( "icon.USER_GROUP_my.gif","Personal group","", $tmpInfo["notes"] );
		
		$sqlsel = "USER_GROUP_ID,NAME from USER_GROUP where DB_USER_ID=".$id . " order by NAME";
		$sqlo->Quesel($sqlsel);
		$grpManage=array();
		while ( $sqlo->ReadRow() ) {
		    $tmpid = $sqlo->RowData[0];
		    $tmpname = $sqlo->RowData[1];
		    if ($tmpid!=$myGroup) $grpManage[$tmpid]= $tmpname;
		}
		if (!empty($grpManage)) {
			$tmpInfo["notes"]  = $this->_show_list($grpManage, "USER_GROUP");
		} 
		$this->_featTabOut( "icon.USER_GROUP.gif","Group manager of","", $tmpInfo["notes"] );
			
	    echo "</table>\n";
	    echo "<br><br>\n";
	    
	    echo '<h2>Summary of your role rights</h2>';
	    $MainLib = new oDB_USER_rigAllRig( $id );
	    $MainLib->sh_objects( $sqlo );
	    $MainLib->sh_functions( $sqlo );  
	}
}

global $error, $varcol;

$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] );

$mode = $_REQUEST['mode'];
$backtablename = $_REQUEST['backtablename'];
// $id     = $_REQUEST['id'];
$seltab = $_REQUEST['seltab'];
$go     = $_REQUEST['go'];
$setti  = $_REQUEST['setti'];

if ($mode==NULL) $mode='default';


$user_name = "";
$id_user   = $_SESSION['sec']['db_user_id'];
$sqls = "select nick from DB_USER where DB_USER_ID=".$id_user;
$sql->query($sqls);
if ( $sql->ReadRow() ) {
    $user_name = $sql->RowData[0];
}

$title   = "MyAccount : user ".$user_name;
$title_sh= "MyAccount";
$locrow  = array( array("home.php", "home") );

if ($mode=='groups') {
	$title   .= ' : profile';
	$title_sh = 'profile';
	$locrow[]  = array( $_SERVER['PHP_SELF'], "MyAccount");
}

// $back_url = "";

$infoarr			 = NULL;
$infoarr["scriptID"] = "";
$infoarr["title"]    = $title;
$infoarr["title_sh"] = $title_sh;
$infoarr["help_url"] = "preferences.html";
$infoarr["icon"]     = "images/ic24.userprefs.png";
$infoarr["css"] 	 = ".t1 { color:#808080; }";
$infoarr["form_type"]= "tool";
$infoarr["locrow"]   = $locrow;


$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);



$mainlib = new fMainPrefsGui();

if ( $user_name=="" ) {  
    htmlFoot("Error", "User [$id_user] not found");
}

/**************************/

if ($mode=='groups') {
    echo '&nbsp; <span class="yGgray">Info: These features are defined by the system-administrator or your group-administrator.</span><br />';
    
	echo '<ul>';
	$useLib = new this_roles_Groups($id_user);
	$useLib->showTable($sql);
	echo '</ul>'."\n";
	$pagelib->htmlFoot();
}
		
$prefsGuiLib = new fPrefsGuiC();		
$prefsGuiLib->showHeadTabs( "GLOBAL", $backtablename );

if ( $seltab ) {
    echo "<blockquote>";
    echo "[<a href=\"obj.db_user.settings.php\">main preferences</a>] <br><br>\n";
    
    echo "<B>Select preferences for following object type:</B> <br><br>\n"; 
    $sql->query("SELECT table_name, nice_name FROM cct_table where TABLE_TYPE='BO' ORDER BY nice_name");
	echo "<form action=\"glob.objtab.pref.php\" style=\"display:inline;\">\n";
    echo "<select name=\"tablename\">\n";
    echo "<option value=\"\">--- select object type ---\n";
    while ( $sql->ReadRow() ) {
	  $tmp_tab = $sql->RowData[0];
      $tmp_nice = $sql->RowData[1]; 
	  echo "<option value=\"".$tmp_tab."\">".$tmp_nice."\n";
	}
    echo "</select>\n";
    echo "<br><input type=submit>\n";
    echo "</blockquote>";
    htmlFoot();
}
echo "<blockquote>\n";

// $topt=array();
// $topt["nohead"] = 1;
// htmlInfoBox( "", "", "open", "INFO", $topt ); 
?>
<span style="color:gray">
  <a href="<?php echo $_SERVER['PHP_SELF'].'?mode=groups'?>"><img src="images/icon.DB_USER.gif" border=0> Profile</a> (groups, roles)
&nbsp;&nbsp;&nbsp;  <a href="obj.db_user.manage_users_groups.php"><img src="images/but.40.new.png" height=20> Object creation settings</a> 
&nbsp;&nbsp;&nbsp;  <a href="obj.db_user.paw.php?user_id=<?echo $id_user?>"><img src="images/ico.password.gif" border=0> Change password</a> 
&nbsp;&nbsp;&nbsp;  <a href="obj.db_user.prefs_save.php"><img src="images/ic.save.gif" border=0> Save settings</a>
</span>
<br>
<?
//htmlInfoBox( "", "", "close" );

if ( $go>0 ) {
    $mainlib->save($setti);
}

echo "<br>\n";

$mainlib->form();

echo "<br>";

?>

<I><font color=gray>&nbsp;All user preferences will be saved after the logout automatically.</font></I><br>
&nbsp;<br>&nbsp;<br>&nbsp;<br>
<hr size=1 noshade><center><font color=gray>


| <a class=t1 href="p.php?mod=DEF/o.DB_USER.info">Preferences info</A>
| <a class=t1 href="obj.db_user.settings.php?seltab=1">Select object types</a> 
| <a class=t1 href="view.tmpl.php?t=DB_USER">user list</a>
| <a class=t1 href="view.tmpl.php?t=USER_GROUP">group list</a>
<?       
if ( $_SESSION['sec']['appuser']=="root" OR $_SESSION['s_suflag']>0 ) {
    echo '| &nbsp;&nbsp;&nbsp;<b>Admin links: </b>| <a class=t1 href="rootsubs/rootFuncs.php">administration</a> ';
  
}
?>
</font></center>
     
<?php

$pagelib->htmlFoot();
