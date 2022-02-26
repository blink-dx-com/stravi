<?php
/**
 * [appHome] application home navigation page
 * - manage the theme-park
 *  $_SESSION['userGlob']["theme.[THEME]"]= serialize(
 *     'widget_f'=>array(key=>val)
 * );
 * 
 * $Header: trunk/src/www/pionir/home.php 59 2018-11-21 09:04:09Z $
 * @package home.php
 * @see     file://CCT_QM_doc/89_1002_SDS_code.pdf
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param layer = ""      use layer.home.inc
		          "array" use layer.array.inc
		          "wafer" use layer.wafer.inc
                  "subst" use layer.subst.inc
                  "lab"   use ../lab/layer.lab.inc
                  "home" -> show the theme park overview
 * @param array $widget_f widget_flag   - save in
 *   KEY => -1,1  : show, unshow
 *        
	GLOBAL:	 $_SESSION['s_sessVars']["g.surfMode"] 
		 
 * used funcs: layer_css()
 * @todo use a class for layer_css() and other !!!
 */
session_start(); 


require_once("db_access.inc");
require_once("globals.inc");
require_once("o.DB_USER.subs2.inc");
require_once("home.funcs.inc");
require_once("func_head.inc");
require_once("utilities.inc");
require_once("o.PROJ.subs.inc");
require_once("menuBuilder2.inc");
require_once 'o.USER_PREF.manage.inc';

if ( !is_partisan_open() ) {
	glob_relogin( "", "sessout" );
}


/**
 * abstract class for layers
 * changes:
 * - class drumherum bauen
 * - layer_show verliert erstes Object
 * @author steffen
 *
 */
class gThemeClsAbs {
	var $__homeFuncObj;
	var $_homeProjId; /* the home project
		[HOME]/.profile/queries  : for sql-queries
	*/
	protected $theme_vars=array();
	
	function __construct($layer) {
	    $this->__layer=$layer;
	    $sess_var_name='theme.'.$layer;
	    if(isset($_SESSION['userGlob'][$sess_var_name])) {
	        $this->theme_vars= unserialize($_SESSION['userGlob'][$sess_var_name]);
	    }
	    
	    // manage input
	    if (isset($_REQUEST['widget_f'])) {
	        foreach( $_REQUEST['widget_f'] as $key=>$val ) {
	            $this->theme_vars['widget_f'][$key]=$val;
	        }
	        $_SESSION['userGlob'][$sess_var_name]= serialize($this->theme_vars);
	    }
	   
	}
	
	// set home project
	function _setHomeProject(&$sqlo, $projid) {
		$this->_homeProjId=$projid;
	}
	
	/**
	 * - show all SQL-Query-documents as HTML-table from project: $this->_homeProjId / .profile / queries
	 * - need: $this->_homeProjId
	 * @param $sqlo
	 * @return -
	 */
	function _showQueries(&$sqlo) {
		require_once("gui/o.LINK.c_query_listGui.inc");
		
		global $error;
		$FUNCNAME= __CLASS__.':_showQueries';
		if (!$this->_homeProjId) {
			$error->set( $FUNCNAME, 1, 'need _homeProjId' );
			return;
		}
		$homeProjId = $this->_homeProjId;
		
		$projSubLib = new cProjSubs();
		$profileProj = $projSubLib->getProjByName($sqlo, $homeProjId, '.profile');
		if (!$profileProj) {
			$error->set( $FUNCNAME, 1, 'need ".profile" project in project '.$homeProjId.'.' );
			return;
		}
		$queriesProj = $projSubLib->getProjByName($sqlo, $profileProj, 'queries');
		if (!$queriesProj) {
			$error->set( $FUNCNAME, 2, 'need "queries" project in project '.$profileProj.'.' );
			return;
		}
		
		$queryShowLib = new oLINK_c_query_listGui($queriesProj);
		$htmlTmp = '<a href="edit.tmpl.php?t=PROJ&id='.$queriesProj.'">'.$queriesProj.'</a>';
		$queryShowLib->showTable($sqlo, 0, 'Queries', '(in project: '.$htmlTmp.')');
		
	}
	
	// widget is set active ?
	function _widget_is_active($key) {
	    if(!isset($this->theme_vars['widget_f'])) {
	        return 0;
	    }
	    $val = $this->theme_vars['widget_f'][$key];
	    return $val;
	}
	
