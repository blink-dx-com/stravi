<?php
/**
 * create GUI-menu for project-navigator
 * - only used by obj.proj.xsubst.inc
 * 
 * $Header: trunk/src/www/pionir/obj.proj.menu.inc 59 2018-11-21 09:04:09Z $
 * @package    obj.proj.navigator
 * @subpackage obj.proj.menu.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param string $projTables 
 */


require_once('globals.inc');
require_once('o.CCT_TABLE.info.inc');

class oPROJ_menu {
	
/**
 * - calculate automatic menu, 
 * - show objects, where user has table-right: insert
 * - reduce SQL-queries: cache the analysed tables in $_SESSION['s_formState']['obj.proj.menu']
 * @global $_SESSION['s_formState']['obj.proj.menu']
 */
function _getNewTables(&$sqlo) { 
    
    
	$useTables=NULL;
	
	if ( isset($_SESSION['s_formState']['obj.proj.menu']) ) {
		$useTables=$_SESSION['s_formState']['obj.proj.menu'];
		return ($useTables);
	}
	
    $TabLib = new oCCT_TABLE_info();
    $tab    = $TabLib->getTablesNiceByType($sqlo, 'BO');
	$seltab = NULL;
     
	// do NOT show them in menu > new ...
    $badTables = array(
    	'BCBATCH',  'ABS_CONTAINER',   'PROJ' );
    foreach( $tab as $table => $nice ) {
    	
    	if ( in_array($table,$badTables) ) continue;
    	
    	// user has "insert"-role-right ?
    	$rights = tableAccessCheck( $sqlo, $table );
    	if ($rights['insert']<=0) continue;
    	
		$useTables[]=$table;
       
	} 
	$_SESSION['s_formState']['obj.proj.menu'] = $useTables;
	return ($useTables);
    
}

function _projemu($icon, $text, $url, $sizex) {
	if ( $icon!="" ) {
		$iconx = "<img src='images/".$icon."' border=0 hspace=1> ";
	} else {
		$iconx = "<img src='0.gif' width=16 height=13 border=0> ";
	}
	$textx = $iconx . $text;
	$tmpelem = new MenuItem( $textx, $url, $sizex);
	
	return ($tmpelem);
}

/**
 * create the menu
 * @param array $projTables array of distinct tables in PROJ; array[table]=1
 */
function &getMenu(&$sql, $id, &$o_rights, &$projTables) {

	
	
	// TBD: get this from the menu-class !!!
	$popdown = NULL; // "&nbsp;<img src=menu.popdown.gif border=0 hspace=4>";
    $popup   = NULL; // "<img src=menu.popup.gif border=0>";
    $miActive   = 0;
	$miInactive = 1;
	$miRuler    = 2;
    $miVertical = true;
    $miHorizontal = false;
    
    $menu=NULL;
	$menu[0][] = new Menu($miHorizontal, '', 0, 0, 20, 60, 'itemBorder', 'itemText');
	// item text, no link, no frame, width if item, spacing (-1 means overlapping of one pix), submenus)
	$menu[0][] = new MenuItem($popdown . 'object ', '',  2, $miActive, 70);
	$menu[0][] = new MenuItem($popdown . 'edit'  , '',  'edit');
	$menu[0][] = new MenuItem($popdown.  'select all', '',  'sel', $miActive, 90);
	$menu[0][] = new MenuItem($popdown . 'view ', '', 4);
	$menu[0][] = new MenuItem($popdown . 'functions ', '', 'func', $miActive, 90);
	$menu[0][] = new MenuItem($popdown . 'help ', '', "help");
	
	/* $menu[1][0] = new Menu($miHorizontal, '', 30, 0, 20, 70, 'itemBorder', 'itemText'); */
	/* dummy menu, due to continous   index space*/
	
	$menu[2][] = new Menu($miVertical, $popup, 0, 21, 100, 20, 'itemBorder', 'itemText');
	$menu[2][] = new MenuItem( 'new &nbsp;&nbsp;<img src="images/menu.popup.gif" border="0">', '', 3);
	$menu[2][] = new MenuItem( 'access', "glob.obj.access.php?&t=PROJ&id=".$id, 0);
	$menu[2][] = new MenuItem( 'object tracking ', 'glob.obj.search_usage.php?tablename=PROJ&prim_id='.$id, 0);
	$menu[2][] = new MenuItem( 'meta features', "glob.obj.feat.php?&t=PROJ&id=".$id, 0); 
	$menu[2][] = new MenuItem( 'Import/Export', "p.php?mod=DEF/paxml/paxml_io&id=".$id, 0);  
	$menu[2][] = new MenuItem( 'preferences', "glob.obj.pref.php?tablename=PROJ&id=".$id, 0);
	
	// $menu[3][0]= new Menu($miVertical, '', 151, 0, 150, 20, 'itemBorder', 'itemText');
	    
	if ( $o_rights["insert"] ) {
		$menu[3][]= new Menu($miVertical, '', 100, 0, 150, 20, 'itemBorder', 'itemText');
		$menu[3][] = new MenuItem( "folder", "obj.proj.elementact.php?proj_id=".$id."&tablename=PROJ&actio_elem=new", 0);
		$menu[3][] = new MenuItem('&nbsp;', '#',0, $miRuler, 3);
		
		$menxarr = $this->_getNewTables($sql);
		if ( sizeof($menxarr) ) {
		    foreach( $menxarr as $tabxname ) {
				$menu[3][] = new MenuItem(tablename_nice2($tabxname), "javascript:createElem('".$tabxname."')", 0);
			}
		}
  
	} else {
		$menu[3][]= new Menu($miVertical, '', 100, 0, 150, 20, 'itemBorder', 'itemText');
		$menu[3][] = new MenuItem( "<font color=gray>no permission</font>", "", 0);
	}
	
	$menu["edit"][] = new Menu($miVertical, $popup, 0, 21, 110, 20, 'itemBorder', 'itemText');
	
	if ( $o_rights["insert"] ) {
	  $menu["edit"][] = new MenuItem( "cut", "javascript:document.objectsel.actio_elem.value='cut'; document.objectsel.submit();", 0);
	} else $menu['edit'][] = new MenuItem( "<font color=gray>cut</font>", "", 0);
	
	$menu["edit"][] = new MenuItem( 'copy', "javascript:document.objectsel.actio_elem.value='copy';document.objectsel.submit();", 0);
	
	if ( $o_rights["insert"] ) {
	  $menu["edit"][] = new MenuItem( 'paste', 'obj.proj.elementact.php?actio_elem=link_paste&proj_id='.$id, 0);
	  $menu["edit"][] = new MenuItem( 'advanced paste',  'obj.proj.elementact.php?actio_elem=paste_new&proj_id='.$id, 0);
	  $menu["edit"][] = new MenuItem( 'delete', "javascript:document.objectsel.actio_elem.value='del'; document.objectsel.submit();", 0);
	  
	} else {
	  $menu['edit'][] = new MenuItem( "<font color=gray>paste</font>", "", 0);
	  $menu['edit'][] = new MenuItem( "<font color=gray>delete</font>", "", 0);
	}
	
	if ($id!='NULL') $menu["edit"][] = new MenuItem( 'copy this folder', 'obj.proj.elementact.php?actio_elem=copy&proj_id='.$id.'&sel[PROJ_ORI]['.$id.']=1',  0);  
	$menu["edit"][] = new MenuItem( '&nbsp;', '#',  0, $miRuler, 3); 
	$menu["edit"][] = new MenuItem( 'check all', 'javascript:selall()', 0);
	$menu["edit"][] = new MenuItem( 'selected =&gt; list', "javascript:document.objectsel.actio_elem.value='select';document.objectsel.submit();", 0);
	
	$menu["edit"][] = new MenuItem( 'search', "javascript:open_lfr('obj.proj.search.php?id=".$id."&f=slim')", 0);
	
	$tmpurl = $_SESSION['globals']["htmlFrameTop.homeBut"];  
	
	$menu["sel"][0] = new Menu($miVertical, $popup, 0, 21, 170, 20, 'itemBorder', 'itemText');
	
	$tmpsearch = "=".$id;
	if ($id=="NULL") $tmpsearch = " is NULL";
	$sqls="select count(PROJ_ID) from PROJ where PRO_PROJ_ID".$tmpsearch;
	$sql->query($sqls);
	$sql->ReadRow();
	if ($sql->RowData[0]) { 
	    $menu["sel"][] = new MenuItem( "folders", "javascript:viewSubProjs()", 0);
	} 
	
	if (sizeof($projTables)) {
	    foreach( $projTables as $tmp_tablename=>$dummy ) {
	        $tmp_nicetabname = tablename_nice2($tmp_tablename);
	        if ($tmp_tablename=="PROJ") $tmp_nicetabname="linked folders";
	        $menu["sel"][] = new MenuItem( $tmp_nicetabname, "javascript:viewlist('".$tmp_tablename."')", 0);
	    }
	    
	}
	                                                                  
	
	$menu[4][] = new Menu($miVertical, $popup, 0, 21, 150, 20, 'itemBorder', 'itemText');
	//$menu[4][1] = new MenuItem( 'objects in list', '', 5);
	$menu[4][] = new MenuItem( 'view tree', 'javascript:viewTree()', 0);
	$menu[4][] = new MenuItem( 'home (app)', $tmpurl, 0);
	$menu[4][] = new MenuItem( 'show frameset', "main.fr.php", 0);
	//$menu[4][4] = new MenuItem( 'open in left frame', "javascript:open_lfr('frame.left.nav.php?id=".$id."&mode=proj')", 0);
	
	
	
	
	$menu["func"][0] = new Menu($miVertical, $popup, 0, 21, 170, 20, 'itemBorder', 'itemText');
	
	
	//if ( $o_rights["write"] ) {
	//  $menu["func"][] = $this->_projemu( "ic.object_new_set.gif", "new exp series", "obj.proj.series.php?proj_id=".$id, 0);
	//}
		
	if ( $projTables["EXP"] ) {
		//$menu["func"][] = $this->_projemu( "ic.EXCEL2.gif", "export exp results", "objtools/EXP.filter_eisen.php?proj_id=".$id, 0);
		//$menu["func"][] = $this->_projemu( "icon.IMG.gif", "show exp-images", "obj.proj.images_show.php?proj_id=".$id, 0);
		//$menu["func"][] = $this->_projemu( "but.iconoclust.gif", "Iconoclust", "obj.exp.list_iconorun.php?proj_id=".$id, 0);
		$menu["func"][] = $this->_projemu( "ic.mixedvisu.gif", "MixedVisu", "obj.exp.res_visu_arr.php?proj_id=".$id, 0);
		//$menu["func"][] = $this->_projemu( "ic.refspotpiek.gif", "RefSpotPieker", "javascript:open_win_new('obj.img.spotpos_gui.php?proj_id=".$id."')", 0);
		//$menu["func"][] = $this->_projemu( "ic.expspotpiek.gif", "ExpSpotPieker", "javascript:open_win_new('obj.exp.virtarrx_gui.php?proj_id=".$id."')", 0);
		$menu["func"][] = $this->_projemu( "icon.CONCRETE_PROTO.gif", "Protocols", "obj.concrete_proto.m_comgui.php?proj_id=".$id, 0);
	} else {
		$menu["func"][] = new MenuItem( "<font color=gray>no experiments</font>", "", 0, 1);
	}
	$menu["func"][] = new MenuItem('&nbsp;', '#',  0, $miRuler, 3);
	
	//if ( $o_rights["write"] ) {
	  // $menu["func"][] = new MenuItem( "substance import", "impexp/file.php?proj_id=".$id, 0);
	//} 
	$menu["func"][] = new MenuItem( "<b>...more</b>", "obj.proj.morefunc.php?id=" . $id, 0);
	
	if ( $_SESSION['globals']["lab_path"]!=NULL ) {
		$labFuncs = "../".$_SESSION['globals']["lab_path"]."/obj.proj.menu_s.inc";
		if ( file_exists($labFuncs) ) { 
			require_once($labFuncs);
			$menu["func"][] = new MenuItem('&nbsp;', '#',  0, $miRuler, 3);
		    this_projMenu( $menu, $id ); // add entries
		}
	}
	$menu["help"][] = new Menu($miVertical, $popup, 0, 21, 80, 20, 'itemBorder', 'itemText');
	$menu["help"][] = new MenuItem('docbook', "javascript:open_info('f.help.php?f=o.PROJ.html')", 0);
	
	return ($menu);
}

}	
