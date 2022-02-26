<?php

/**
 * called function lab_list_menu($menu) has full acces to the $menu structure
 * @package view.menu.inc
 * @param array $menu
 * @param string $tablename
 * @param array $t_rights
 * @param int $primasnum
 * @version $Header: trunk/src/www/pionir/view.menu.inc 59 2018-11-21 09:04:09Z $
 */
function fViewMenu( &$menu, $tablename, &$t_rights, $primasnum ) { 
	require_once("menuBuilder2.inc");
	
	$is_admin = glob_isAdmin();
	$popdown  = NULL; //"&nbsp;<img src=menu.popdown.gif border=0> ";
	$popup    = NULL; // "<img src=menu.popup.gif border=0>";
	
	$miActive   = 0;
	$miInactive = 1;
	$miRuler    = 2;
	 
	$menu[0][] = new Menu($miHorizontal, '', 0, 0, 20, 60, 'itemBorder', 'itemText');
	// $menu[0][1] = new MenuItem("<img src='images/but.list3.gif' title='object list view' hspace=2 vspace=2> ", "javascript:open_info('help/robo/pionir_list_view.html')", '', $miActive, 45);
	$menu[0][] = new MenuItem($popdown . 'object ', '',  "obj", $miActive, 70);
	$menu[0][] = new MenuItem($popdown . 'edit', '',   "edit");
	$menu[0][] = new MenuItem($popdown . 'view', '',   "view");
	$menu[0][] = new MenuItem($popdown . 'functions ', '', "func", $miActive , 90);
	$menu[0][] = new MenuItem($popdown . 'help ', '', "help");
	
	$menu["obj"][0] = new Menu($miVertical, $popup, 0, 21, 160, 20, 'itemBorder', 'itemText');
	
	$tmp=NULL;
	// do NOT allow "new" for ASSOC elements for normal users
	if ( $t_rights["insert"] )  {
	    if ( $primasnum==1 ) { // if is a single_PK object
	        $tmp = new MenuItem( "new", "glob.obj.crea_wiz.php?tablename=".$tablename, 0);	
	    } else {
	        if ($is_admin) $tmp = new MenuItem( "new", "add.php?tablename=".$tablename, 0);
	        else {
	            $tmp = new MenuItem( '<span style="color:gray">new</span>', "", 0);
	        }
	    }
	} else $tmp = new MenuItem( '<span style="color:gray">new</span>', "", 0);
	$menu["obj"][] = $tmp;
	
	$tmp = new MenuItem( '<span style="color:gray">delete</span>', "", 0);
	// do NOT allow "delete" for ASSOC elements for normal users
	if ( $t_rights["delete"] )  {
	    if ( $primasnum==1 ) { // if is a single_PK object
	        $tmp = new MenuItem( "delete", "javascript:formAction('glob.objtab.delete.php')", 0);
	    } else {
	        // onyl allow for admin
	        if ($is_admin) $tmp = new MenuItem( "delete", "javascript:formAction('glob.objtab.delete.php')", 0); 
	    }
	}
	$menu["obj"][] =  $tmp;
	
	$menu["obj"][] = new MenuItem('&nbsp;', '#',  0, $miRuler, 3);
	$menu["obj"][] = new MenuItem( "access", "glob.objtab.access.php?tablename=".$tablename, 0);
	$menu["obj"][] = new MenuItem( "object tracking", "glob.objtab.trackgui.php?tablename=".$tablename, 0);
	$menu["obj"][] = new MenuItem( "preferences", "glob.objtab.pref.php?tablename=".$tablename, 0);
	$menu["obj"][] = new MenuItem( "export / import","glob.objtab.export_list.php?tablename=".$tablename, 0);
	$menu["obj"][] = new MenuItem( "export as *.xlsx","glob.objtab.exp.php?t=".$tablename."&format=xlsx", 0);
	
	$menu["edit"][0] = new Menu($miVertical, $popup, 0, 21, 130, 20, 'itemBorder', 'itemText');
	$menu["edit"][] = new MenuItem( "select", "javascript:formAction('select')", 0);
	$menu["edit"][] = new MenuItem( "copy selected", "javascript:formAction('copysel.php')", 0);
	
	$clip_obj_id_arr = clipboardC::obj_get ( $tablename );
	$clip_obj_id=$clip_obj_id_arr[0];
			
	if ($clip_obj_id) { 
		// add.php?tablename=".$tablename."&blueprint_val=".$clip_obj_id
		$menu["edit"][] = new MenuItem( "paste as new", "glob.objtab.insert.php?tablename=".$tablename, 0); 
	} else {
		$menu["edit"][] = new MenuItem( "paste as new", "", 0, $miInactive,  0);
	}
	$menu["edit"][] = new MenuItem( '&nbsp;', '#',  0, $miRuler, 3);
	$menu["edit"][] = new MenuItem( "bulk update", "javascript:formAction('glob.objtab.edit.php')", 0);
	$menu["edit"][] = new MenuItem( "bulk edit",   "javascript:formAction('glob.objtab.editinter.php')", 0);
	$menu["edit"][] = new MenuItem( "bulk string replace",   "javascript:formAction('glob.objtab.change_field.php')", 0);
	// glob.objtab.edit.php?tablename=".$tablename
	
	
	$menu["view"][0] = new Menu($miVertical, $popup, 0, 21, 120, 20, 'itemBorder', 'itemText');
	$menu["view"][]  = new MenuItem( "search advanced", "searchAdvance.php?tablename=".$tablename, 0);
	$menu["view"][]  = new MenuItem( "Pro-Feature-Search", "glob.objtab.search_1.php?t=".$tablename, 0);
	$menu["view"][]  = new MenuItem( "as slim html", "view.tmpl.php?t=".$tablename."&viewmode=lean&view_opt[setsPerView]=all", 0);
	$menu["view"][]  = new MenuItem( "full details", "glob.objtab.shobj.php?t=".$tablename, 0);
	$menu["view"][] = new MenuItem( '&nbsp;', '#',  0, $miRuler, 3);
	if ($primasnum<2) {
		$menu["view"][]  = new MenuItem( "in left frame", "javascript:open_lfr('frame.left.nav.php?listtab=".$tablename."')", 0);
	}
	$menu["view"][]  = new MenuItem( "object home", "ohome.php?t=".$tablename, 0);
	$menu["view"][] = new MenuItem('theme park', $_SESSION['globals']["htmlFrameTop.homeBut"], 0);
	
	$menu["func"][] = new Menu($miVertical, $popup, 0, 21, 200, 20, 'itemBorder', 'itemText');
	
	$menu["help"][] = new Menu($miVertical, $popup, 0, 21, 100, 20, 'itemBorder', 'itemText');
	$menu["help"][] = new MenuItem('list view', "javascript:open_info('f.help.php?f=pionir_list_view.html')", 0);
	$menu["help"][] = new MenuItem('object (docbook)', "javascript:open_info('f.help.php?f=o.".$tablename.".html')", 0);
	$menu["help"][] = new MenuItem('object (interactive)', "glob.objtab.info.php?tablename=".$tablename, 0);
	
	
	// following methods are included by file "obj.TABLE.xview.inc" (TABLE: name of current table)

	// get object dependent actions 
	if ( function_exists('obj_selection') ) {
		$functab = obj_selection(); /* object dependent insert pionir-actions */
		if ( sizeof ($functab)) {
		    foreach( $functab as $key=>$th ) {
		        foreach( $th as $dummy=>$th2 ) {
					$menu[$key][] = new MenuItem( $th2["txt"], "javascript:formAction('".$th2["url"]."')", 0);
				}
			}
		}
	} 
    if ( function_exists('lab_list_menu')) {
        // object dependent LAB menu items, store method in file obj.{table}.xview.inc
    	
        $functab = lab_list_menu(); 
    	if ( sizeof ($functab)) {
			
    	    foreach( $functab as $key=>$th ) {
    			$menu[$key][] = new MenuItem('&nbsp;', '#',  0, $miRuler, 3);
    			
    			foreach( $th as $dummy=>$th2 ) {
					if ($key=='help') {
						// show url for help-pages in extra windows
						$menu[$key][] = new MenuItem( $th2["txt"], 
								"javascript:open_info('".$th2["url"]."')", 0); 
					} else {
						$menu[$key][] = new MenuItem( $th2["txt"], "javascript:formAction('".$th2["url"]."')", 0);
					}
				}
			}
		}
    	
    }
   
}
