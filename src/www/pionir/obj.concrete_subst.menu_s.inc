<?
function obj_menu( &$menu, $id ) {
	$miRuler    = 2;
	// $menu["view"][] = new MenuItem( "QC form","obj.concrete_subst.viqc.php?id=".$id."&t=CONCRETE_SUBST", 0);

	// $menu["func"][] = new MenuItem( "sample tracking",	"obj.concrete_subst.tracking.php?tablename=CONCRETE_SUBST&id=".$id, 0);
	// $menu["func"][] = new MenuItem( "[ProtoQuant] - protocol tracking","obj.concrete_subst.track2.php?id=".$id, 0);
	$menu["func"][] = new MenuItem( "Usage &gt; forward explosion","p.php?mod=DEF/o.CONCRETE_SUBST.prot_follow&id=".$id, 0);
	// $menu["func"][] = new MenuItem( "Consumption",       "obj.concrete_subst.conOne.php?id=".$id, 0);
	// $menu["func"][] = new MenuItem( "DataFile analysis", "p.php?mod=DEF/g.QC_datafile_analysis&t=CONCRETE_SUBST&id=".$id, 0);
	//$menu["func"][] = new MenuItem( "Aliquot anlegen",   "p.php?mod=LAB/o.CONCRETE_SUBST.alinew2&t=CONCRETE_SUBST&id=".$id, 0);
	// $menu["func"][] = new MenuItem( "produced check",    "p.php?mod=DEF/o.CONCRETE_SUBST.prodcheck&id=".$id, 0);
	$menu["func"][] = new MenuItem( "Deactivate protocol"  ,  "p.php?mod=DEF/o.SUC.protDeakt&id=".$id, 0);
	$menu["func"][] = new MenuItem( "Tschimtarga - Checker",  "p.php?mod=DEF/o.CONCRETE_SUBST.TTC_g&id=".$id, 0);
	$menu["func"][] = new MenuItem( '&nbsp;', '#',0, $miRuler, 3);
	
}
