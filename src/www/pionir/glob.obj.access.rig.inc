<?php


/**
 * show group rigths
 * mainly used by glob.obj.access.sub.inc
 * @package glob.obj.access.rig.inc
 * @swreq UREQ:xxxxxxxxxxxx
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/www/pionir/glob.obj.access.rig.inc 59 2018-11-21 09:04:09Z $
 */
class gObjAccRightsC {

function __construct( &$sql, $tablename) {
	$this->tablename = $tablename;
}

public function setMyGroup($myGroup) {
	$this->myGroup = $myGroup;
}

/**
 * if group if a singel user: get full name
 * @param  $sqlo
 * @param int $group_id
 * @return string (can be NULL)
 */
private function getFullNameOfGrp(&$sqlo, $group_id)  {
	
	$sqlo->Quesel("FULL_NAME from DB_USER where DB_USER_ID in (select DB_USER_ID from DB_USER_IN_GROUP where USER_GROUP_ID=".$group_id.")");
	$sqlo->ReadRow();
	$fullname = $sqlo->RowData[0];
	return $fullname;
}

/**
 * show group name + optional user-fullname and link
 * INPUT: $this->myGroup
 * @param $group_id
 * @param $group_name
 * @param $is_single_user
 * @param $is_group_select 0,1
 * @return html-string
 */
private function groupOut( &$sqlo, $group_id, $group_name, $is_single_user, $is_group_select=0) {
	
	
	$theimg = "<img src=\"images/icon.USER_GROUP.gif\" title=\"group\">";
	$group_name_out = $group_name;
	
	if ($is_single_user) {
		$theimg = "<img src=\"images/icon.USER_GROUP_my.gif\" title=\"personal group\">";
		// get full name of user
		$userFullName = $this->getFullNameOfGrp($sqlo, $group_id);
		if ($userFullName!=NULL) $group_name_out .= ' <span style="color:gray;">('.$userFullName.')</span>';
	}
	
	if ($this->myGroup AND ($this->myGroup==$group_id)) $theimg = "<img src=\"images/icon.USER_GROUP_my.gif\" title=\"your personal group\">";
	
	if (!$is_group_select) {
		$retval = $theimg." <a href=\"edit.tmpl.php?t=USER_GROUP&id=$group_id\">$group_name_out</a>" ;
	} else {
		$retval = $theimg." ".$group_name_out ; // no link around ...
	}
	return ($retval);
}

/**
 * show one ROW of one group (name, rights)
 * @param object $sqlo
 * @param $rights
 * @param $setRights
 * @param $group_name can be a complex HTML-text
 * @param $group_id
 * @param $is_single_user  // 0|1
 */
public function write_access_table_row (&$sqlo,  $rights, $setRights, $group_name, $group_id, $is_single_user,
	$is_group_select=0
	) {
	
	if (!is_object($sqlo)) {
		sys_error_my('input $sqlo missing');
	}
	
	echo "<tr><td>".$this->groupOut($sqlo, $group_id,$group_name, $is_single_user, $is_group_select)."</td>";
	  foreach($rights as $right => $value) {
		if (($setRights != "no") && $group_id) {
		  echo "<td align='center'><input type='checkbox' name='${right}_grp[]'".($value ? " checked" : "")." value='$group_id'></td>";
		} else {
		  echo "<td align='center'><img src='images/but.".($value ? "checked" : "checkno").".gif'></td>";
		}
	}
	echo "</tr>\n";
}

public function show_access_table_user (   $user_name,  $o_rights, $t_rights ) {
  // RETURN: flag. if a RED-icon occured: 0|1
  
  $memCheckRed=0; // #EEEEFF
  echo "<tr bgcolor=#FFFFA0><td>$user_name</td>";
  foreach($o_rights as $right => $value) {
	$sum_rig = 0;
	$sum_rig = 2*$value;
	switch ($right) {
		case "insert": if ($sum_rig && !$t_rights["write"]) $sum_rig = 1;
			break;
		case "entail": // no action
			break;
		default: if ($sum_rig && !$t_rights[$right]) $sum_rig  = 1;
			break;
	}
	
	if (!$sum_rig)   $img = "checkno";
	if ($sum_rig==1) $img = "checkred";
	if ($sum_rig==2) $img = "checked";
	echo "<td align='center'><img src='images/but.".$img.".gif'></td>";
	if ($sum_rig==1) $memCheckRed=1;
  }
  echo "</tr>\n";
  return ($memCheckRed);
}

/**
 * manage the row: NEW GROUP
 * @param  $rights
 * @param  $setRights 'no', 'yes', 'entail'
 */
public function write_access_table_row_new_group ( &$sqlo,  $rights,       $setRights) {

	if ($setRights == "no") return;
	
	$jsFormLib = new gJS_edit();
	$fopt = array( 'noshDel'=>1, 'selUseTxt'=>'[Add group]', 'emptyOnNoVal'=>1);
	
	// new_groupid
	$ngroup = $jsFormLib->getAll('USER_GROUP', 'new_groupid', 'not_set', NULL,  0, $fopt);

	$this->write_access_table_row ($sqlo, array("read"=>0, "write"=>0, "delete"=>0, "insert"=>0, "entail"=>0),
							$setRights, $ngroup, "new_group", 0, 1);
	
}

public function convRights_form2array (   
	$group_id, 
  	$user_rights,
  	&$grouparr // array(read_grp, write_grp, delete_grp, insert_grp, entail_grp )
	) {
# descr: converts the rights the user has set in the formular to an array for access_write
# input: group_id ... user_group_id whose values are to be prepared
#        user_rights ... rights which the user has (return array of function 'access_check')
#        the values the user set are taken from global variables
# return: array usable with function 'access_write'

  $rights_in_form = array();
  
  foreach($user_rights as $right_name => $dummy) {
	$rights_in_form[$right_name] = in_array($group_id, $grouparr[$right_name], 1) ? 1 : 0;
  }
  return $rights_in_form;
}




}