<?php
/**
 * Object creation settings: manage the preferences for NEW objects of the user
 * 
 * - DB_MODIFIED: tables: USER_PREF
 * - FORMAT: $mask: active select update insert delete entail
 * MAX number of ACTIVE groups: 7 (MAX_NUM_GROUP_ACT)
 * @package obj.db_user.manage_users_groups.php
 * @swreq UREQ:0002098: g > Object creation settings 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @global $_SESSION['userGlob']['umask.'.$group_id]
 * @param  
 *   $go : 0,1
 *   $action:
 *     'acc_set'
 *     'grp_new'
 *   $user_id   -- only allowed for admin
 *   gdel[]
     act_group_nrs[], 
     sel_group_nrs[], 
     upd_group_nrs[], 
     del_group_nrs[], 
     ins_group_nrs[], 
     ent_group_nrs[]
 *   $new_groupid : ID of new group
 * @output user_id, act_group_nrs[], sel_group_nrs[], upd_group_nrs[], del_group_nrs[], ins_group_nrs[], ent_group_nrs[]
  
 */


session_start(); 

require_once ("insert.inc");
require_once ("func_head.inc");
require_once ("db_access.inc");
require_once ("globals.inc");
require_once ("role.inc");
require_once ("o.DB_USER.subs2.inc");    
require_once ( "javascript.inc" );

class gObjCreaSet_gui {
	var $MAX_NUM_GROUP_ACT=7;
	var $MAX_NUM_GROUP_SAV=20;
	
	var $groups; 
	private $user_id;
	
	function __construct(&$sqlo, $user_id) {
	    $this->user_id   = $user_id;
		$this->getGroups($sqlo);
		$this->set_session_var=1;
		
	}
	
	function is_other_user() {
	    $this->_is_other_user=1;
	    $this->set_session_var=0;
	}
	
	function getGroups(&$sql) {
		$this->groups = userGroupsGet($sql, $this->user_id );
		return $this->groups;
	}
	
	
	function shIcon($tmpRights, $key) {
		if ($tmpRights[$key]>0) $icon = "<img src=\"images/but.checked.gif\">";
		else $icon= "<img src=\"images/but.checkno.gif\">";
		echo "<td>".$icon."</td>";
	}
	
	function rightTabx( $method, $groupname, $textarray, $colors) {
		echo "<tr align=center bgcolor=".$colors["tr"].">";
	
		if ($method=="info") {
		
			echo "<td bgcolor=".$colors["delgrp"].">&nbsp;";
			echo "</td>";
			echo "<td align=right>".$groupname."</td>";
			echo "<td bgcolor=".$colors["bgactive"].">&nbsp;</td>";
			echo "<td colspan=5>".$textarray[0]."</td>";
			echo "</tr>\n";
		}
	}

	
	/**
	 * do a precheck of settings
	 * @param $newVals : 
	 * 	  0 : check old vals
	 *    1 : check new vals
	 */
	function preCheck($newVals=0) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$groups        = $this->groups;
		
		$act_group_nrs = $_REQUEST['act_group_nrs'];
		$gdel 		   = $_REQUEST['gdel'];
		if (!is_array($gdel)) 		   $gdel 		  = array();
	    if (!is_array($act_group_nrs)) $act_group_nrs = array();
		
		$cntActive=0;
		
