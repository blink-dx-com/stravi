<?php
// @package obj.user_group.xview.inc
function obj_selection() {

	$argu=array();
	$argu[] = array("url"=>"p.php?mod=DEF/o.USER_GROUP.track_prefs", "txt"=>"Track in user preferences");
    $menu = array('func'=>$argu );
    
	return ($menu);
}

?>

