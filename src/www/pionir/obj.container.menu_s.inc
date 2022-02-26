<?

function obj_menu( &$menu, $id, &$o_rights ) {
    $menu["func"][] = new MenuItem( "stocktaking > export", "p.php?mod=DEF/o.CONTAINER.stocktake&action=export&id=".$id, 0);
    $menu["func"][] = new MenuItem( "stocktaking > import", "p.php?mod=DEF/o.CONTAINER.stocktake&action=import&id=".$id, 0);
    $menu["func"][] = new MenuItem( "show log", "p.php?mod=DEF/o.CONTAINER.entrylog&id=".$id, 0);
	$menu["func"][] = new MenuItem( "reserve entries", "obj.container.entryRes.php?id=".$id, 0);
	$menu["func"][] = new MenuItem( "import entries from file", "p.php?mod=LAB/o.CONTAINER.move_mat_f&id=".$id, 0);
	
}
