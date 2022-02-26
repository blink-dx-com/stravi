<?php

function obj_selection() {

	$argu=array();

	$argu[] = array("url"=>"obj.link.searchDoc.php", "txt"=>"Search content");
	$argu[] = array("url"=>"obj.link.list_export.php", "txt"=>"Export documents");
	$argu[] = array("url"=>"obj.link.list_crea.php", "txt"=>"Import documents");
	$argu[] = array("url"=>"obj.link.list_vercr.php", "txt"=>"Activate v-control");
	$argu[] = array("url"=>"obj.link.list_dlnk.php", "txt"=>"Show download list");
	
    $menu = array('func'=>$argu );
    
	return ($menu);
}

?>

