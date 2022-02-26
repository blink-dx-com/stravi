<?php
function obj_menu( &$menu, $id, &$o_rights ) {
	$menu["func"][]  = new MenuItem( "export Excel-MTP-Protocol Template"	, "p.php?mod=LAB/o.EXP.mr_prot_exp&t=EXP_TMPL&id=".$id, 0);

}
