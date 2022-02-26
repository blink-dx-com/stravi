<?php
/**
 * change access for a selection of objects
 * @package glob.objtab.access_mod.php
 * @swreq   UREQ:0001150: g > modify access for list of objects
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param   
 * 		$tablename
	    $group_id
	   [$rig]      right array [NAME] = "", 1 : set, 2 : unset
       [$go]
       parx['noReopen'] : 0,1 : if user is root: it can ignore the action to reopen the objects
 */
session_start(); 

require_once ('reqnormal.inc');
require_once("sql_query_dyn.inc");
require_once("access_mani.inc");
require_once("glob.objtab.access.helper.inc");
require_once("f.visu_list.inc"); 
require_once ("visufuncs.inc");
require_once('access_info.inc');
require_once('o.CCT_ACCLOG.subs.inc');
require_once('f.accAndAcclog.inc');

class fAccessModList {

    function __construct($tablename) {
    	$this->AccHelperLib = new fAccessHelper();
    }
    
    
    /**
     * 
     * @param array $right_arr  array["RIGHT"] = 0, 1:set, 2:unset
     * @param array $rights  array["RIGHT"] = 0|1  as return value
     * @param int $group_remove  return value tells, if group is to be removed because no rights set
     * @return number
     */
    function rights_interprete ( $right_arr, &$rights, &$group_remove ) {
    
      $right_exists = 0;
      $group_remove = 0;
    
      foreach( $right_arr as $right_name=>$flag) { // traverse through rights which are to be set
    	if ($flag>0) {
    		$rights[$right_name] = ($flag<=1) ? 1 : 0; // if it is set, then it is 'on'
    		$right_exists += $rights[$right_name];
    		// echo " $right_name::".$rights[$right_name]." \n";
    	}
      }
     
      
      if ( $right_exists == 0 ) $group_remove=1;
    
    
      if ($group_remove) echo " remove group from right list.";
      echo "<br>";
      return 0;
    }

} // END class

class fAccessModLGui {
    // GUI class 
    function __construct( &$sql, $tablename, $group_id, $go, $parx) {
    	$this->tablename = $tablename;
    	$this->group_id = $group_id;
    	$this->go = $go;
    	$this->parx = $parx;
    	
    	$this->AccHelperLib = new fAccessHelper();
    	$this->accModLib    = new fAccessModList($tablename) ;
    	$this->accLogLib    = new oAccLogC();
    	$this->accAndLogLib = new fAccAndAcclogC($sql);
    }
    
    function init(&$sql) {
    	global $error;
    	$sql->query("SELECT name FROM user_group WHERE user_group_id = ".$this->group_id);
    	if ($error->printLast()) htmlFoot();
    	if (!$sql->ReadRow()) htmlFoot("Error", "Group_id '".$this->group_id."' does not exist!");
    	$this->group_name = $sql->RowData[0];
    }
    
    function showrights ( ) {
      
      $o_rights = $this->o_rights;
      reset($o_rights);
      
      $keyarr = array("<font color=gray>Untouched</font>", "<font color=green>ON (allow)</font>", "<font color=red>OFF (deny)</font>");
      $colorarr = array("#EFEFEF", "#DFFFDF", "FFDFDF");
      $i=0;
      while ($i<3) {
        echo "<tr bgcolor=".$colorarr[$i]."><td align=right>".$keyarr[$i]."</td>";
    	echo "<td>&nbsp;</td>";
    	$tmpselected = "";
    	if (!$i) $tmpselected = "checked";
    	foreach( $o_rights as $right_name=>$right) {
    	
    		echo "<td>";
    		echo "<input type='radio' name='rig[".$right_name."]' value='".$i."' ".$tmpselected."><br>";
    		echo "</td>";
    		
    	}
    	reset($o_rights);
    	echo "<tr>\n";
    	$i++;
      }
    }
    
    function formOpen(  ) {
    	// RETURN: $this->o_rights
    			
    	$tablename= $this->tablename;
    	$group_id = $this->group_id;
    	$this->o_rights = NULL;
    	
    	echo "<form name='editform' ACTION='".$_SERVER['PHP_SELF']."?tablename=".$tablename."&go=1' METHOD='POST'>\n";
    	echo '<input type="hidden" name="group_id" value="'.$group_id.'">';
    	
    	$this->o_rights = $this->AccHelperLib->right_names;
    	
    }
    
