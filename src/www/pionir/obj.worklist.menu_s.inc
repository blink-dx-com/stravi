<?

function obj_menu( &$menu, $id, &$o_rights ) {
	
  $miRuler=2;
  $menu["edit"][]  = new MenuItem('&nbsp;', '#',  0, $miRuler, 3);
  $menu["edit"][]  = new MenuItem( "insert content"	, "p.php?mod=DEF/o.WORKLIST.pasteColDat&id=".$id, 0);
  $menu["func"][] = new MenuItem( "Import selected objects", "p.php?mod=DEF/o.WORKLIST.guimod3&id=".$id, 0);
  $menu["func"][] = new MenuItem( "Start data analysis", "p.php?mod=DEF/g.QC_analysis2&t=WORKLIST&id=".$id, 0);



}