	/**
	 * 
	 * @param string $key
	 * @param string $title
	 * @param int $show_flag
	 *    2 : shown forever, no inactive link
	 *    1 : is shown
	 *    -1 : is not shown
	 */
	function _widget_head($key, $title, $is_active ) {
	    echo '<div style="border: 1px solid #CCC; 
            padding: 15px; background-color:#F0F0F0; 
            margin-bottom:10px;
            color:#808080; ">';
	    
	    if ($is_active<2) {
    	    $url_base = $_SERVER['PHP_SELF'].'?widget_f['.$key.']=';
    	    $butimg='';
    	    if ( $is_active>0 ) {
    	        $butimg  = "res/img/chevron-down.svg";
    	        $url_base .= '-1';
    	    } else {
    	        $butimg   = "res/img/chevron-right.svg";
    	        $url_base .= '1';
    	    }
    	    echo '<a href="'.$url_base.'"><img src="'.$butimg.'"  height=20></a> ';
	    } else {
	        echo '<img src="res/img/chevron-down.svg"  height=20> ';
	    }
	    
	    echo $title;
	    echo '</div>';
	}
	
	function layer_params() {
		$lparams = array ( "bodycss" => "");
		return $lparams;
	}
	function layer_css() {}
	function layer_HtmlInit() {}
	function layer_show(&$sqlo, &$sqlo2) {}
	
	/**
	 * extend menu; EXAMPLE:
		 $menu["func"][]  = new MenuItem( 'Create array-batches', 'obj.w_wafer.add.php', 0);
	 * @param array $menu
	 */
	function layer_menu( &$menu ) {}

}


/**
 * manage home-layer
 * @author steffen
 *
 */
class fHomeC {
	
