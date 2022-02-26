<?php
/**
 * build menu for edit.tmpl.php
 * $Header: trunk/src/www/pionir/edit.menu.inc 59 2018-11-21 09:04:09Z $
 * @package edit.tmpl.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param $object_is_bo
 * @param $has_single_pk
 * @param $tablename
 * @param $primasid
 * @param $id
 * @param $o_rights
 * @param $tablename_l
 * @param $menuRight
 */
require_once('menuBuilder2.inc');
require_once('glob.obj.conabs.inc');

function edit_menu_lab_make( &$menu, $id, $o_rights, $menu_file_lab) {

    $edit_menu=array();
    include($menu_file_lab);   		// object dependend menu in lab
    
    if (empty($edit_menu)) {
        return;
    }
    
    if (!empty($edit_menu['func'])) {
        foreach($edit_menu['func'] as $row) {
            $menu['func'][] = new MenuItem( $row['title'], $row['url'], 0, $row['active']);
        }
    }
}

// build menu for edit.tmpl.php
function edit_menu( $object_is_bo
    ,$has_single_pk
    ,$tablename
    ,$primasid
    ,$id
    ,$o_rights
    ,$tablename_l
    ,$menuRight ) {
    

$miActive   = 0;
$miInactive = 1;
$miRuler    = 2;
$table_nice = tablename_nice2($tablename);

$icon_spacer='<img src="0.gif" width=20 height=13>';
$popdown = NULL; //"&nbsp;<img src='menu.popdown.gif' border='0'> ";
$popup   = NULL; //"<img src='menu.popup.gif' border='0'>";
 
$menu=array();
$miHorizontal=NULL; //NEW:
$miVertical=NULL;

$menu[0][] = new Menu($miHorizontal, '', 0, 0, 20, 60, 'itemBorder', 'itemText');
//$menu[0][1] = new MenuItem("<img src='images/but.singobj.gif' title='single object' hspace=2 vspace=2> ", "javascript:open_info('help/robo/single_object_sheet.html')", '', $miActive, 45);
$menu[0][] = new MenuItem($popdown . 'object ', '',  "obj", $miActive , 70);
$menu[0][] = new MenuItem($popdown . 'edit', '',   "edit");
$menu[0][] = new MenuItem($popdown . 'view', '',   "view");
$menu[0][] = new MenuItem($popdown . 'functions ', '', "func", $miActive , 90);
$menu[0][] = new MenuItem($popdown . 'help ', '', "help");

$objLinkLib = new fObjViewC();
$icon_file =$objLinkLib->_getIcon($tablename);
$icon_html='<img src="'.$icon_file.'" hspace=2> ';

$menu['obj'][0] = new Menu($miVertical, $popup, 0, 21, 160, 20, 'itemBorder', 'itemText');
if ( $object_is_bo || $has_single_pk) {
    $menu['obj'][] = new MenuItem( $icon_html.'new '.$table_nice, 'glob.obj.crea_wiz.php?tablename='.$tablename.
		  		'&obj_blueprint_id='.$id.'&selecter=obj', 0);
} else { 
    $menu['obj'][] = new MenuItem( $icon_html.'new '.$table_nice, 'add.php?tablename='.$tablename, 0);
}

$absConcLib = new gConcAbsC();
$tabcrea    = $absConcLib->abs2concrete($tablename);
if ($tabcrea!=NULL) {
	// new menu-point to create a CONCRETE_OBJECT from a TEMPLATE
    $tabcreaNice = tablename_nice2($tabcrea);  
    $icon_c_file =$objLinkLib->_getIcon($tabcrea);
    $icon_c_html='<img src="'.$icon_c_file.'" hspace=2> ';
    $menu['obj'][] = new MenuItem( $icon_c_html.'new '.$tabcreaNice.' (from this)', 'glob.obj.crea_wiz.php?tablename='.$tabcrea.'&obj_tmpl_id='.$id.'&selecter=obj_tmpl', 0);
} 
			
$menu['obj'][] = new MenuItem('&nbsp;', '#',  0, $miRuler, 3);

if (isset($primasid[1])) {
	$assocObjUrlAdd = '&id[1]='.$primasid[1].
		'&id[2]='.(isset($primasid[2]) ? $primasid[2] : '');
} else $assocObjUrlAdd = NULL;

$menu['obj'][] = new MenuItem( $icon_spacer.'access ',	
    'glob.obj.access.php?t='.$tablename.'&id='.$id.$assocObjUrlAdd, 0 );
$menu['obj'][] = new MenuItem( $icon_spacer.'meta features', "glob.obj.feat.php?&t=".$tablename."&id=".$id, 0); 
$menu['obj'][] = new MenuItem( $icon_spacer.'object tracking ', 'glob.obj.search_usage.php?tablename='.$tablename.'&prim_id='.$id, 0);
$menu['obj'][] = new MenuItem( '<img src="res/img/settings.svg" hspace=5 height=13>'.'preferences ', 'glob.obj.pref.php?tablename='.$tablename.'&id='.$id, 0);
if ( $object_is_bo ) $menu['obj'][] = new MenuItem( $icon_spacer.'export (paxml)', 'impexp/partisanxml/export.php?cct_table='.$tablename.'&cct_id='.$id, 0);


$menu['edit'][0] = new Menu($miVertical, $popup, 0, 21, 100, 20, 'itemBorder', 'itemText'); 

$clip_obj_id_arr = clipboardC::obj_get ( $tablename );
$clip_obj_id     = $clip_obj_id_arr[0];
if ( $object_is_bo || $has_single_pk) {
    $menu['edit'][] = new MenuItem( 'copy',	'subs/clipboard.php?&action=copy&tablename='.$tablename.'&ida='.$id, 0);
    $menu['edit'][] = new MenuItem( 'copy plus',	'subs/clipboard.php?&action=copyplus&tablename='.$tablename.'&ida='.$id, 0);
}
if ($clip_obj_id) {
  $menu['edit'][] = new MenuItem( 'paste as new',	'glob.obj.crea_wiz.php?tablename='.$tablename.'&paste_new=1', 0);
} else {
  $menu['edit'][] = new MenuItem( 'paste as new', '', 0, $miInactive,  0);
}
if (!empty($_SESSION['s_clipboard'])) $menu['edit'][] = new MenuItem( 'paste advanced',	'glob.obj.assocpaste.php?tablename='.$tablename.'&id='.$id, 0);

$menu['edit'][] = new MenuItem( 'search',	'searchAdvance.php?tablename='.$tablename, 0);
$menu['edit'][] = new MenuItem( 'add to favorites',	'p.php?mod=DEF/g.obj.favact&t='.$tablename.'&id='.$id.'&act=add', 0);


$menu['view'][0] = new Menu($miVertical, $popup, 0, 21, 130, 20, 'itemBorder', 'itemText');
$menu['view'][]  = new MenuItem( "<img src='images/but.list2.gif' border='0'> list ", 'view.tmpl.php?t='.$tablename, 0);
$menu['view'][]  = new MenuItem( "object tree ", 'p.php?mod=DEF/g.obj.tree&t='.$tablename.'&id='.$id.'&show=all_auto', 0);
$menu['view'][]  = new MenuItem( "object home ", 'ohome.php?t='.$tablename, 0);
$menu['view'][]  = new MenuItem( 'show frame', "main.fr.php", 0);

$menu['func'][0] = new Menu($miVertical, $popup, 0, 21, 200, 20, 'itemBorder', 'itemText');


$menu["help"][] = new Menu($miVertical, $popup, 0, 21, 100, 20, 'itemBorder', 'itemText');
$menu["help"][] = new MenuItem($table_nice.' (docbook)', "javascript:open_info('f.help.php?f=o.".$tablename.".html')", 0);
$menu["help"][] = new MenuItem($table_nice.' (interactive)', "glob.objtab.info.php?tablename=".$tablename, 0);
$menu["help"][] = new MenuItem('single object form', "javascript:open_info('f.help.php?f=single_object_sheet.html')", 0);


$menu_file='obj.'.$tablename_l.'.menu_s.inc';
if ( file_exists($menu_file) ) {
	include($menu_file);   		/* object dependend menu */
	obj_menu($menu, $id, $o_rights);							
}
if ( $_SESSION['globals']["lab_path"]!="" ) {
	$menu_file_lab = "../".$_SESSION['globals']["lab_path"]. "/config_lab/o.".$tablename.".edit.inc";
	if ( file_exists($menu_file_lab) ) {
		require_once($menu_file_lab);   		// object dependend menu in lab
		edit_menu_lab_make($menu, $id, $o_rights, $menu_file_lab);
		// obj_menu_l($menu, $id, $o_rights);
	}
} 

$mb = new MenuBuilder();
$mb->createMenu($menu);
$mb->menuRight($menuRight);

echo "\n";

}