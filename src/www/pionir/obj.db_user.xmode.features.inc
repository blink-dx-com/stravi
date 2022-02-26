<?
/**
 * user extension
 * @package obj.db_user.xedit.php
 * @swreq 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/www/pionir/obj.db_user.xedit.php 59 2018-11-21 09:04:09Z $
 */

require_once ("insert.inc");

class o_DB_USER_feature_cls {
    
    public $editAllow=0;
    
    function __construct($sqlo,  $user_id) {
        $this->user_id   = $user_id;
        
        $editAllow=0;
        if ($_SESSION['sec']['db_user_id'] == $user_id) $editAllow=1;
        
        $this->editAllowRoot = 0;
        if ( $_SESSION['sec']['appuser'] == "root" ) {
            $this->editAllowRoot = 1;
        }
        
        $t_rights = tableAccessCheck( $sqlo, 'DB_USER' );
        if ( $t_rights["admin"] == 1 ) {
            $this->editAllowRoot = 1; // user has role for update ...
        }
        
        $tmpallow = role_check_f ( $sqlo, "g.appPrefSave" );
        if ( $tmpallow == "deny" ) {
            $err_text = "Edit of user profile not allowed; due to your personal role-right 'g.appPrefSave'=DENY";
            $editAllow = 0;
            htmlErrorBox("Info", $err_text);
        }
        if ($_SESSION['sec']['appuser'] == "root") $editAllow=1;
        
        $this->editAllow = $editAllow;
    }
    
    private  function _shIcon($flag) {
        if ($flag>0) $icon = "<img src=\"images/but.checked.gif\">";
        else $icon= "<img src=\"images/but.checkno.gif\">";
        return $icon;
    }
    
    function showcreationProfile(&$sql) {
        
        $id = $this->user_id;
        
        echo "<br>\n";
        $groups = userGroupsGet($sql, $id );
        $mod_link='';
        if ( $this->editAllow ) {
            $mod_link = '[<a href="obj.db_user.manage_users_groups.php?user_id='.$id.'">Edit settings</a>]';
        }
        
        $color_bgactive   = "#DDDDFF";
        $topt=array();
        $topt["width"]   = "AUTO";
        $topt['morehead']=' '.$mod_link;
        htmlInfoBox( "Group access rights for new objects ", "", "open", "CALM", $topt );
        
        if ( !sizeof($groups) ) {
            //echo $mod_link."<br>\n";
            echo '- No creation settings.<br>';
            htmlInfoBox( "", "", "close" );
            return;
        }
        
        ?>
	<table cellspacing="1" border="0" frame="void">
	<tr valign=top>
		<th> user group </th>
		<th bgcolor=<?echo $color_bgactive?>>active</th>
		<th colspan="5">user group rights</th>
		</tr>
	
		<tr align="center">
		<td>&nbsp;&nbsp;</td>
		<td bgcolor=<?echo $color_bgactive?>>&nbsp; active &nbsp;</td>
		<td>&nbsp; read &nbsp;</td>
		<td>&nbsp; write &nbsp;</td>
		<td>&nbsp; insert &nbsp;</td>
		<td>&nbsp; delete &nbsp;</td>
		<td>&nbsp; entail &nbsp;</td>
		</tr>
    	<?php
    	foreach($groups as $gkey => $gval) {
    
    		$sql->query("SELECT name FROM user_group WHERE user_group_id=".$gkey);
    		$sql->ReadRow();
    		$tmpGrpName = $sql->RowData[0];
    			
    		// if ($gval["user_in_group"]) $tr_color=$color_userInGroup;
    
    		if ($gval["active"]) {
    
    			echo "<tr align=center>";
    			
    			echo "<td align=right><a href=\"edit.tmpl.php?t=USER_GROUP&id=".$gkey."\">".$tmpGrpName."</a></td>";
    			echo "<td bgcolor=".$color_bgactive.">".$this->_shIcon($gval["active"]);
    			echo "</td><td>".$this->_shIcon($gval["select"]);
    			echo "</td><td>".$this->_shIcon($gval["update"]);
    			echo "</td><td>".$this->_shIcon($gval["insert"]);
    			echo "</td><td>".$this->_shIcon($gval["delete"]);
    			echo "</td><td>".$this->_shIcon($gval["entail"]);
    			echo "</td></tr>\n";
    		}
    	}
    	echo "</table>\n";
    	htmlInfoBox( "", "", "close" );
		
    }
    