    function formClose() {
      echo "</form>\n";
    }
    
    function rights_interprete($rig) {
    	// set right mask
        $o_rights=array();
        $group_remove=0;
    	$this->accModLib->rights_interprete($rig, $o_rights, $group_remove);
    	$this->o_rights = $o_rights;
    	$this->group_remove = $group_remove;
    }
    
    function access_show ($manipulate, $chkbox_name) {
    	$o_rights = $this->o_rights;
    	if (!is_array($o_rights)) $o_rights = array(); // fall back
    	$o_rights_show = array_merge($this->AccHelperLib->right_names_minus_one, $o_rights);
    	$this->AccHelperLib->access_show ($o_rights_show, $manipulate, $chkbox_name);
    }
    
    function doloop( &$sql, &$sql2, $go ) {
    	/* FUNCTION:
    		  - analyse OR modify rights
    		  - get "haslog": check , if object has MANIPULATION rights and is under AUDIT-control
    		  - after successful access-right-manipulation: 
    				- if "haslog" : add audit-entry "reopen"
    	*/
    	global $error;
    	
    	$tablename = $this->tablename;
    	$group_id  = $this->group_id;
    	$o_rights  = $this->o_rights;
    	$group_remove = $this->group_remove;
    	$cntarray = NULL;
    	
    	
        $h_alog_reopen_id = oH_ALOG_ACT_subs::getH_ALOG_ACT_ID( $sql2, $tablename, "reopen" );
    	$addLogParx = array("action"=>$h_alog_reopen_id);
    	
    	$o_rightsHasMani = access_isManiMask( $o_rights );
    	$sqlopt=array();
    	$sqlopt["order"] = 1;
    	$sqlAfter  = get_selection_as_sql( $tablename, $sqlopt);
    	
    	$tmp_main_col = PrimNameGet2($tablename);
    	$sql->query("SELECT x.cct_access_id, a.db_user_id, x.$tmp_main_col FROM $sqlAfter", "doloop");
    	if ( $error->Got(READONLY) ) return;
    
    	
    	$cntarray["cnt"]             = 0;
    	$cntarray["deny_cnt"]        = 0;
    	$cntarray["entail_cnt"]      = 0;
    	$cntarray["entail_deny_cnt"] = 0;
    	$cntarray["haslog"]			 = 0;
    	$cntarray["haslogadd"]		 = 0;
    	//$group_arr       = array();
    	
    	$allowReopen = 1;
    	if ( $this->parx['noReopen']>0 )$allowReopen = 0;
    	
    	while ($sql->ReadRow()) {
    		
    		$loophaslog   = 0; // object has audit trail and has NO manipulation rights ?
    		$access_id    = $sql->RowData[0];
    		//$tmp_user     = $sql->RowData[1];
    		$obj_id       = $sql->RowData[2];
    		
    		
    		// check HAS_RIGHTS AND CCT_ACCLOG
    		$change_allow = access__allowed_to_chmod( $sql2, $access_id, $tablename ); 
    		if ( $error->Got(READONLY) ) return;	
    		
    		$accLogArr  = $this->accAndLogLib->objHasLog( $sql2, $access_id );
    		$loophaslog = ($accLogArr["haslog"]>0 and $accLogArr["hasMani"]<1 ) ? 1 : 0;
    		
    		if ($loophaslog) $cntarray["haslog"]++;
    		
    		$loopManiOk=0; // did a manipulation ?
    		switch ($change_allow) {
    			case "no":     $cntarray["deny_cnt"]++; break;
    			case "entail": $cntarray["entail_cnt"]++;
    				if ($go) {
    					access_write_with_check($sql2, $access_id, $o_rights, $group_id, $group_remove, "");
    					if ($error->got(CCT_WARNING_READONLY)) {
    						$tmpval = $error->getLast(); // ('access_rights', 100); // problems on entail
    						$cntarray["entail_deny_cnt"]++;		 
    					}
    					$loopManiOk=1;
    					if ( $error->Got(READONLY) ) return;
    				}
    				break;
    			case "yes": 
    				if ($go) {
    					access_write($sql2, $access_id, $o_rights, $group_id, $group_remove);
    					if ( $error->Got(READONLY) ) return;
    					$loopManiOk=1;
    				}
    				break;
    			default: htmlfoot("ERROR", " change_allowhas no proper value.");
    		}
    		
    		if ($loopManiOk AND $loophaslog AND $o_rightsHasMani and $allowReopen) {
    			// if new right has MANIPULATION-rights : add reopen-entry
    			$this->accLogLib->setObject( $sql2, $tablename, $obj_id, $access_id);
    			$this->accLogLib->addLogSub( $sql2, $addLogParx );
    			if ( $error->Got(READONLY) ) {
    				// may be same right
    				$errLast   = $error->getLast();
         			$error_txt = $errLast->text;
    				echo "Info: CCT_ACCESS_ID: $access_id, problem at addLogSub(): ".$error_txt."<br>";	
    				$error->reset();	
    			}
    			$cntarray["haslogadd"]++;
    		}
    		
    		$cntarray["cnt"]++;
    	}
    	
    	$cntarray["addsMani"] = $o_rightsHasMani;
    	
    	$this->cntarray = $cntarray;
    	
    	return ($cntarray);
    }
    
