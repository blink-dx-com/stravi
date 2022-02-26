<?php

require_once ('f.accAndAcclog.inc');
require_once ('access_info.inc');
require_once ('access_lock.inc');
require_once ("access_mani.inc");
require_once ('glob.obj.access.head.inc');
require_once ('glob.obj.access.rig.inc');
require_once ('glob.obj.access.proj.inc');
require_once 'o.H_ALOG_ACT.subs.inc';
require_once ('f.help.inc');
require_once ('subs/obj.proj.accMod.inc');
require_once ("class.obj.search_usage.inc");


require_once ('o.CCT_ACCLOG.gui.inc');


/**
 * - used by: glob.obj.access.php
 * - for business objects: get access rights
 * - this is the decision matrix: <pre>
 * user	        object	    hasStatus	possible action
 * ------------------------------------------------------
 * normal	    entail	    no			access-mani
 * normal	    entail	    yes			access-mani
 * normal	    noEntail 	no			read-only
 * normal	    noEntail	yes			read-only
 * table-admin	entail	    no			access-mani
 * table-admin	entail	    yes			access-mani
 * table-admin	noEntail    no			access-mani
 * table-admin	noEntail    yes		    reopen
 * ROOT         ANY         no          access-mani + reopen (since 2022-02-11)
 * </pre>
 * @package glob.obj.access.sub.inc
 * @swreq SREQ:0002180: g > one object > show/edit access matrix
 * @unittest exists: glob.obj.access.sub.inc:
 * 	- checkAuditState()
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class gObjAccessSub {

    var $tablename;
    var $object_is_bo;
    var $assoc_is ;
    var $basetable; // access based on this table
    var $cct_access_id;
    var $infox;
    var $flowlog;   // array of logged actions
    var $haslog;    // has audit-status? 0,1
    var $myGroup;   // personal group
    var $prim_id;
    var $setRights; // "entail", "yes", "no" ::: permission-flag to set rights: "yes", "no", "entail"
    private $objHasEntail; // 0 or 1
    private $last_H_ALOG_ACT=0;
    private $last_H_ALOG_ACT_name=NULL;
    
    function __construct( &$sql, $tablename, $objid) {
    	$this->tablename = $tablename;
    	$this->prim_id 	 = $objid;
    	$this->infox     = NULL;
    	$this->flowlog = NULL;
    	$this->accAndLogLib = new fAccAndAcclogC($sql);
    	$this->rightLib = new gObjAccRightsC( $sql, $tablename); // $mainRigLib
    }
    
    function _addActionLog($text) {
    	$this->flowlog[] = $text;
    }
    
    function getActionLog() {
    	return ($this->flowlog);
    }
    
    function showNavTab(){
    	$accHeadLib = new gObjAccessHead( 'perm', $this->tablename, $this->prim_id );
    	$accHeadLib->showNavTab();
    }
    
    function _setMyGroup(&$sql) {
    
    	$this->myGroup = access_getPersGrp ($sql, $_SESSION['sec']['db_user_id'] );
    	$this->rightLib->setMyGroup($this->myGroup);
    	
    }
    
    
    
    function show_missRoleRight( &$t_rights, $basetable_nice) {
    	// show missing role rights
    	
    	
    	$role_show=0;
    	if (!$t_rights["read"] )   $role_show=1;
    	if (!$t_rights["write"] )  $role_show=1;
    	if (!$t_rights["delete"] ) $role_show=1;
    	
    	if ($role_show) {
    		echo "<B>Role rights for user ".$_SESSION['sec']['appuser']."</B> (general rights for object type '$basetable_nice')<blockquote>\n";
    	
    		
    		echo '<table cellspacing="0" cellpadding="1" border="0"><tr><td bgcolor="#808080">';
    		echo "<table cellspacing='0' cellpadding='4' bgcolor='#ffffff' width='100%' border='0'>\n";
    		echo "<tr bgcolor='#DDDDDD'>";
    		echo "<th>Read</th><th>Write</th><th>Delete</th></tr>\n";
    	
    		echo "<tr>";
    	
    		echo "<td align='center'><img src='images/but.".($t_rights["read"] ? "checked" : "checkno").".gif'></td>";
    		echo "<td align='center'><img src='images/but.".($t_rights["write"] ? "checked" : "checkno").".gif'></td>";
    		echo "<td align='center'><img src='images/but.".($t_rights["delete"] ? "checked" : "checkno").".gif'></td>";
    	
    		echo "</tr>\n";
    	
    		echo "</table>\n";
    		echo "</tr></td></table>\n";
    		echo "<small><b>Write</b> controls also the accession permission for
    		'Insert' (reflects  feature list elements).<br></small>\n";
    		echo "<I>This table is only shown, if one right is missing.</I><br>\n";
    		echo "</blockquote>\n";
    	}
    }
    
    function rightManageHints(&$sql, $accdata, $myGroup, $cct_access_id) {
    	
    	/* if ( !$myGroup ) {
    		echo "<img src='images/i13_info.gif'> &nbsp;&nbsp; You have no 'personal user group'. Please ask the admin!<br>\n";
    	} 
    	*/
    	if ( ($_SESSION['globals']["security_write"]>0) ) {
    	
    		$tableAdminInfo =  $this->get_admins( $sql );
    		htmlInfoBox( "Group admins", 
    			"The users  ".$tableAdminInfo." are 'table admins' to add group rights. ".
    			"<a href=\"help/robo/g.accWriteDataSecurity.html\" target=help>See also help ...</a>",
    			"", "INFO" );
    		echo "<br>\n";
    		
    	}
    }
    
    
    /**
     * set some variables of class
     * @global  $this-> ... <pre>
    		prim_id
    		basetable
    		basetable_nice
    		mothertable
    		mothertable_nice
    		assoc_is      - ok
    		object_is_bo  - ok
    		</pre>
     */
    function setInitVars(&$sql) {
    	
    	
    	$tablename = $this->tablename;
    	$prim_id = $this->prim_id ;
    	
    	// Init
    	$this->object_is_bo = 0;
    	$this->assoc_is     = 0;
    	$this->basetable    = $tablename; // access based on this table
    	
    	
    	if ($_SESSION['globals']["security_write"]>0) {
    		$this->_setMyGroup($sql);
    	}
    	// Actions
    	$tablename_nice = tablename_nice2($tablename);
    	$this->basetable_nice = $tablename_nice;
    	$this->mothertable    = mothertable_get2($tablename);
    	
    	if (cct_access_has2($tablename)) $this->object_is_bo=1;
    	if ( !$this->object_is_bo ) {
    		if ($this->mothertable!="") {
    			if (cct_access_has2($this->mothertable)) {
    				$this->assoc_is = 1;
    				$mothertable_nice    = empty($this->mothertable) ? 'object': tablename_nice2($this->mothertable);
    				$this->basetable = $this->mothertable;
    				$this->basetable_nice = $this->mothertable_nice;
    				
    			} else {
    				htmlFoot('ERROR', 'Table &quot;'.$tablename_nice.'&quot; is a system object! There are no access-rights for this table.');
    			}
    		} else {
    			htmlFoot('ERROR', 'Table &quot;'.$tablename_nice.'&quot; is a system object! There are no access-rights for this table.');
    		}
    	}
    }
    
    
    /**
     * get CCT_ACCESS_ID
     * OUTPUT:
    		$this->cct_access_id
    		$this->most_imp_col_value
    		$this->setRights
     * @param $sql
     */
    function get_cct_access_id( &$sql ) {
    
    	global $error;
    	
    	$basetable = $this->basetable;
    	$prim_id   = $this->prim_id;
    	$this->objHasEntail = NULL;
    	$this->setRights    = 'no';
    	
    	$prim_name         = PrimNameGet2($basetable);
    	$most_imp_col_name = importantNameGet2($basetable);
    	
    	$sql->query("SELECT cct_access_id, $most_imp_col_name ".
    		  "FROM $basetable WHERE $prim_name = $prim_id");
    	if ($error->printLast()) htmlFoot();
    	if ($sql->ReadRow()) {
    		$cct_access_id      = $sql->RowData[0];
    		$most_imp_col_value = $sql->RowData[1];
    	} else {
    		htmlFoot("ERROR", $this->basetable_nice." with id=".$prim_id." does not exist.");
    	}
    	
    	$this->cct_access_id = $cct_access_id;
    	$this->most_imp_col_value = $most_imp_col_value;
    	
    	/**
    	 * "no"     user not allowed to set rights for bo
         * "yes"    is table-admin
         * "entail" object has the entail right for user
    	 * @var $this->setRights
    	 */
    	$this->setRights = access__allowed_to_chmod ( $sql, $cct_access_id, $basetable );
    	if ($this->assoc_is) {
    		$this->setRights =  "no"; // not for ASSOC elements; change them on the mother object !!!
    		return; // no further analysis
    	}
    	
    	// analyse entail-rigth
    	$this->objHasEntail = access__userHasEntail( $sql, $cct_access_id );
    	
    	// analyse audit status ...
    	$answer       = $this->accAndLogLib->objHasLog( $sql, $cct_access_id );
    	$this->haslog = $answer["haslog"];
    	$this->last_H_ALOG_ACT_id   = $answer["lastId"]; // without reopen
    	$this->last_H_ALOG_ACT_name = NULL;
    	if ($this->last_H_ALOG_ACT_id) {
    	   $this->last_H_ALOG_ACT_name = oH_ALOG_ACT_subs::statusID2Name($sql, $this->last_H_ALOG_ACT_id);
    	}
    	
    	return ($cct_access_id);
    }
    
    /**
     * check audit state
     * INPUT: 
     *   $this->haslog 
     *   $this->setRights
     *   $this->objHasEntail
     * OUTPUT: $this->specialAct
     * @param  $sql
     * @unittest: exists
     */
    function checkAuditState(&$sql) { 
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
    	$this->specialAct = ""; 
    	
    	/**
    	 * see decision matrix ...
    	 */
    	if ($this->haslog AND 
    	    $this->setRights=="yes" and 
    	    !$this->objHasEntail and 
    	    $this->last_H_ALOG_ACT_name!='started'
    	) {
    		$this->specialAct = "reopen";
    		$this->setRights  = "no"; // deny rights manipulation
    	}
    	if (glob_isAdmin()) {
    	    # Admin is allowed to set rights !!!
    	    $this->setRights  = "yes"; 
    	}
    	
    	debugOut('(280) haslog:'.$this->haslog.' setRights:'.$this->setRights.' objHasEntail:'.$this->objHasEntail. 
    	    ' last_H_ALOG_ACT_name:'. $this->last_H_ALOG_ACT_name.' specialAct:'.$this->specialAct, 
    	    $FUNCNAME,2);
    }
    
    /**
     * show main RIGHT-table
     * @global	$this->basetable, 
     * @global $this->infox (part) OUTPUT
     * @param $sql
     * @param $sql2
     * @param $o_rights
     * @param $t_rights
     * @return ---
     */
    function mainForm( &$sql,&$sql2,$o_rights,$t_rights) {
    	global $error;
    	
    	$tablename = $this->tablename;
    	$cct_access_id = $this->cct_access_id;
    	$prim_id   = $this->prim_id;
    	$mainRigLib = &$this->rightLib;
    	
    //	if ($this->specialAct == "reopen" ) {
    //		$this->setRights  = "no"; // deny !!!
    //	}
    	$setRights = $this->setRights;
    	
    	$editRightsAllow=0;
    	if ($this->setRights != "no") {
    		$editRightsAllow=1;
    		echo "<form name='editform' style=\"display:inline;\" ACTION='".$_SERVER['PHP_SELF'].
    			"?t=$tablename&id=$prim_id' method='POST'>\n";
    	}
    	
    	$accColSpan = 6;
    	
    	// table has a special CSS-style: class='hoverTable'
    	echo '<table cellspacing="0" cellpadding="1" border="0"><tr><td bgcolor="#808080">';
    	echo "<table cellspacing='0' cellpadding='4' bgcolor='#ffffff' width='100%' border='0' class='hoverTable'>\n";
    	echo "<tr bgcolor='#808080'><td colspan =".$accColSpan."><B><font color=#E0E0E0 size=-1>&nbsp;Group Access List</font></b></td></tr>";
    	echo "<tr bgcolor='#DDDDDD'><th><font color=#707070>Groups</font></th>";
    	echo "<th>Read</th><th>Write</th><th>Delete</th><th>Insert</th><th>Entail</th></tr>\n"; 
    	$this->infox["accCheckRed"] = $mainRigLib->show_access_table_user ( "<img src=\"images/icon.DB_USER.gif\" title=\"user\">".
    					" <font color=#606060>summary for </font><b>".$_SESSION['sec']['appuser']."</B>",  $o_rights, $t_rights );
    	$mainRigLib->write_access_table_row_new_group($sql2, $o_rights, $setRights);
    	
    	$sql->query("SELECT user_group_id, name, SINGLE_USER FROM user_group WHERE user_group_id IN ".
    				"(SELECT user_group_id FROM cct_access_rights WHERE cct_access_id = $cct_access_id)");
    	$hasMyGroup=0;
    	while ($sql->ReadRow()) {
    		$user_group_id = $sql->RowData[0];
    		if ( $this->myGroup==$user_group_id) {
    			$hasMyGroup=1;
    		}
    		$group_rights = access_ch_group ($sql2, $this->basetable, $prim_id, $user_group_id);
    		$mainRigLib->write_access_table_row  ($sql2, $group_rights, $setRights, 
    			$sql->RowData[1], $user_group_id, $sql->RowData[2]);
    	}
    	
    	// show personal group, if not show before
    	if (!$hasMyGroup and $this->myGroup and $editRightsAllow) {
    		$user_group_id= $this->myGroup;
    		$tmpMyGrpName = obj_nice_name ( $sql, 'USER_GROUP', $this->myGroup ); 
    		$group_rights = access_ch_group ($sql2, $this->basetable, $prim_id, $user_group_id);
    		$mainRigLib->write_access_table_row  ($sql2, $group_rights, $setRights, 
    			$tmpMyGrpName, $user_group_id, 1);
    		echo '<input type="hidden" name="myGroupAct" value="1">'."\n";	
    	}
    	
    	if ( $editRightsAllow ) {
    		if ($tablename == 'PROJ') {
    			echo '<tr bgcolor="#DDDDDD"><td colspan="'.$accColSpan.'">';
    			$helpLib = new fHelpC();
    			$hlpopt=NULL;
    			$helpText = $helpLib->link_show("o.PROJ.acceditor1.html", "help", $hlpopt);
    			
    			echo '<font color=gray>Object options:</font> ['.$helpText.']'."<br />\n"; 
    			echo '<input type="checkbox" name="proj_recursive" value="1"> Set rights for <b>all</b> objects in this project and its subprojects.<br>';
    			echo '&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;';
    			echo '<input type="radio" name="proj_recursive_method" value="add" checked> Add these rights to the existing ones.<br>';
    			echo '&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;';
    			echo '<input type="radio" name="proj_recursive_method" value="set"> Set these rights as the <u>only</u> ones.<br>';
    			echo '</td></tr>';
    		}
    		
    	}
    	
    	if ($editRightsAllow or $this->specialAct == "reopen" ) {
    		echo "<tr bgcolor='#EFEFEF'><td align='right'>&nbsp;</td><td colspan='5'>"; // #DDDDDD
    		if ($editRightsAllow) {
    			echo "<input type='hidden' name='go' value='1'>";
    			echo "<input type='submit' class='yButton' value='save'>\n";
    			echo "</form>";
    		}
    		if ( $this->specialAct == "reopen" ) {
    			
    			echo "<form name='editform2' style=\"display:inline;\" ACTION='".$_SERVER['PHP_SELF'].
    				"?t=$tablename&id=$prim_id' method='POST'>\n";
    			echo "<input type='hidden' name='go' value='1'>";
    			echo "<input type='hidden' name='specialAct' value='reopen'>";
    			echo "<input type='submit' value='Reopen to me!' class='yButton'>";
    			echo "</form>\n";
    		}
    		echo '</td></tr>'."\n";
    	}
    
    	
    	echo "</table>\n";
    	echo "</tr></td></table>\n";
    	
    	
    
    }
    
    function _sumAccessVals($accessArray) {
    	if (!sizeof($accessArray)) return;
    	reset ($accessArray);
    	$valsum=0;
    	foreach( $accessArray as $val) {
    		$valsum=$valsum+$val;
    	}
    	
    	return ($valsum);
    }
    
    /**
     * change rights
     *
     * - do write-op after checking rights! (to prevent attack)
     * - throw error on problem
     * 
     * @param $sql
     * @param $sql2
     * @param $o_rights
     * @param $read_grp
     * @param $write_grp
     * @param $delete_grp
     * @param $insert_grp
     * @param $entail_grp
     * @param $new_groupid
     * @param int $proj_recursive 0,1 change rights in project recursive
     * @global $this->setRights
     * @return $allOK 0,1
     */
    function changeit(
    		&$sql,
    		&$sql2,
    		&$o_rights,
    		$read_grp,
    		$write_grp,
    		$delete_grp,
    		$insert_grp,
    		$entail_grp,
    		$new_groupid,
    		$proj_recursive,
    		$proj_recursive_method
    		) {
    	
    	global $error;
    	$FUNCNAME= 'changeit';
    	
    	$allOK = 1;
    	$cct_access_id = $this->cct_access_id;
    	$mainRigLib    = &$this->rightLib;
    		
    	if (empty($read_grp)) 		$read_grp = array();
    	if (empty($write_grp)) 		$write_grp = array();
    	if (empty($delete_grp)) 	$delete_grp = array();
    	if (empty($insert_grp)) 	$insert_grp = array();
    	if (empty($entail_grp)) 	$entail_grp = array();
    	if (empty($new_groupid)) 	$new_groupid = 0;
    	
    	$this->_addActionLog('new_groupid: '.$new_groupid);
    	
    	$grouparr = array(
    		"read" => $read_grp, 
    		"write" =>$write_grp,
    		"delete" =>$delete_grp,
    		"insert" =>$insert_grp,
    		"entail" =>$entail_grp
    		);
    	
    	if ($this->setRights == "no") {  // user has no rights to change rights but nevertheless he tried it
    		$error->set( $FUNCNAME, 3, "No right to change rights!");
    		return;
    	}
    	
    	$all_rights_arr = array(); // also used for proj_recurcive == 1
    	
    	$myGroup = $this->myGroup;
    	$myGroupInLoop=0;
    	
    	$sql->query("SELECT USER_GROUP_ID FROM cct_access_rights WHERE cct_access_id = $cct_access_id");
    	if ($error->printLast()) htmlFoot();
    	$this->_addActionLog('get all groups from this CCT_ACCESS_ID');
    	while ($sql->ReadRow()) {
    	
    		$group_id = $sql->RowData[0];
    		if ($myGroup==$group_id) $myGroupInLoop=1;
    		
    		$rights_in_form = array();
    		foreach($o_rights as $right_name => $dummy) {
    			$array_name = $right_name;
    			$rights_in_form[$right_name] = in_array($group_id, $grouparr[$array_name]);
    		}
    		
    		$all_rights_arr[$group_id] = $mainRigLib->convRights_form2array ($group_id, $o_rights, $grouparr);
    	
    		access_write($sql2,$cct_access_id,$all_rights_arr[$group_id],$group_id);
    		if ($error->Got(READONLY))  {
    			$error->set( $FUNCNAME, 1, 'error on object with CCT_ACCESS_ID='.$cct_access_id );
    			return;
    		}
    	}
    	
    	$this->_addActionLog('myGroup: '.$myGroup.' myGroupAlreadyInAccessList? '.$myGroupInLoop);
    	if ($myGroup and !$myGroupInLoop) {
    		$group_id = $myGroup;
    		$new_group_rights = $mainRigLib->convRights_form2array ($group_id, $o_rights, $grouparr);
    
    		if ( $this->_sumAccessVals($new_group_rights) > 0 ) { 
    			$this->_addActionLog('new myGroup-rights');
    			$all_rights_arr[$group_id] = $new_group_rights;
    			access_write($sql2, $cct_access_id, $new_group_rights, $group_id);
    			if ($error->Got(READONLY))  {
    				$error->set( $FUNCNAME, 1, 'error on object with CCT_ACCESS_ID='.$cct_access_id );
    				return;
    			}
    		} else $this->_addActionLog('no myGroup-rights!');
    	}
    	
    	// now insert data for new group
    	$new_group_rights = $mainRigLib->convRights_form2array ("new_group", $o_rights, $grouparr);
    	if ( ($this->_sumAccessVals($new_group_rights)>0) and !strcmp($new_groupid, "not_set") ) { 
    		// set values but no new group
    		htmlInfoBox( "WARNING", "You forgot to select a new group.", "", "WARN" );
    		echo "<br>";
    		return;
    	}
    	
    	if ($new_groupid > 0) { // check rigths for new group
    		$this->_addActionLog('write newGroup rights');
    		$all_rights_arr[$new_groupid] = $new_group_rights;
    		access_write($sql2, $cct_access_id, $new_group_rights, $new_groupid);
    		if ($error->Got(READONLY))  {
    			$error->set( $FUNCNAME, 1, 'error on object with CCT_ACCESS_ID='.$cct_access_id );
    			return;
    		}
    	} else $this->_addActionLog('no newGroup');
    	
    	if ($proj_recursive == 1) { 
    		// do rights setting recursivly through project
    		$projRecLib = new O_PROJ_accMod();
    		$projRecLib->setProj($this->prim_id, $this->most_imp_col_value, $this->cct_access_id);
    		$projRecLib->projectDo( $sql, $proj_recursive_method, $all_rights_arr );
    		echo '</ul>';
    		$errcnt = $projRecLib->getErrCnt();
    		if ($errcnt) {
    			$allOK=0; // show the messages, stop page
    			echo "<br><font color=red>Denied:</font> ".$errcnt." objects.<br />\n";
    		}
    		
    		if ($error->Got(READONLY))  {
    			$error->set( $FUNCNAME, 1, 'error on recursive project access manipulation.' );
    			return;
    		}
    		
    	}
    	
    	return $allOK;
    }
    
    function set_objHasEntail($data) {
        $this->objHasEntail=$data;
    }

}

