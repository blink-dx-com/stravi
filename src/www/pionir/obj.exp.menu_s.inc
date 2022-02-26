<?php

function obj_menu( &$menu, $id, &$o_rights ) {
	$miRuler    = 2;
	$menu["obj"][] = new MenuItem('&nbsp;', '#',  0, $miRuler, 3);
	//if (!empty($o_rights["insert"])) { 
	//	$menu["obj"][] = new MenuItem( "import result set", "obj.exp.res_import.php?EXP_ID=".$id, 0);
	//}
	$menu["obj"][]  = new MenuItem( "export results"	, "obj.exp.res_export_gui.php?EXP_ID=".$id, 0);
	// $menu["func"][] = new MenuItem( "virtual hybridization", "objtools/EXP.virthyb.php?exp_id=".$id, 0);
	//$menu["func"][] = new MenuItem( "show time series"	, "obj.exp.res_visu_series.php?id=".$id,  0);
	//$menu["func"][] = new MenuItem( "[patternmatch]"	, "obj.exp.nearpatt.php?id=".$id,  0);
	// $menu["func"][] = new MenuItem( "[R Interface]"	, "objtools/EXP.result_trafo.php?id=".$id,  0);
	//$menu["func"][] = new MenuItem( "Show selected spots IMG"	, "obj.exp.resSpotImg.php?id=".$id."&parx[spotsel]=1",  0);
	$menu["func"][] = new MenuItem( "Compare Cartridge-MAC protocols", "p.php?mod=DEF/o.SOCKET.mr_prot_P&t=EXP&id=".$id, 0);
	
}
