<?
// @SWREQ: 0001157: o.EXP > show thumbnails: erweitern um die Anzeige der timemedia diagramme
function obj_selection() {

    $argu=array();	
	//$argu[] = array("url"=>"obj.proj.images_show.php", "txt"=>"Show image thumbnails");
	//$argu[] = array("url"=>"obj.proj.images_show_series.php", "txt"=>"Show timedia thumbnails");
	//$argu[] = array("url"=>"objtools/EXP.filter_eisen.php", "txt"=>"Export results (CSV)");
	$argu[] = array("url"=>"obj.concrete_proto.m_comgui.php", "txt"=>"Protocols compare");
    //$argu[] = array("url"=>"obj.exp.list_iconorun.php", "txt"=>"Iconoclust");
	$argu[] = array("url"=>"obj.exp.res_visu_arr.php","txt"=>"MixedVisu");
	//$argu[] = array("url"=>"obj.img.spotpos_gui.php", "txt"=>"RefSpotPieker");
	//$argu[] = array("url"=>"obj.exp.virtarrx_gui.php", "txt"=>"ExpSpotPieker");
	$argu[] = array("url"=>"obj.exp.prot_comp2.php", "txt"=>"ProtoQuant");
    $argu[] = array("url"=>"obj.exp.list_funcs.php", "txt"=>"more ...");
    
    $menu = array('func'=>$argu );
    
	return ($menu);
}
