<?php
/**
 * USER_GROUP extension; called by edit.tmpl.php
 * @package obj.user_group.xedit.php
 * @swreq   
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/www/pionir/obj.user_group.xedit.php 59 2018-11-21 09:04:09Z $
 */


require_once ("f.objview.inc");

class oUSER_GROUP_xedSub {
	
    function __construct($id, &$argu) {
    	$this->id=$id;
    	$this->argu=$argu;
    }
    
    function _tableColumn($title,$notes=NULL) {
    	echo '<span style="color: #08457E; font-size:1.2em; font-weight:bold;">'.$title.'</span> '.$notes.'<br />'."\n";
    	echo '<img src="0.gif" width=1 height=5>'."<br />\n";
    	
    }
    
    function group_members(&$sql, $id) {
    	// echo "<li> List of users: </li>";
    	echo "<ul>";
    	$sqls = "select g.DB_USER_ID, u.NICK, u.FULL_NAME from DB_USER_IN_GROUP g, DB_USER u ".
    			" where g.user_group_id=".$id. " AND g.db_user_id=u.db_user_id order by UPPER(u.nick)";
    	$sql->query($sqls);
    	$cnt=0;
    	$MAX_SHOW = 30;
    	while ( $sql->ReadRow() ) {
    		
    		if ($cnt>$MAX_SHOW) {
    			echo " <b>... more ...</b><br>\n";
    			break;
    		}
    		
    		$tmpid   = $sql->RowData[0];
    		$tmpnick = $sql->RowData[1];
    		$tmpFull = $sql->RowData[2];
    		echo "<a href=\"edit.tmpl.php?t=DB_USER&id=".$tmpid."\">".
    			"<img src=\"images/icon.DB_USER.gif\" border=0> $tmpnick</a> ";
    		if ($tmpFull!=NULL) echo "&nbsp;(".$tmpFull.")";
    		echo "<br>\n";
    		
    		$cnt++;
    	}
    
    	echo "</ul>\n";
    }
    
    /**
     * analyse ENTAIL-right
     * @param object $sqlo
     */
    private function _isExtGroupAdmin($sqlo) {
    	$obj_rights = access_check ($sqlo, 'USER_GROUP', $this->id);
    	if ($obj_rights['insert']>0) return 1;
    	else return 0;
    }
    
    /** 
     * get group admins
     * @param object $sqlo
     * @return array(nice user names)
     */
    private function _getGroupAdmins(&$sqlo) {
    	
    	$hasInsertArr=array();
    	
    	// main admin ....
    	$admin_id = $this->argu["DB_USER_ID"];
    	$nicename_mainAdmin=NULL;
    	if ($admin_id) {
    		$nicename_mainAdmin = glob_elementDataGet( $sqlo, 'DB_USER',  'DB_USER_ID', $admin_id, 'FULL_NAME');
    		$hasInsertArr[] = $nicename_mainAdmin;
    	}
    	
    	$cct_access_id = glob_elementDataGet( $sqlo, 'USER_GROUP',  'USER_GROUP_ID', $this->id, 'CCT_ACCESS_ID');
    	if(!$cct_access_id) return $hasInsertArr;
    	
    	$groups_rights = access_getAllRights( $sqlo, $cct_access_id );
    	if (!is_array($groups_rights)) return $hasInsertArr;
    	
    	
    	foreach($groups_rights as $group=>$infoarr) {
    		
    		if($infoarr['insert']) {
    			$sqlo->Quesel("SINGLE_USER, NAME from USER_GROUP where USER_GROUP_ID=".$group);
    			$sqlo->ReadRow();
    			$SINGLE_USER=$sqlo->RowData[0];
    			$NAME  = $sqlo->RowData[1];
    			
    			$nicename=$NAME;
    			if ($SINGLE_USER) {
    				// get user name
    				$nicename = glob_elementDataGet( $sqlo, 'DB_USER',  'NICK', $NAME, 'FULL_NAME');
    			}
    			
    			if ($nicename!=$nicename_mainAdmin) {
    				$hasInsertArr[]=$nicename;
    			}
    		}
    	}
    	
    	return $hasInsertArr;
    	
    	
    }
    