    function showRigInfo($cntarray) {
        $info_tab2=array();
    	$info_tab2[] = array("Selected elements", $cntarray["cnt"]);
    	$info_tab2[] = array("Set rights for group", "<img src=\"images/icon.USER_GROUP.gif\"> <a href=\"edit.tmpl.php?t=USER_GROUP&id=".
    				$this->group_id."\">".$this->group_name."</a>");
    	$info_tab2[] = array("Allowed", $cntarray["cnt"] - $cntarray["deny_cnt"]);
    	// $info_tab2[] = array("Allowed due to entail-right", $cntarray["entail_cnt"]);
    	if ($cntarray["entail_deny_cnt"] > 0) {
    		$info_tab2[] = array("<font color='#ff0000'>Rights not set because of entail-restrictions</font>", $cntarray["entail_deny_cnt"]);
    	}
    	$coltext= $cntarray["deny_cnt"] ? "<font color='#ff0000'>Denied for manipulation</font>" : "Denied for manipulation";
    	$info_tab2[] = array($coltext, $cntarray["deny_cnt"]);
    	
    	if ( $cntarray["haslog"] ) {
    		$info_tab2[] = array( "LOCKED and controlled", $cntarray["haslog"] );
    	}
    	
    	if ( $cntarray["addsMani"] ) {
    		$info_tab2[] = array( "New rights", "contain manipulation rights" );
    	}
    	
    	if  ($cntarray["haslogadd"] ) {
    		$info_tab2[] = array( "reopened", $cntarray["haslogadd"] );
    	}
    	
    	$topt = array("mode" => "easy", "keyTxtAlign"=>"right");
    	$header = NULL;
    	visufuncs::table_out( $header, $info_tab2, $topt );
    	
    	if (!$this->go AND $cntarray["haslog"]) {
    		echo "<br>";
    		htmlInfoBox( "Some objects will be reopended", $cntarray["haslog"]." objects are under audit control and have no manipulation rights.".
    				" These objects will be reopened if you add manipulation rights.", "", "WARN" );
    		echo "<br>";
    	}
    	
    	if ($this->go AND $cntarray["haslogadd"]) {
    		echo "<br>";
    		htmlInfoBox( "Some objects were reopended", $cntarray["haslogadd"]." objects were under audit control and have no manipulation rights.".
    				" These objects are reopened now.", "", "WARN" );
    		echo "<br>";
    	}
    }
    
