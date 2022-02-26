<?php
function obj_menu( &$menu, $id ) {
	// $menu["view"][] = new MenuItem( "QC form","obj.concrete_subst.viqc.php?id=".$id."&t=ABSTRACT_SUBST", 0);
    $menu["func"][] = new MenuItem( "Usage > forward explosion","p.php?mod=DEF/o.ABSTRACT_SUBST.prot_follow&id=".$id, 0);
    $menu["func"][] = new MenuItem( "MAA Spec Release"	, "p.php?mod=DEF/o.ABSTRACT_SUBST.spec&id=".$id, 0);
    $menu["func"][] = new MenuItem( "export Excel-MTP-Protocol Template"	, "p.php?mod=LAB/o.EXP.mr_prot_exp&t=ABSTRACT_SUBST&id=".$id, 0);
}
