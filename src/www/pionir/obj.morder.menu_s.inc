<?php

function obj_menu( &$menu, $id, &$o_rights ) {
    $menu["func"][] = new MenuItem( "Show calendar", 'p.php?mod=DEF/o.MORDER.li_calendar', 0);
	
	
}