	var $__layer;

function __construct( ) {
	$this->__homeFuncObj = new fHomeFunc();
	
}

/**
 * check access to LAYER
 * @param object $sqlo
 * @param string $mod
 * @return int
 *    0 : deny
 *   >0: allow
 */
private function __checkPermission(&$sqlo, $layer) {

	$answer = 0;
	$rolename='LAYER:'.$layer;

	// check, if exists
	if (!role_plugin_right_exists($sqlo, $rolename)) return 1;

	// check, if user has the right
	$answer = role_check_p( $sqlo, $rolename);
	if ($answer=='execute') return 2;

	return 0;
}

/**
 * initialize LAYER
 * layer "home" must be defined by the admin !
 */
function __start(&$sqlo, $layer) {
    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	global  $error;
	
	//$xtra_file = "layer.home.inc";

	if ( isset($layer) ) {
	    $old_val = $_SESSION['userGlob']["g.homemode"];
		$_SESSION['userGlob']["g.homemode"]=$layer;
		if ($old_val!=$layer) {
		    // save new layer immediatly in user preferences ...
		    oUSER_PREF_manage::entry_update($sqlo, "g.homemode", $layer);
		}
	}
	$layer_now = empty($_SESSION['userGlob']["g.homemode"]) ? "" : $_SESSION['userGlob']["g.homemode"];
	if ( $layer_now==NULL ) {
	    $layer_now='home';
	}
	
	$layer_now_def = &$this->__homeFuncObj->nicearr[$layer_now];
	if (empty($layer_now_def)) {
	    $error->set( $FUNCNAME, 10, 'Theme-park "'.$layer_now.'" not defined.' );
	    return;
	}
	
	if (empty($layer_now_def['code_name'])) {
	    $layer_code=$layer_now;
	} else {
	    $layer_code=$layer_now_def['code_name'];
	}
	$xtra_file = "layer.".$layer_code.".inc";
  	if ($layer_now_def["dir"] == "lab") 
  	    $xtra_file = "../".$_SESSION['globals']['lab_path']."/".$xtra_file;
	
	
	$result = $this->__checkPermission($sqlo, $layer_now);
	if ($result<=0) {
		$error->set( $FUNCNAME, 20, 'You have no access permissions to see this Theme-park "'.$layer_now.'". Please ask the Admin.' );
		return;
	}
	
	if ( file_exists($xtra_file) ) {
		require_once($xtra_file);   // layer class
	} else {
	    $error->set( $FUNCNAME, 30, 'Theme-park "'.$layer_now.'" not defined. (Code:'.$xtra_file.')' );
		return;
	}
	
	$this->__layer = $layer_now;
	
	$this->themePage = new themeCls($layer_now);
	if (!is_object($this->themePage)) {
		$error->set( $FUNCNAME, 40, 'Theme-park-module contains not the class.' );
		return;
	}
	$this->themePage->__homeFuncObj = &$this->__homeFuncObj;

}


/**
 * start page
 */
function __pageStart(&$sqlo, &$sqlo2) {
	
	
	$layer_now = $this->__layer;
	
	$lparams = $this->themePage->layer_params();
	
	$bodyDesignParam = "";
	if ($lparams["bodycss"]!="")  $bodyDesignParam = " class=\"".$lparams["bodycss"]."\"";
	
	$title = $this->__homeFuncObj->nicearr[$layer_now]["nice"];
	
	$cparams = $this->themePage->layer_css();
	
	$fhopt   = array(
	    "css" => $cparams,  
		"headIn"=>menuBuilder::getCssInclude2(),
	    'jsFile'=>array('res/js/jquery-3.5.1.min.js')
	); 
	
	$pagelib = new gHtmlHead();
	$pagelib->_PageHeadStart ( "Home: ".$title, $fhopt); // _PageHead ( "Home: ".$title,  $fhopt );
	if ($fhopt["css"]!="") {
		echo "<style type=\"text/css\">\n";
		echo $fhopt["css"];
		echo "\n</style>\n";
	}
	if ( is_array($fhopt['jsFile']) ) {
	    foreach($fhopt['jsFile'] as $jsfile)  {
	        echo '<script language="JavaScript" src="'.$jsfile.'"></script>'."\n";
	    }
	    
	}
	   
	$menu = $this->menu();
	
	// TBD: !! eventuell GLOBAL VAR problem
	/**
	 *   @todo solve this !!!
	 */
	
	
	$this->themePage->layer_HtmlInit();
	$enopt = array("noBody" => 1);
	$pagelib->_PageHeadEnd($enopt);
	
	echo "<body ".$bodyDesignParam." >\n";
	?>
	<script>
	<!--
		
		
	   	function linkto( desttable ) {
	  		
	  		location.href= "view.tmpl.php?t=" + desttable;
			
	  	}
		function linktoElem( destinfo ) {
	  		
	  		location.href= "edit.tmpl.php?t=" + destinfo;
			
	  	}
	   
		function Go(x)
		{
		  if(x == "---")
		  {
	            document.forms[0].reset();
	            document.forms[0].elements[0].blur();
	            return;
		  }
		  else 
		  {
	            location.href = 'view.tmpl.php?t='+x;
	            document.forms[0].reset();
	            document.forms[0].elements[0].blur();
		  }
		}
	//-->
	</script>
	<?
	$mb = new MenuBuilder();
	if (  $_SESSION['s_sessVars']["g.surfMode"] != "text" ) {
		$mb->createMenu($menu);
		$mb->menuRight("");
	}
	echo "\n";
	
	if (  $_SESSION['s_sessVars']["g.surfMode"] == "text" ) {
		$mb->printAsText( $menu );
	}
	
	
	$this->themePage->layer_show($sqlo, $sqlo2);
	
}

function _svg_crea($input) {
    $svg = '
    <svg xmlns="http://www.w3.org/2000/svg" width="40" height="40" viewBox="0 0 40 40" >
    <circle cx="20" cy="20" r="20" style="fill:'.$input['c'].';opacity:0.9" />
    <text x="20" y="25" fill="white" text-anchor="middle" font-family="Arial, Helvetica, sans-serif">'.$input['t'].'</text>
    </svg>';
    return $svg;
}

/**
 * show Theme park menu
 * can be extended by $this->themePage->layer_menu()
 * @return MenuItem
 */
function menu() {

	$miActive   = 0;
	$miInactive = 1;
	$miRuler    = 2;
	$popup = NULL;
	$popdown ='';
	
	// direction of menu
	$miVertical = true;
	$miHorizontal = false;
	
	
	$menu=array();
	$menu[0][0] = new Menu($miHorizontal, '', 0, 0, 20, 90, 'itemBorder', 'itemText');
	
	$menu[0][1] = new MenuItem($popdown . 'system ', '',  1);
	$menu[0][2] = new MenuItem($popdown . 'theme park ', '',   "theme", $miActive, 110);
	$menu[0][3] = new MenuItem($popdown . 'functions ', '', "func");
	
	$menu[1][0] = new Menu($miVertical, $popup, 0, 21, 170, 20, 'itemBorder', 'itemText');
	$menu[1][] = new MenuItem( "<img src='images/icon.DB_USER.gif' border=0 hspace=3> myAccount", "obj.db_user.settings.php", 0);
	$menu[1][] = new MenuItem( "<img src='images/i13_info.gif' border=0 hspace=5> system info ", 'n.syslinks.php', 0);
	
	if ( $_SESSION['s_suflag'] == 1 OR $_SESSION['sec']['appuser'] == "root" )    
		$menu[1][] = new MenuItem( "<img src='images/i13.system3.png' border=0 hspace=3> administration", "rootsubs/rootFuncs.php", 0);
	
	$menu[1][] = new MenuItem('&nbsp;', '#',  0, $miRuler, 3);
	$menu[1][] = new MenuItem( "<img src='images/ic.logout.gif' border=0 hspace=5>logout ", 'logout.php', 0);
	
	$menu["theme"][0] = new Menu($miVertical, $popup, 0, 21, 130, 20, 'itemBorder', 'itemText');
	
	
	// shows only important entries ( nicearr[KEY]['menu']>0 )
	$niceThemes = $this->__homeFuncObj->nicearr;
	
	foreach( $niceThemes as $key => $infoarr ) {
	
		if ($infoarr['menu']<=0) continue; // show only important entries ...
		
		if ($infoarr['ty']=='hr') {
		    $menu["theme"][]  = new MenuItem('&nbsp;', '#',  0, $miRuler, 3);
		    continue;
		}
		if ($key=='home') $outtext ='<b>'.$infoarr["nice"].'</b>';
		else $outtext = $infoarr["nice"];
		
		
		if ($infoarr['icon']) { // ONLY icon, no text ...
		    if ($infoarr["dir"]=='lab') $lab_dir='../'.$_SESSION['globals']["lab_path"].'/';
		    else $lab_dir='';
		    $ic = '<img src="'.$lab_dir.'images/'.$infoarr['icon'].'?d=2"> ';
		    $outtext =  $ic . ' '. $outtext; 
		}
		if ($infoarr['shortsvg']) { 
		    
		    $svg = $this->_svg_crea($infoarr['shortsvg']);
		    $outtext =  $svg . ' '. $outtext;
		}
		
		
		$menu["theme"][]  = new MenuItem( $outtext, 'home.php?layer='.$key, 0);
		//if ($key=='home')  $menu["theme"][]  = new MenuItem('&nbsp;', '#',  0, $miRuler, 3);
	}
	
	$menu["func"][0] = new Menu($miVertical, $popup, 0, 21, 180, 20, 'itemBorder', 'itemText');
	$menu["func"][]  = new MenuItem( "<img src='images//ic.33.search.png' height=15> quick search ", 'glob.obj.qsearch.php', 0); 
	$menu["func"][]  = new MenuItem( "<img src='images/ic.myqueryLogo.40.png' height=15> My Search Center", 'obj.link.c_query_mylist.php', 0);
	$menu["func"][]  = new MenuItem( "<img src='images/b17.lfram.gif'> show frameset", "main.fr.php", 0);
	
	
	//$menu["func"][] = new MenuItem('&nbsp;', '#',  0, $miRuler, 3);
	$this->themePage->layer_menu($menu);
	
	return ($menu);
}

}

