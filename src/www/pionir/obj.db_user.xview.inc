<?php

function obj_selection() {

  $argu=array();	
	$argu[] = array("url"=>"obj.user_roles.create_r4u.php", "txt"=>"Add role to users");
	$argu[] = array("url"=>"p.php?mod=DEF/o.DB_USER.addPref_li", "txt"=> "Set preferences to users");
	$argu[] = array("url"=>"glob.objtab.import3.php?parx[methodKey]=DB_USER", "txt"=> "Import users from CSV file");
	
    $menu = array('func'=>$argu );
    
	return ($menu);
}
