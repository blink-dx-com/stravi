<?
function obj_menu( &$menu, $id ) {
	


$menu["func"][] = new MenuItem( "Create entries", "obj.abs_container.creaEntries.php?id=".$id, 0);

}