    /**
     * show user feature table row
     * @param $col1
     * @param $status
     * @param $addText
     * @param $addUrl
     * @param string $actions html links of actions tools
     * @param $notes
     * @param int $editAllow 0,1
     */
    private function _featTabOut($col1, $status, $addText, $addUrl, $actions, $notes, $editAllow) {
        
        echo "<tr bgcolor=#EFEFEF><td><B>".$col1."</B></td>";
        
        $statusOut = $status;
        if ($status=="OK") 		$statusOut = "<font color=green>$status</font>";
        if ($status=="MISSING") $statusOut = "<font color=#808020>$status</font>";
        
        echo "<td>".$statusOut."</td>";
        echo "<td >";
        $addUrlOut=0;
        if ($editAllow and $addUrl!=NULL) {
            echo '&nbsp;<a href="'.$addUrl.'">'.$addText."</a>";
            $addUrlOut=1;
        }
        if ($addUrlOut and $actions!=NULL) echo ' &nbsp; ';
        if ($actions!=NULL) echo "&nbsp;".$actions."&nbsp;";
        echo "</td>";
        echo "<td>".$notes."</td>";
        echo "</tr>\n";
    }
    
    private function _show_list($list, $table=NULL) {
        $retlist="";
        if (!sizeof($list)) return;
        $tmpkomma = "";
        
        foreach ($list as $id => $tmpgroup) {
            $urltxt = "<a href=\"edit.tmpl.php?t=".$table."&id=$id\">".$tmpgroup."</a>";
            $retlist .= $tmpkomma . $urltxt;
            $tmpkomma = ", ";
        }
        
        return ($retlist);
    }
    
    
    function userFeatTab($sql, $home_proj_id, &$user_groups, &$user_roles) {
        
        $id = $this->user_id;
        $editAllowRoot = $this->editAllowRoot;
        
        $role_cnt=count($user_roles);
        //$tmpUserGrpUrl = urlencode("DB_USER_ID=".$id);
        //$tmpRoleUrl    = urlencode("DB_USER_ID=".$id);
        $edit_icon='<img src="res/img/settings.svg" height=20 hspace=4>';
        
        echo "<table border=0 cellpadding=1 cellspacing=1  bgcolor=#D0D0FF>\n";
        echo "<tr><td colspan=4 bgcolor=#DFDFDF align=left>&nbsp;<font color=gray><B>Primary user configuration</B></font></td></tr>\n";
        
        // PASSWORD
        $this->_featTabOut("<img src=\"images/ico.password.gif\" border=0 TITLE=\"password\"> password",
        "", $edit_icon." Change", "obj.db_user.paw.php?user_id=".$id, "", "", $editAllowRoot);
        
        // HOME PROJECT
        $tmpInfo=NULL;
        if ( $home_proj_id ) {
            $tmpInfo["status"] = "OK";
            $tmpInfo["notes"]  = "<a href=\"edit.tmpl.php?tablename=PROJ&id=".$home_proj_id."\">home project</a>";
        } else {
            $tmpInfo["status"]  = "MISSING";
            $tmpInfo["url"]     = "obj.db_user.proj_home.php?user_id=".$id;
            $tmpInfo["urltxt"]  = "Create";
        }
        $this->_featTabOut("<img src=\"images/icon.PROJ.gif\" border=0 TITLE=\"project\"> home project",
            $tmpInfo["status"] , $tmpInfo["urltxt"], $tmpInfo["url"], "", $tmpInfo["notes"], $editAllowRoot );
        
        // USER ROLES
        $tmpInfo=NULL;
        if ( $role_cnt ) {
            $tmpInfo["status"] = "OK";
            $tmpInfo["actions"]  = "<a href=\"obj.db_user.show_all_rights.php?user_id=".$id."\">".
                '<img src="res/img/eye.svg" height=20 hspace=4> '.
                "Right Summary</a>";
            
            $tmpInfo["notes"]  = $this->_show_list($user_roles, "ROLE");
        } else {
            $tmpInfo["status"]  = "MISSING";
        }
        $tmpInfo["url"]     = "p.php?mod=DEF/o.DB_USER.action&action=role_edit&id=".$id;
        $tmpInfo["urltxt"]  = $edit_icon." Edit";
        $this->_featTabOut("<img src=\"images/icon.USER_ROLES.gif\" border=0 TITLE=\"role\"> Has roles",
            $tmpInfo["status"] , $tmpInfo["urltxt"], $tmpInfo["url"], $tmpInfo["actions"], $tmpInfo["notes"], $editAllowRoot );
        
        // USER GROUP
        $tmpInfo=NULL;
        if (count($user_groups)) {
            $tmpInfo["status"] = "OK";
            //if ($editAllowRoot) $tmpInfo["actions"]=
            //          '<a href="view.tmpl.php?t=DB_USER_IN_GROUP&tableSCond='.$tmpUserGrpUrl.'&condclean=1">List</a>';
            $tmpInfo["notes"]  = $this->_show_list($user_groups, "USER_GROUP");
        } else {
            $tmpInfo["status"]  = "MISSING";
        }
        $tmpInfo["url"]     = 'p.php?mod=DEF/o.DB_USER.action&action=grp_edit&id='.$id;
        $tmpInfo["urltxt"]  = $edit_icon." Edit";
        $this->_featTabOut("<img src=\"images/icon.USER_GROUP.gif\" border=0 TITLE=\"user group\"> Member of groups",
            $tmpInfo["status"] , $tmpInfo["urltxt"], $tmpInfo["url"], $tmpInfo["actions"] , $tmpInfo["notes"], $editAllowRoot );
        
        // personal group
        if ($_SESSION['globals']["security_write"]>0) {
            $myGroup = access_getPersGrp ($sql, $id);
            $tmpInfo = NULL;
            if ($myGroup>0) {
                $sql->query("select nick from db_user where DB_USER_ID=".$id);
                $sql->ReadRow();
                $userNick = $sql->RowData[0];
                $tmpInfo["status"] = "OK";
                $tmpInfo["notes"]  = "<a href=\"edit.tmpl.php?t=USER_GROUP&id=".$myGroup."\" >$userNick</a>";
            } else {
                $tmpInfo["status"] = "MISSING";
            }
            $this->_featTabOut( "<img src=\"images/icon.USER_GROUP_my.gif\" border=0 > Personal group",
                $tmpInfo["status"] , $tmpInfo["urltxt"], $tmpInfo["url"], "", $tmpInfo["notes"], $editAllowRoot );
        }
        
        echo "</table>\n";
        echo "<br>\n";
    }
}
     





