<?php
function obj_menu( &$menu, $id ) {
	$menu["func"][] = new MenuItem( "Add role to users", "obj.user_roles.create_r4u.php?role_id=".$id, 0);
}
