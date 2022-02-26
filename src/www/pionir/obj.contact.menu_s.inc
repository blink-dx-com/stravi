<? // @package obj.contact.menu_s.inc
function obj_menu( &$menu, $id ) {
	$menu["func"][] = new MenuItem( "[ProtoQuant] Samples > Protocols","obj.contact.track.php?id=".$id, 0);
	//$menu["func"][] = new MenuItem( "encrypt notes-field","obj.contact.crypt.php?id=".$id."&field=NOTES", 0);
	//$menu["func"][] = new MenuItem( "encrypt password-field","obj.contact.crypt.php?id=".$id."&field=password", 0);
	$menu["func"][] = new MenuItem( "show letter formatted","obj.contact.addressprint.php?id=".$id, 0);
	$menu["func"][] = new MenuItem( "show in Google Maps","p.php?mod=DEF/o.CONTACT.showinmap&id=".$id, 0);
}