    function userPart(&$sql) {
    	
    	
    	$id= $this->id;
    	$admin_id = $this->argu["DB_USER_ID"]; 
    	$tmp1_SelUsers = rawurlencode ("x.DB_USER_ID IN ( select DB_USER_ID from DB_USER_IN_GROUP where user_group_id=".$id.")" );
    	
    	$groupadmins = $this->_getGroupAdmins($sql);
    	$this->_tableColumn('Group Admins', '(insert-rights on group)');
    	if (!empty($groupadmins)) {
    		
    		$komma='';
    		foreach($groupadmins as $oneAdmin) {
    			echo $komma.$oneAdmin;
    			$komma=', ';
    		}
    		echo '<br>'."\n";
    	} else {
    		echo 'none.';
    	}
    	echo '<br>'."\n";
    	
    	$this->_tableColumn('Group members');
    	
    	
    	
    	if ($admin_id == $_SESSION['sec']['db_user_id'] OR glob_isAdmin() OR $this->_isExtGroupAdmin($sql) ) {
    		
    		echo '[<a href="view.tmpl.php?t=DB_USER&cctgo[sel]='.
    			urlencode('obj.user_group.addusers.php?id='.$id.'&action=add').
    			'&cctgo[info]='.urlencode('Add selected users to group').'"><B>Add users</B></a>]  &nbsp;';
    		echo '[<a href="view.tmpl.php?t=DB_USER&cctgo[sel]='.
    			urlencode('obj.user_group.addusers.php?id='.$id.'&action=del').
    			'&condclean=1&tableSCond='.$tmp1_SelUsers.'&cctgo[info]='.urlencode('Remove selected users from group').
    			'"><B>Delete users</B></a>]  &nbsp;';
    		
    	} else {
    		echo "<font color=gray>Add/Delete by <b>group-admin</b>)</font> &nbsp;"; 
    	}
    	echo "[<a href=\"view.tmpl.php?t=DB_USER&tableSCond=".$tmp1_SelUsers."&condclean=1\">".
    			"users in list view</a>]<br />\n";
    	
    	$this->group_members($sql, $id);
    }
    
    function _getSubGroups(&$sqlo) {
    	$subGroups = NULL;
    	$MAX_CNT= 40;
    	$sqlsel = "USER_GROUP_ID from USER_GROUP where MASTER_GROUP_ID=".$this->id. ' order by NAME';
    	$sqlo->Quesel($sqlsel);
    	$cnt=0;
    	while ( $sqlo->ReadRow() ) {
    	    $tmpid = $sqlo->RowData[0];
    	    if ($cnt>$MAX_CNT) {
    	    	$subGroups[] = '... more';
    	    	break;
    	    }
    	    $subGroups[] = $tmpid;
    	    $cnt++;
    	}
    	return $subGroups;
    }
    
    function groupHierarchy(&$sqlo) {
    	$id = $this->id;
    	
    	$this->_tableColumn('Group hierarchy');
    	
    	
    		
    	// get edit-link of BO + NAME + icon (object)
    	$objLinkLib = new fObjViewC();
    	
    	if (!glob_column_exists('USER_GROUP', 'MASTER_GROUP_ID') ) {
    		echo ' ... not yet implemented.';
    		return;
    	}
    	
    	$subGroups = $this->_getSubGroups($sqlo);
    	$hasMaster = 0;
    	
    	$treeIcon  = '<img src="images/ic.tree3_13.gif">';
    	$spaceIcon = '<img src="0.gif" width=16 height=1>';
    	$separator = NULL;
    	if ($this->argu["MASTER_GROUP_ID"]) {
    		$hasMaster=1;
    		$htmlTemp = $objLinkLib->bo_display( $sqlo, 'USER_GROUP', $this->argu["MASTER_GROUP_ID"] );
    		echo $htmlTemp."<br />"."\n";
    		$separator = $treeIcon; 
    	}
    	
    	$htmlTemp = $objLinkLib->bo_display2( $sqlo, 'USER_GROUP', $id); 
    	echo $separator . '<span style="font-size:1.4em; font-weight:bold;">'.$htmlTemp."</span><br />"."\n";
    	if ($hasMaster) $separator = $spaceIcon . $separator;
    	else $separator = $treeIcon;
    	
    	if (!empty($subGroups)) {
    		
    		$cnt=0;
    		foreach( $subGroups as $dummy=>$oneGroupId) {
    			if (!is_numeric($oneGroupId) ) {
    				echo $separator . " ..... <br />"."\n";
    				break;
    			}
    			$htmlTemp = $objLinkLib->bo_display( $sqlo, 'USER_GROUP', $oneGroupId );
    			echo $separator . $htmlTemp."<br />"."\n";
    			$cnt++;
    		}
    		
    	}
    	
    	if (!$hasMaster and empty($subGroups)) {
    		echo 'Info: This group is not part of a hierarchy.<br />'."\n";
    	}
    }


}


class obj_edit_ext extends gObj_edit_feat_ABS {
    
    function rightWindow( &$sqlo, &$sqlo2) {
        $id = $this->id;
        $argu  = &$this->argu;
        
    	$subLib = new oUSER_GROUP_xedSub($id,$argu);
    	
    	echo "<ul>\n";
    	
    	echo '<table border=0 cellspacing=1 cellpadding=0><tr valign=top>'."\n";
    	echo '<td width=300>'."\n";
    	$subLib->userPart($sqlo);
    	echo '</td>';
    	echo '<td width=3 background="images/bg.dotline.gif"><img src="0.gif" width=1 height=20>';
    	echo '</td>';
    	echo '<td width=10><img src="0.gif" width=10 height=20>';
    	echo '</td>';
    	echo '<td width=300>'."\n";
    	$subLib->groupHierarchy($sqlo);
    	echo '</tr></table>'."\n";
    	
    	echo "</ul>\n";
    	return 0;
    }

}

