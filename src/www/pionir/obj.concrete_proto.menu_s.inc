<?	
function obj_menu( &$menu, $id ) {
	
	$menu["edit"][] = new MenuItem( "paste steps", "obj.concrete_proto.cp.php?id=".$id, 0);
	$menu["view"][] = new MenuItem( "print version", "obj.concrete_proto.print.php?id=".$id, 0);
	$menu["view"][] = new MenuItem( "QC form", "obj.concrete_subst.viqc.php?t=CONCRETE_PROTO&id=".$id, 0);
	$menu["func"][] = new MenuItem( "ProtoImporter", "obj.exp.imp_sample.php?&tablename=CONCRETE_PROTO", 0);
}
