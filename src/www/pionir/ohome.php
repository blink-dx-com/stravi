<?php
/**
 * home page for an object type
 * optional: include additional module from :
	- ../[LAB]/obj.TABLE._home.inc : class oTABLE_home_LAB
	- ./obj.TABLE._home.inc : class oTABLE_home
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param string $t tablename 
 */
session_start(); 

require_once ('reqnormal.inc');
require_once ("f.help.inc");
require_once("subs/toolbar.inc");

class gObjTypeHome {
	

function __construct($tablename) {
	$this->tablename=$tablename;
	$this->pkNum = countPrimaryKeys($tablename);
}

function _subHeader( $text, $notes=NULL ) {
	// echo "<tr bgcolor=#EFEFEF><td colspan=2>&nbsp;&nbsp;&nbsp;<font color=gray><b>".$text.":</b></font>";
	echo "<tr><td colspan=2>&nbsp;&nbsp;&nbsp;<font style='color:#336699; font-size:1.2em;'>".$text."</font>";
	echo "&nbsp;&nbsp;<font color=gray>".$notes."</font>";
	echo "<hr size=1 noshade color=\"#6699DD\">";
	echo "</td></tr>\n";
}
function _subHeaderClose() {
	echo "<tr><td colspan=2>&nbsp;</td></tr>\n";
}

function _showTab( &$xmodes ) {
	$out =  '<table cellpadding=0 cellspacing=0 border=0>'."\n";
	foreach( $xmodes as $key=>$valarr) {
		$text1   = $valarr[2]=="" ? $valarr[1] : "<a href=\"".$valarr[2]."\">".$valarr[1]."</a>";
		$outhead = $valarr[0];
		if ($outhead!="")  $outhead .= ":&nbsp;";
		$out .= "<tr><td valign=top>&nbsp; <font color=#607080>".$outhead."</font>".
			"</td><td>".$text1."</td></tr>\n";
	}
	reset ($xmodes);
	$out .=  '</table>'."\n"; 
	return $out;	
}

function _getTable0() {
	$tablename   = $this->tablename;
	$tablename_l = strtolower($this->tablename);
  	$bgimg		 = "images/obj.".$tablename_l.".jpg";
    echo '<table cellpadding=0 cellspacing=0 border=0><tr>';
	if ( file_exists($bgimg) ) {
		
	echo "<td width=75 background=\"".$bgimg.
		"\"><img src=\"0.gif\" height=75 width=75></td>";
	}
	
	$objcolor = globTabMetaByKey( $tablename, 'COLOR');
    if ( $objcolor=="" ) $objcolor="#E0E0E0";
    
    echo '<td width=7 bgcolor='.$objcolor.'><img src="0.gif" width=7 height=1></td>'."\n";
	
}

function _helpAnalyse() {
	$helpLib = new fHelpC();
	$tablename = $this->tablename;
    
	$baseflag  = 'lab';
	$helpparam = "o.".$tablename.".html";
	$urlpath   = $helpLib->htmlCheckHelp( $helpparam, $baseflag );
	$helpInfo  = NULL;
	
	if ($urlpath=="") { 
	 //
	} else {
		$helpInfo .= ' &nbsp;&nbsp; <a href="f.help.php?f='.$helpparam.'&base='.$baseflag.'" target=_help>'.
			'<img src="images/help.but.gif" border=0> Lab specific DocBook</a> ';
	}
	return $helpInfo;
}


/**
 * create the menu
 */
function _menu() {
	/*
	
	$miHorizontal
	$miVertical
	$miRuler
	$miActive
	
	*/
	require_once("menuBuilder2.inc");
	
	$tablename = $this->tablename;
	$object_is_bo  = 0;
	$has_single_pk = 0;
	if ($this->pkNum==1) $has_single_pk = 1;
	
	$popdown = NULL; //"&nbsp;<img src=menu.popdown.gif border=0> &nbsp;";
	$popup   = NULL; //"<img src=menu.popup.gif border=0>";
	$miActive   = 0;
	$miInactive = 1;
	$miRuler    = 2;
	
	$menu=NULL;
	$menu[0][0] = new Menu($miHorizontal, '', 0, 0, 20, 90, 'itemBorder', 'itemText');
	
	$menu[0][] = new MenuItem($popdown . 'object ', '',  "obj", $miActive , 70);
	$menu[0][] = new MenuItem($popdown . 'help ', '', "help");

	$menu['obj'][0] = new Menu($miVertical, $popup, 0, 21, 120, 20, 'itemBorder', 'itemText');
	if ( $has_single_pk) {
	  $menu['obj'][] = new MenuItem( 'new ', 'glob.obj.crea_wiz.php?tablename='.$tablename, 0);
	} else { 
	  $menu['obj'][] = new MenuItem( 'new', '', 0, $miInactive,  0);
	}
	$menu["obj"][] = new MenuItem( "import","glob.objtab.import.php?tablename=".$tablename, 0);
	
	$menu["help"][] = new Menu($miVertical, $popup, 0, 21, 100, 20, 'itemBorder', 'itemText');
	$menu["help"][] = new MenuItem('object (docbook)', "javascript:open_info('f.help.php?f=o.".$tablename.".html')", 0);
	$menu["help"][] = new MenuItem('object (interactive)', "glob.objtab.info.php?tablename=".$tablename, 0);
	
	return ($menu);
}

function head(&$sqlo) {
	$tablename = $this->tablename;
	
	
	
	echo '<body color=#000000 bgcolor=#ffffff style="margin: 0;" >'."\n";
	$menu = $this->_menu();
	$mb = new MenuBuilder();
	$mb->createMenu($menu);
	$menuopt=array("menushow"=> 1);
	$objHeadLib = new fObjToolbar($tablename, 0, "home");
	$menuRight = $objHeadLib->getMenuRight($menuopt);
	$mb->menuRight($menuRight);
	
	echo "\n";
	
	// $this->table_cct_table_inf( $sql );
	$icon       = htmlObjIcon($tablename, 1);
	$tableShort = globTabMetaByKey( $tablename, 'SHORT' );
	
	
	$labHelp = $this->_helpAnalyse();

	// color:white
	$textHead  = '<img src="0.gif" width=1 height=4><br /><img src="'.$icon.'" height=20> <span style="font-size:1.4em;">'.
		''.tablename_nice2($tablename).' ';
	if ($tableShort!=NULL) $textHead .= '&nbsp;('.$tableShort.')';
	$textHead .= '</span><br />';
	
	/*
	$xmodes = NULL;
	// $xmodes[] = array( "Object-Type",  $text1 );
	$xmodes[] =	array( "Description",table_remark2($tablename) );
	$xmodes[] =	array( "Main tools",'<a href="view.tmpl.php?t='.$tablename.'">'.
		'<img src="images/but.list2.gif" border=0> list view</a>&nbsp;&nbsp;&nbsp;');
	*/	
	// color:white
	$moreOut = '<span style="">Description: '. table_remark2($tablename).'</span><br />'."\n";
	// color:white
	$moreOut .= '<a href="view.tmpl.php?t='.$tablename.'" style=""><img src="images/but.list2.gif" border=0> list view</a>';
	
	$searchForm = '&nbsp;&nbsp;<form method="post" style="display:inline;" name="editform"  action="glob.obj.qsearch.php">'.
		'<input type=hidden name="tablename" value="'.$tablename.'">'.
		'<input type=hidden name="go" value="2">'.
		'<input type=text name=idx value="" size=20> <input type=submit value="Search">';
	$moreOut .= $searchForm;
	
	$outhtml = $textHead . $moreOut; // $this->_showTab($xmodes);
	
	$toolOpt=NULL;
	$objHeadLib = new fObjToolbar($tablename, 0, "home", $toolOpt);
	$tolopt=array("menushow"=>1);
	$objHeadLib->toolbar_show($sqlo, $outhtml, $tolopt );
    $objHeadLib->toolbar_end();
   
    /*
	$this->_getTable0();
	echo '<td valign=top>'."\n";
	echo '<img src="0.gif" height=5 width=1><br />'."\n";
	*/
	
	
	// echo "</td></tr></table>\n";
}

/**
 * include a page-class, if exist
 * @param $sqlo
 */
function includePage(&$sqlo) {
	$tablow = strtolower($this->tablename);
	$modname='obj.'.$tablow.'._home.inc';
	
	$labFileExists=0;
	$class_base='o'.$this->tablename.'_home';
	$addLib = NULL;
	
	$scripLabPath = $_SESSION['globals']['lab_path'];
	if ($scripLabPath!=NULL) {
		$mod_file = '../'.$scripLabPath.'/'.$modname;
		if ( file_exists($mod_file) ) {
			$labFileExists=1;
			require_once($mod_file);
			$classname = $class_base.'_LAB';
			$addLib    = new $classname();
			
		} 
	}
	
	if (!$labFileExists) {
		$mod_file = './'.$modname;
		if ( file_exists($mod_file) ) {
			require_once($mod_file);
			$addLib = new $class_base();
		} 
		
	}
	
	if ( is_object($addLib) ) {
		$addLib->start($sqlo);
	} else {
		echo '<ul>';
		echo '<div style="color:gray;">Info: This page can contain object-type dependent content, designed by an application developper.</div>';
		echo '</ul>';
	}
}

}

// --------------------------------------------------- 
global $error, $varcol;

$error = & ErrorHandler::get();
$sqlo  = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();

$tablename 	= $_REQUEST['t'];
$title		= 'object type HOME';


$infoarr			 = NULL;
$infoarr['scriptID'] = '';
$infoarr['title_sh']    = $title;
$infoarr['title'] = tablename_nice2($tablename).' - HOME';
$infoarr['form_type']= 'tool';
$infoarr['help_url'] = 'o.'.$tablename.'.html';
$infoarr['locrow']   = array( array('home.php', 'home') );

$pagelib = new gHtmlHead();

if ($tablename==NULL) {
	$pagelib->startPage($sqlo, $infoarr);
	$pagelib->htmlFoot('ERROR', 'No tablename given');
}

$headOpt = array("noBody"=>1, "headIn"=> '<link rel="stylesheet" type="text/css" href="res/css/glob.menu2.css?d=1" />'."\n" );
$pagelib->_PageHead ( $infoarr["title"],  $headOpt );

$mainLib = new gObjTypeHome( $tablename );

$mainLib->head($sqlo);
$mainLib->includePage($sqlo);

$pagelib->htmlFoot();