		if ($newVals) {
			foreach($groups as $gkey => $gval) {
		
		      if ( in_array($gkey, $gdel) ) { // remove form list
				 // delete
		      } else {
				$tmpAct = in_array($gkey, $act_group_nrs) ? 1 : 0;
				$cntActive = $cntActive + $tmpAct;
		      }
		    }
			if ($cntActive>=$this->MAX_NUM_GROUP_ACT) {
		    	return array('ok'=>-1, 'text'=>'Too many ACTIVE groups. Max:'.$this->MAX_NUM_GROUP_ACT);
		    }
		    
		} else {
			// check old values
			
			foreach($groups as $gkey => $gval) {
		
				$tmpAct = $gval['active']>0 ? 1 : 0;
				$cntActive = $cntActive + $tmpAct;
		      
		    }
			if ($cntActive>=$this->MAX_NUM_GROUP_ACT) {
		    	return array('ok'=>-1, 'text'=>'Too many ACTIVE groups. Max:'.$this->MAX_NUM_GROUP_ACT);
		    }
		}
		
	    
	    return array('ok'=>1);
	}
	
	function writeUmask($sql, $new_groupid, $action) {
	    $user_id = $this->user_id;
	    oDB_USER_sub2::writeUmask ($sql, $user_id, $new_groupid, $action, $this->set_session_var);
	}
	
	function saveGroups(&$sqlo) {
		
		$groups = $this->groups;
		$user_id= $this->user_id;
		
		$gdel = $_REQUEST['gdel'];
		$act_group_nrs = $_REQUEST['act_group_nrs'];
		$sel_group_nrs = $_REQUEST['sel_group_nrs'];
		$upd_group_nrs = $_REQUEST['upd_group_nrs'];
		$ins_group_nrs = $_REQUEST['ins_group_nrs'];
		$del_group_nrs = $_REQUEST['del_group_nrs'];
		$ent_group_nrs = $_REQUEST['ent_group_nrs'];
	
		
		if (!is_array($gdel)) 		   $gdel 		  = array();
	    if (!is_array($act_group_nrs)) $act_group_nrs = array();
	    if (!is_array($sel_group_nrs)) $sel_group_nrs = array();
	    if (!is_array($upd_group_nrs)) $upd_group_nrs = array();
	    if (!is_array($ins_group_nrs)) $ins_group_nrs = array();
	    if (!is_array($del_group_nrs)) $del_group_nrs = array();
	    if (!is_array($ent_group_nrs)) $ent_group_nrs = array();
	    
	    foreach($groups as $grp_id => $gval) {
	
    	      if ( in_array($grp_id, $gdel) ) { // remove form list
    	          oDB_USER_sub2::writeUmask ($sqlo, $user_id, $grp_id, NULL, $this->set_session_var); 
    			  echo "<font color=green><B>Success:</B></font> Removed a group.<br>\n";
    	      } else {
    			$umask  = in_array($grp_id, $act_group_nrs) ? "active " : "";
    			$umask .= in_array($grp_id, $sel_group_nrs) ? "select " : "";
    			$umask .= in_array($grp_id, $upd_group_nrs) ? "update " : "";
    			$umask .= in_array($grp_id, $ins_group_nrs) ? "insert " : "";
    			$umask .= in_array($grp_id, $del_group_nrs) ? "delete " : "";
    			$umask .= in_array($grp_id, $ent_group_nrs) ? "entail " : "";
    	
    			oDB_USER_sub2::writeUmask ($sqlo, $user_id, $grp_id, $umask, $this->set_session_var);
    	      }
	    }
	    
	    echo "<font color=green><B>Success:</B></font> Updated group preferences.<br>\n";
	}
	
	function form_new_group($changeAllow) {
	    
	    $user_id = $this->user_id;
	    $topt=array();
	    $topt["width"] = "AUTO";
	    htmlInfoBox( "Add a new group", "", "open", "CALM", $topt );
	    ?>
        <form name="editform" style="display:inline;"  method="post" action="<?php echo $_SERVER['PHP_SELF'] ?>">
        new group:
        <?php
    	$jsFormLib = new gJS_edit();
    	$fopt = array("noshDel"=>1);
    	$answer = $jsFormLib->getAll('USER_GROUP', 'new_groupid', '', NULL,  0, $fopt);
    	echo $answer;
        
        ?>
        <input type="hidden" name="user_id" value="<?php echo $user_id ?>">
        <input type="hidden" name="go" value="1">
        <input type="hidden" name="action" value="grp_new">
        <?php
        if ($changeAllow != "deny") echo '&nbsp;&nbsp;&nbsp;<input type=submit value="Add Group">';
        ?>
        </form>   
        <?php
        htmlInfoBox( "", "", "close" );
    }

}


$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot();

$title        	  = "Object creation settings: access rights";
$title_sh         = "Object creation settings";
$color_bgactive   = "#DDDDFF";  // was #D0D0FF
//$color_userInGroup= "#8899FF";
$color_delgrp	  = "#FFDDDD";  // red
$colspan          = 8;
$colors			  = array("bgactive"=>$color_bgactive, "userInGroup"=> "#8899FF", "delgrp"=> "#FFDDDD", "tr"=>"#ffffff" );


$new_groupid   = $_REQUEST['new_groupid'];
$go            = $_REQUEST['go'];
$action        = $_REQUEST['action'];

$my_user_id    = $_SESSION['sec']['db_user_id'];
$input_user_id = $_REQUEST['user_id'];
if (!$input_user_id) $input_user_id=$my_user_id;

if ( $input_user_id == $my_user_id ) { // access_check()
    
    $infoarr=array();
    $infoarr["title"]    = $title;
    $infoarr["title_sh"]    = $title_sh;
    $infoarr["form_type"]= "tool";
    $infoarr["help_url"] = "o.DB_USER.manage_users_groups.html";
    $infoarr["icon"]     = "images/ic24.userprefs.png";
    $infoarr["locrow"]   = array( array("obj.db_user.settings.php", "MyAccount")	);
} else {
    
    //$user_fea  = glob_ObjDataGet( $sql, 'DB_USER', $input_user_id, array('FULL_NAME'));
    $title_sh .=' for a user';

    $infoarr=array();
    $infoarr["title"]    = $title;
    $infoarr["title_sh"] = $title_sh;
    $infoarr["form_type"]= "obj";
    $infoarr['obj_name'] = 'DB_USER';
    $infoarr['obj_id']   = $input_user_id;
    $infoarr['checkid']  = 1;
    $infoarr["show_name"]= 1;
}
$pagelib = new gHtmlHead();