class obj_edit_ext extends gObj_edit_feat_ABS {
    
    function rightWindow( &$sqlo, &$sqlo2) {
        $id = $this->id;
        $argu  = &$this->argu;

        require_once("insert.inc");
        require_once("o.DB_USER.subs2.inc");
		require_once("o.DB_USER.subs.inc");
		
		$main_lib = new o_DB_USER_feature_cls($sqlo, $id);
		$editAllow = $main_lib->editAllow;
        
        echo "<UL>";
      
        $param_nick = $argu["NICK"];
        
        
            
        $home_proj_id = oDB_USER_sub2::userHomeProjGet ($sqlo, $id);
        $user_groups  = oDB_USER_sub2::groupListGet($sqlo, $id);
        $user_roles   = oDB_USER_sub2::roleListGet ($sqlo, $id);
        
        
        $main_lib->userFeatTab($sqlo, $home_proj_id, $user_groups, $user_roles);
       
      
        if ( $_SESSION['sec']['db_user_id'] == $id ) { // root can not set this for other users
        ?>
            <a href="obj.db_user.settings.php"><img src="images/ic.user_app_set.gif" border=0 TITLE="user preferences"> Application preferences</a><br>
        <?
        } 
        
        if ($_SESSION['sec']['appuser'] == "root") {
            echo '<a href="obj.db_user.su.php?user_id='.$id.'" ><img src="images/ico.password.gif" border=0 >'.
    				 ' Login as user <b>'.$param_nick.'</b></A> (Change user identity)<br>';
            echo '<a href="p.php?mod=DEF/o.DB_USER.action&action=deactivate&id='.$id.'" ><img src="images/but.lock.in.gif" border=0 >'.
            		' Deactivate user</a><br>';
        }
        
        echo "<br>";
    	echo "<table border=0 bgcolor=#EFEFEF width=400>\n";
        echo "<tr><td colspan=4 bgcolor=#DFDFDF align=left>&nbsp;<font color=gray><B>";
        echo "<img src=\"images/icon.IMG.gif\" border=0 TITLE=\"image\"> User portrait\n";
    	echo "</B></font></td></tr></table><UL>\n";
       
        $pfilename = DB_userC::getIcon($id);
        if ( file_exists($pfilename) ) {
            echo "<img src=\"glob.obj.img_show.php?tablename=DB_USER&primid=".$id."&extension=jpg\" hspace=\"10\"><br>\n";
    		if ( $editAllow ) {
    			echo " <a href=\"obj.db_user.imgdel.php?id=$id\"><img src=\"images/but9.del.gif\" border=0> Delete
    			image</A><br>\n";
    			
    		}
    		echo "<br>";
        } else {
    		echo "<font color=gray>[no image]</font><br>\n";
    	}
            
        if ( $editAllow ) {
    		$tmpbackurlEnc = urlencode("edit.tmpl.php?t=DB_USER&id=".$id);
    		?>
    		<form ENCTYPE="multipart/form-data" ACTION="data.upload.php?tablename=DB_USER&ext=jpg&id=<?php
    		echo $id;
    		echo "&backurl=".$tmpbackurlEnc;
    		?>" METHOD=POST>
    		<INPUT TYPE="submit" VALUE="Upload image">
    			<INPUT NAME="userfile" TYPE="file" accept="*.*" >
    			<INPUT TYPE="hidden" name="MAX_FILE_SIZE" value="30000"> 
    		<br>
    		(<I>JPG-format, Max file size: 30 kbytes.)</I>
    		</form>
    		<?php
    
       }
       echo "</UL>";
       
       $main_lib->showcreationProfile($sqlo);
       echo "</UL>";
             
       return 0;
    }

}

