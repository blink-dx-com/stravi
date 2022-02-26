<?php

function obj_selection() {

	$argu=array();
	$argu[] = array("url"=>"obj.proj.images_show.php", "txt"=>"show thumbnails");
	$argu[] = array("url"=>"obj.img.list_info.php", "txt"=>"image info");
	$argu[] = array("url"=>"obj.img.list_convert.php", "txt"=>"convert images");
	$argu[] = array("url"=>"obj.img.list_export.php", "txt"=>"export images");
	// $argu[] = array("url"=>"obj.img.list_cpfiles.php", "txt"=>"upload image files");

	$argu[] = array("url"=>"p.php?mod=DEF/o.IMG.fileDel_li", "txt"=>"delete image-files");
	$argu[] = array("url"=>"obj.img.cache_act.php", "txt"=>"clear thumbnail-cache");

    $menu = array('func'=>$argu );
    
	return ($menu);
}

?>