function this_ErrorPageHead(&$sqlo) {
	$infoarr=NULL;
	$infoarr			 = NULL;
	$infoarr['title']    = 'theme park';
	$infoarr['form_type']= 'tool';
	$infoarr['design']   = 'norm';

	$pagelib = new gHtmlHead();
	$pagelib->startPage($sqlo, $infoarr);
}

// --------------------


$error  = & ErrorHandler::get();
$varcol = & Varcols::get();
$sqlo   = logon2( $_SERVER['PHP_SELF'] );
$sqlo2  = logon2(  );

$layer = $_REQUEST['layer'];

$mainLib     = new fHomeC();

$mainLib->__start($sqlo, $layer);
if ($error->Got(READONLY))  {
	this_ErrorPageHead($sqlo);
	$error->printAllPrio (5);
	
	echo '<br>[<a href="'.$_SERVER['PHP_SELF'].'?layer=home">go to standard theme-park</a>]'."\n";
	return;
}

$mainLib->__pageStart($sqlo, $sqlo2);
if ($error->Got(READONLY))  {
	$error->printAllPrio (5);
	return;
}


if (!$_SESSION['s_sessVars']["start"]) {       // session has been started ?
	$opturl = "";
	$opturl .= "&startsessflag=1"; // show at the first time after login
	$_SESSION['s_sessVars']["start"]=1;

	?>
	<script>
	  	<!--
		if ( parent.left != null ) { // check if exists
			parent.left.location.href='frame.left.nav.php?<?php echo $opturl?>';
		}
		//-->
	</script>
	<?php
}

htmlFoot();