    function showRightTable() {
    	
    	
    	echo '<table cellspacing="0" cellpadding="1" border="0"><tr><td bgcolor="#999999">';
    	echo '<table cellspacing="0" cellpadding="4" bgcolor="#ffffff" width="100%" border="0">';
    	echo '<tr bgcolor="#DDDDDD"><th>Rights</th><th><font size="1">&nbsp;</font></th>';
    	echo '<th>Read</th><th>Write</th><th>Delete</th><th>Insert</th><th>Entail</th></tr>';
    	if (!$this->go) {
    		
    		$this->showrights ();
    		// echo '<td><small>&lt;-- select to give/remove the right</small></td></tr>';
    		if (glob_isAdmin() and $this->cntarray["haslog"] ) {
    			// only for admins
    			echo '<tr bgcolor="#DDDDDD"><td>optional</td><td >&nbsp;</td>'.
    			 '<td colspan="5"><input name="parx[noReopen]" type="checkbox" value="1"> Do not reopen objects</td>';
    			echo "</tr>";
    		}
    		
    		echo '<tr bgcolor="#DDDDDD"><th>&nbsp;</th><th ><font size="1">&nbsp;</font></th>'.
    			 '<th colspan="5"><input type="submit" value="Set rights" class="yButton"></th>';
    	
    		//echo '<td><small>&lt;-- select the rights you want to change</small></td></tr>';
    		// echo "<tr>";
    	} else {
    		echo '<tr><td>new rights:</td><td>&nbsp;</td>';
    		$this->access_show (0, "rig");
    		echo "</tr>";
    	}
    	
    	echo "</tr>";
    	echo '</table></tr></td></table>'."<br>\n";
    }

}

// ******************************************************************** //

$parx 		= $_REQUEST['parx'];
$tablename  = $_REQUEST['tablename'];
$go  		= $_REQUEST['go'];
$rig		= $_REQUEST['rig'];
$group_id	= $_REQUEST['group_id'];

$tablename_nice = empty($tablename) ? "" : tablename_nice2($tablename);
$title  = "Set access rights for a list of objects in $tablename_nice";
$title2 = "Set access rights";
$error  = & ErrorHandler::get();
$sql    = logon2( $_SERVER['PHP_SELF'] );
$sql2   = logon2( );
$infoarr=array();
$infoarr["help_url"] = "access_info.html";

$infoarr["title"]    = $title;
$infoarr["title_sh"] = $title2;
$infoarr["form_type"]= "list";
$infoarr["obj_name"] = $tablename;
$infoarr["obj_cnt"]  = 1; 
$infoarr["locrow"]   = array( array("glob.objtab.access.php?t=".$tablename, "AccessInfo") );
		
$pagelib = new gHtmlHead();
$headarr     = $pagelib->startPage($sql, $infoarr);
$listVisuObj = new visu_listC();
echo "<ul>";

if (empty($tablename))            htmlFoot("ERROR", "Table name missing!");
if (!cct_access_has2($tablename)) htmlFoot("ERROR", "Objects are not business objects!");
if (empty($group_id))             htmlFoot("ERROR", "group_id missing!");
if (empty($go))                   $go = 0;
if (empty($rig))                  $rig = array();
// if (empty($set))                  $set = array();

$mainlib = new fAccessModLGui($sql, $tablename, $group_id, $go, $parx);
$mainlib->init($sql);


// check TABLE selection
$copt = array ("elemNum" => $headarr["obj_cnt"] ); // prevent double SQL counting
list ($stopFlag, $stopReason)= $listVisuObj->checkSelection( $sql, $tablename, $copt );
if ( $stopFlag<0 ) {
    htmlFoot("Attention", $stopReason." Please select elements of '".$tablename_nice."'!");
}
$isTabAdmin = role_admin_check ( $sql, $tablename );



if ($go) {
  echo "<B>Setting rights ...</B><br>";
  $mainlib->rights_interprete($rig);
}


$cntarray = $mainlib->doloop( $sql, $sql2, $go );
if (  $error->printAll() ) {
	htmlFoot();
}

$mainlib->showRigInfo($cntarray);

echo "<br>\n";

if (!$go) {
	$mainlib->formOpen();
}

$mainlib->showRightTable();

$rinopt = NULL;
if ($isTabAdmin) $rinopt = array("isTabAdmin"=>1);

$fAccInfoLib = new fAcessInfoC($tablename);
$fAccInfoLib->rightsInfo($sql, $rinopt);



if (!$go) {
  $mainlib->formClose();
} else {
  echo '<br><b>legend:</b><br>';
  echo '<img src="images/but.checked.gif"> - right set to <i>on</i><br>';
  echo '<img src="images/but.checkno.gif"> - right set to </i>off</i><br>';
  echo '<img src="images/but.checkgray.gif"> - right left untouched<br>';
  echo '<br><br>';
  echo "[<a href='glob.objtab.access.php?tablename=$tablename'>Back to access settings</a></B>]";
}

echo "</ul>";
htmlFoot("<hr>\n");

// ******************** end of page ********************
