<?php
function obj_menu( &$menu, $id ) {
	$menu["func"][] = new MenuItem( "Compare MAC protocols", "p.php?mod=DEF/o.SOCKET.mr_prot_P&id=".$id, 0);
}
