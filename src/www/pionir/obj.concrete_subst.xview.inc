<?php

function obj_selection() {

    $argu=array();	
    $argu[] = array("url"=>"", "txt"=>"--- Protocol tools ---");
    $argu[] = array("url"=>"obj.concrete_proto.m_comgui.php",         "txt"=>"A) SimpleCompare protocols");
    $argu[] = array("url"=>"p.php?mod=DEF/o.CONCRETE_SUBST.li_comp2", "txt"=>"B) Show Step Differences (colors)");
    $argu[] = array("url"=>"obj.concrete_proto.m_comp2e.php?parx[action]=protocomp2&tablename=CONCRETE_SUBST",
        "txt"=>"C) Search Step Parameters");
    $argu[] = array("url"=>"p.php?mod=DEF/o.CONCRETE_SUBST.pq",       "txt"=>"D) SuperCompare protocols (ProtoQuant)");

	$argu[] = array("url"=>"obj.exp.imp_sample.php?tablename=CONCRETE_SUBST", "txt"=>"import protocols");
	
	$argu[] = array("url"=>"", "txt"=>"--- Other ---");
	
	$argu[] = array("url"=>"obj.container.add_li.php",          "txt"=>"add to container");
	$argu[] = array("url"=>"p.php?mod=DEF/o.CONCRETE_SUBST.ana_cont_li", "txt"=>"analyse in container"); // o.CONTAINER.anaSubst_li
	$argu[] = array("url"=>"obj.concrete_subst.label_li.php"  , "txt"=>"print labels");
	$argu[] = array("url"=>"p.php?mod=DEF/o.CONCRETE_SUBST.TTC_sea", "txt"=>"Tschimtarga > AUTODETECT");
	
	// $argu[] = array("url"=>"obj.concrete_subst.consum_li.php", "txt"=>"consumption calculation");
	// TBD: may be use later :$argu[] = array("url"=>"obj.concrete_subst.track2.php",     "txt"=>"[ProtoQuant-Track] - protocol tracking");
	// $argu[] = array("url"=>"obj.concrete_subst.qcsub_li.php", "txt"=>"[QC_checker]");
	
	$argu_v = array();
	$argu_v[] = array("url"=>"p.php?mod=DEF/o.SUC.datetree", "txt"=>"DATE-tree");
	
	$menu = array('func'=>$argu, 'view'=>$argu_v );
    
	return ($menu);
}
