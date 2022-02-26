<?php
function obj_menu( &$menu, $id ) {
    $menu["func"][] = new MenuItem( "Service Calendar","p.php?mod=DEF/o.CHIP_READER.serviceHome&id=".$id, 0);
    $menu["func"][] = new MenuItem( "Auto create Service events","p.php?mod=DEF/o.CHIP_READER.siccalc&id=".$id, 0);
}
