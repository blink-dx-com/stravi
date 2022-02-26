<?php
/*MODULE: glob.objtab.access.helper.inc
  DESCR:  helper functions to change access for a list of selected objects
  AUTHOR: mac
  USAGE: $this->AccHelperLib=new fAccessHelper();
  DB_MODIFIED: none
  VERSION: 0.1 - 20020909 - initial version
*/

class fAccessHelper {

function __construct() {

	$this->right_name2column_name = array(
		"read"   => "SELECT_RIGHT",
		"write"  => "UPDATE_RIGHT",
		"delete" => "DELETE_RIGHT",
		"insert" => "INSERT_RIGHT",
		"entail" => "ENTAIL_RIGHT");
	
	$this->right_names = array(
		"read"   => 0,
		"write"  => 0,
		"delete" => 0,
		"insert" => 0,
		"entail" => 0);
	
	$this->right_names_minus_one = array(
		"read"   => -1,
		"write"  => -1,
		"delete" => -1,
		"insert" => -1,
		"entail" => -1);
}

// tmp_access_show
function access_show ($o_rights, $manipulate, $chkbox_name="") {
# descr: write one column of the rigths table
# input: o_rights    ... array of the rights
#        manipulate  ... manipultion allowed?
#        chkbox_name ... name of the checkboxes (optional if $manipulate == 0)

  reset($o_rights);
  foreach( $o_rights as $right_name=>$right) {
	if ( $manipulate ) {
	  $checked = $right ? " checked" : "";
	  echo "<th><input type='checkbox' name='${chkbox_name}[$right_name]' value='on'$checked></th>";
	} else {
	  switch ($right) {
	  case  1: $tmp_img = "images/but.checked.gif";   break;
	  case -1: $tmp_img = "images/but.checkgray.gif"; break;
	  default: $tmp_img = "images/but.checkno.gif";
	  }
	  echo '<th><img src="'.$tmp_img.'"></th>';
	}
  }
}


function tabshow2 ($info_tab2) {
# show array in table
  reset($info_tab2);
  echo "<table cellspacing='2' cellpadding='0'>\n";
  foreach( $info_tab2 as $dummy=>$valarr) {
    $key = $valarr[0];
	$val = $valarr[1];
	if (!strcmp($key, "headline241177")) // strcmp returns 0 on equal!
	  echo "<tr><td colspan='3'><br>$val</td></tr>\n";
	else
	  echo "<tr valign='top'><td align='right' nowrap><font color='#999999'>$key:</font>&nbsp;</td><td width='95%'>&nbsp;$val</td></tr>\n";
  }
  echo "</table>\n";
}

}
