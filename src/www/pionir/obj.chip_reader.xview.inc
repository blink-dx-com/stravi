<?php

function obj_selection() {

    $argu=array();	
    $argu[] = array("url"=>"p.php?mod=DEF/o.CHIP_READER.logSearch&action=select", "txt"=>"Search in service log");
	$argu[] = array("url"=>"obj.concrete_proto.m_comgui.php", "txt"=>"compare protocols");
	$argu[] = array("url"=>"p.php?mod=DEF/o.CHIP_READER.proto_exp", "txt"=> "export protocols");
	$argu[] = array("url"=>"obj.exp.imp_sample.php", "txt"=> "import protocol data");
	$argu[] = array("url"=>"obj.chip_reader.lognew_li.php", "txt"=>"add service log");
	$argu[] = array("url"=>"p.php?mod=DEF/o.CHIP_READER.li_serviceHome", "txt"=>"service calendar");
	if (glob_isAdmin()) $argu[] = array("url"=>"p.php?mod=DEF/o.CHIP_READER.all_STATUS_MX", "txt"=>"update STATUS_MX");
	
    $menu = array('func'=>$argu );
    
	return ($menu);
}



