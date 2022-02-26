<?php
function obj_selection() {

    $argu=array();
    $argu[] = array("url"=>"obj.jour_entry.list.php", "txt"=> "show as report");
    
    $menu = array('func'=>$argu );
    
	return ($menu);
}
