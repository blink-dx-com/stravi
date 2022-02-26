<?php
function obj_menu( &$menu, $id ) {
$menu["view"][] = new MenuItem( "print version", "obj.abstract_proto.print.php?id=".$id, 0);
// $menu["view"][] = new MenuItem( "QC form", "obj.concrete_subst.viqc.php?t=ABSTRACT_PROTO&id=".$id, 0);


$menu["func"][] = new MenuItem( "import steps from Excel",   "p.php?mod=DEF/o.ABSTRACT_PROTO.import&id=".$id, 0);
$menu["func"][] = new MenuItem( "import layout", "p.php?mod=DEF/o.ABSTRACT_PROTO.lay_imp&id=".$id, 0);
$menu["func"][] = new MenuItem( "search similar protocols", "p.php?mod=DEF/o.ABSTRACT_PROTO.likely&id=".$id, 0);
$menu["func"][] = new MenuItem( "export steps to Excel", "obj.abstract_proto.print.php?id=".$id."&parx[raw]=1", 0);
$menu["func"][] = new MenuItem( "search protocol parameters", "p.php?mod=DEF/o.CONCRETE_PROTO.li_search&parx[pra_id]=".$id, 0);
//$menu["func"][] = new MenuItem( "[ProtoQuant] settings", "obj.protocol.quant.php?apid=".$id."&go=1", 0);
// $menu["func"][] = new MenuItem( "[SAP_BOM] update", "p.php?mod=LAB/098_limb/o.PRA.SAP_update&id=".$id, 0);

}
