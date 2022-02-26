<?php
require_once('javascript.inc');
require_once 'f.modulLog.inc'; 


/**
 * - HTML page management
 * - can also be used, if session is not initialized
 * @package func_head2.inc
 * @swreq FS-INT-P provide a plugin mechanism
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @example <pre> 
 * $pagelib = new gHtmlHead();
 * $headarr = $pagelib->startPage($sqlo, $infoarr);
 * $pagelib->do_objAccChk($sqlo);
 * $pagelib->chkErrStop();
 *  ...
 * $pagelib->htmlFoot();
 * </pre>
 */
class gHtmlHead {

/**
 * infoarr_STRUCT $infoarr
 * config array with possible keys for array <pre> 
 * ! "title" : title
 *    "title_sh" (short title)			  
 *  ! "form_type" : type of page:
 *  			"obj" : single object: sets "back_url", "back_txt", "show_name" (if not set) 
 *  			"list": object list  : sets "back_url", "back_txt"
 *  			"tool": normal tool  : must set "back_url", "back_txt"
 *  "obj_name"  (tablename) 
 *  "obj_cnt"   ("list": if "obj_name" given, count and show selected elements)
 *  "obj_selinfo" ("list": show selection info)
 *  "obj_id"    ("obj" : PK of object)
 *  "obj_id2"   ("obj" : PK2 of object)
 *  "obj_id3"   ("obj" : PK3 of object)
 *  "help_url"  (help url) YOU MUST CALL htmlPageHead() before to have javascript function open_info()
 *  "help_base" (if "help_base"=="lab" help-document is in .../partisan/www/lab/help/robo)
 *  "row2_extTxt" : extra text: ROW2; before help text
 *  "locrow"   : array(array(url, text)))
 *  'locrow.strict' : [0],1
 *  "locrow2"  : second locrow
 *  "logModulExt" : extension string for logModule (to log a special call parameter)
 *  "icon"     :  image location
 *  "icon_y"   : int HEIGHT
 *  "obj_more" : more text in object-row
 *  "obj_row"  : [1] , -1 : do show OBJECT row ?
 *  'inforow'  : extra info text (notes) in sperate row
 *  'design'   : ['norm'] - normal start with "<ul>"
 *             : or 'slim'  
 *  'version'  : version string; e.g."2018-04-94" or "1.56 2018-04-94",
 *  'version.info'  : info string
 *  'ureq.info': Info of the USer-requirement: e.g. "REQ-0003 Design of production xyz"
 *  'active'   : module is active ? => page stops with INFO-message
 *       -1  : inactive
 *       [1] : active 
 *  'user.fav' : module is a favorite module?
 *  		: 0 : not allowed
 *          : 1 : can be flagged as fav
 *          : 2 : was flagged as fav
 *  'user.fav.conf' : config array for favorite configuration : see  g.modfavs.inc 
 *  'MXID'  : ID of Module-ID of this plugin
 *  'login.check' => FUTURE: still not used ...
 *     [1] default
 *     -1 : do NOT check a login
 *  'modLockChecked' : is the module already check for LOCKED-flag in table MODULE?
 *      [0] : no
 *       1  : yes
 *  
 *  for "form_type" == "obj"
 *  	"checkid"  : 0,1 : -- do a check, if object with "obj_id" exists, else error
 *  	"show_name": 0, [1]
 *  
 *  -- specials _PageHead --
 *   'noBody' = 0|1
 *   'headExtra'  = extra text after <body>
 *   'css'        = css-string : start local style-names with "xs" please
 *   'javascript' = javascript 
 *   'jsFile'     = include js-file, can also be an array of files
 *   'cssFile'    = include array of css-files
 *   'html.frworks' = array of frameworks keys; e.g. 'bootstrap', 'bootstrap-select'
 *   'headIn'	  = extra free text inside <head> section
 *  
 *  
 *  DEPRECATED keys:
 *     'notes     : notes text for module, will be shown belwo main head-table=> use inforow
 *     "sql_obj"   (sql object)
 *     "back_url"	is now in   "locrow"     
 *     "back_txt"  is now in   "locrow"
 *  </pre>
 * @var array $infoarr
 */
public $infoarr;

public $CLASSNAME='gHtmlHead';
public $loginUrl;  // e.g. '/pionir'
public $tablename;
public $locrow;    // array of text rows of page header
public $tableNice;

private $infoarr2=array(); // f.plugin.inc:infoarr2_STRUCT; set by class gPlugin
private $oMODULE_feats;

function __construct() {
	$this->loginUrl  = $_SESSION["s_sessVars"]["loginURL"];
	if ($this->loginUrl==NULL) {
		// fall back, if no valid session
		$this->loginUrl = '.';
	}
	$this->tablename = NULL;
}

public function set_infoarr2(&$infoarr2) {
    $this->infoarr2 = &$infoarr2;
}

// static 
static function getHtmlDocType() {
	// till 2020: need "loose.dtd"; otherwise IE9 has problems with CSS-Menu
	// return ('<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">'."\n");
    return ('<!DOCTYPE html>'."\n"); //NEW:2020-10-13
}

// set loginUrl, e.g. if user is not logged on
function set_loginUrl($loginUrl) {
	$this->loginUrl  = $loginUrl;
}

function _TabCnt( &$sql, $tablename ) {
    require_once ('sql_query_dyn.inc');
    global  $error;
    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
    
    $tmpcnt  = 0;         
    $error   = & ErrorHandler::get();
    $tmpinfo = $_SESSION['s_tabSearchCond'][$tablename]["info"];
    if ($tmpinfo!="") {
         $sqlafter = get_selection_as_sql($tablename);
         $sqls = "select count(*) from ".$sqlafter;
         $sql->query($sqls); 
         if ($error->Got(READONLY))  {
            $errLast   = $error->getLast();
            $error_txt = $errLast->text;  
            $error->set( $FUNCNAME, 1, "bad SQL query: ". $error_txt);
            echo '<span style="color:red; font-weight:bold;">Error:</span> during counting elements';
            return (0);
         }
         $sql->ReadRow();
         $tmpcnt = $sql->RowData[0];
         echo "<B>$tmpcnt</B>";
    }    
    else {
        echo '<span style="color:gray;">0</span>';
    }
    return ($tmpcnt);
}





/**
 * start HTMl page until </head> tag
 * this code must be in sync with html_tpl/macro/head_start.html
 * @param string $title
 * @param array $opt infoarr_STRUCT 
 *     "headIn"
 *     "logModulExt" : module log string extesnion; e.g. "t=EXP"
 */
function _PageHeadStart (  $title, $opt=array()  ) {
	
    echo $this->getHtmlDocType();
	echo "<html>\n";
	echo "<head>\n";
	echo '<meta http-equiv="expires" content="0">'."\n";
	echo '<meta http-equiv="pragma" content="no">'."\n";
	// for Internet Explorer: 7,8,9 : use strict HTML-rendering
	echo '<meta http-equiv="X-UA-Compatible" content="IE=edge">'."\n"; 
	if ($opt["headIn"]!="") {
		echo $opt["headIn"]."\n";
	}
	echo '  <title>'.$title.'</title>'."\n";
	echo '<script language="JavaScript" src="'.$this->loginUrl.'/res/js_goz/main.js?d=3"></script>'."\n";
	
	
	/**
	 * get standard include CSS-files
	 */
	$designid = $_SESSION['userGlob']["g.appDesign"];
	$pathWEB  = $this->loginUrl."/res/css/";
	
	// test if not logged in ...
	if ($this->loginUrl=="") {
	    $pathWEB  = "res/css/";
	}
	if ( $designid=="" ) $designid=1;
	$cssFile    = $pathWEB."glob".$designid.".css";
	$cssAllFile = $pathWEB."glob_all.css"; // independend from "g.appDesign"

	// URL-variable ?d=... is to force reloading a new *.css file
	echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"".$cssFile   ."?d=11\">\n";
	echo "<link rel=\"stylesheet\" type=\"text/css\" href=\"".$cssAllFile."?d=25\">\n"; 
	
	//
	// @swreq SREQ:0003368: g > Modul-Usage-Statistics
	// put this method at the very beginning of a script to cover all PHP-scripts
	$sqldummy = NULL;
	$modulLogLib = new fModulLogC();
	$modulLogLib->logModul($sqldummy, NULL, 0, $opt["logModulExt"]);
	
	
}

function _PageHeadEnd($opt=NULL) {
	echo "</head>\n";
	if ($opt["noBody"]!=1)     echo '<body text="#000000" bgcolor="#ffffff" style="margin: 0;">'."\n";
	if ($opt["headExtra"]!="") echo $opt["headExtra"];
}

/**
 * HTML page start till </head>, optional <body>
 * @param  $opt=NULL : <pre>
 *   
 *   'noBody' = 0|1
 *   'headExtra'  = extra text after <body>
 *   'css'        = css-string : start local style-names with "xs" please
 *   'javascript' = javascript 
 *   'jsFile'     = include js-file, can also be an array of files
 *   'cssFile'    = include array of css-files
 *   'html.frworks' = array of frameworks keys; e.g. 'bootstrap', 'bootstrap-select'
 *   'headIn'	  = extra free text inside <head> section
 */
function _PageHead ($title, $opt=array() ) {
		
  	$this->_PageHeadStart ($title, $opt);
  	
  	if ( is_array($opt['html.frworks']) ) {
  	    if ( in_array('bootstrap', $opt['html.frworks'])) {
      	    if (!is_array($opt['jsFile']))  $opt['jsFile']=array();
      	    if (!is_array($opt['cssFile'])) $opt['cssFile']=array();
      	    
      	    $opt['jsFile'][]='res/js/jquery-3.5.1.min.js';
      	    $opt['jsFile'][]='res/js/popper.min.js';
      	    $opt['jsFile'][]='res/js/bootstrap.min.js';
      	    $opt['cssFile'][]='res/css/bootstrap.min.css';
  	    }
  	    if ( in_array('bootstrap-select', $opt['html.frworks'])) {
  	        // insert after bootstrap !
  	        $opt['jsFile'][] ='res/js/bootstrap-select.min.js';
  	        $opt['cssFile'][]='res/css/bootstrap-select.min.css';
  	    }
  	}
	
	if ($opt["css"]!="") {
		echo "<style type=\"text/css\">\n";
		echo $opt["css"]; 
		echo "\n</style>\n";
	}
	if ($opt['javascript']!="") {
		echo '<script language="JavaScript">'."\n";
		echo '<!-- '."\n";
		echo $opt['javascript']."\n"; 
		echo '//-->'."\n";
		echo '</script>'."\n";
	}
	if ($opt['jsFile']!==NULL) {
	    if ( !is_array($opt['jsFile']) ) {
		  echo '<script language="JavaScript" src="'.$opt['jsFile'].'"></script>'."\n";
	    } else {
	        foreach($opt['jsFile'] as $jsfile)  {
	            echo '<script language="JavaScript" src="'.$jsfile.'"></script>'."\n";
	        }
	    }
	}
	if ($opt['cssFile']!="") {
        foreach($opt['cssFile'] as $cssfile)  {
            echo '<link href="'.$cssfile.'" rel="stylesheet">'."\n";
        }
	    
	}
	
	$this->_PageHeadEnd($opt);
}

/**
 * set internal locrow
 * @param array $infoarr
 *   'locrow.strict' :$this
 *      [0] : add user-locrow to default locrow
 *       1  : use only the user locrow
 */
function _setInfoRow(&$infoarr) {
    
	$this->locrow = array();
	$tablename = $infoarr["obj_name"];
	
	if ( !$infoarr['locrow.strict'] ) {
	
    	switch ( $infoarr["form_type"] ) {
    		case "obj":
    			$objurl = $this->loginUrl."/edit.tmpl.php?t=".$tablename."&id=".$infoarr["obj_id"];
    			if ( $infoarr["obj_id2"]!="" ) {
    				$backurltmp = "&primasid[1]=".$infoarr["obj_id2"];
    				if ($infoarr["obj_id3"]!="") $backurltmp .= "&primasid[2]=".$infoarr["obj_id3"];
    			} else $backurltmp = NULL;
    			
    			if ( !isset($infoarr["show_name"]) ) $infoarr["show_name"]= 1;
    			$this->locrow[] = array( $this->loginUrl."/home.php", "home" );
    			$this->locrow[] = array( $objurl.$backurltmp, $this->tableNice );
    			break;
    		case "list":
    			$objurl = $this->loginUrl."/view.tmpl.php?t=".$tablename;
    			$this->locrow[] = array($this->loginUrl."/home.php", "home" );
    			$this->locrow[] = array($objurl, $this->tableNice. " (list)");
    			break;
    		default:
    			//
    			break;
    	}
	}
	
	if ( isset($infoarr["locrow"]) )
    	if ( sizeof($infoarr["locrow"]) ) {
    		$this->locrow = array_merge($this->locrow, $infoarr["locrow"]);
    	}
	
	if ( isset($this->locrow) )
    	if ( sizeof($this->locrow) ) {
    		$lasttuple = end($this->locrow);
    		reset($this->locrow);
    		$infoarr["back_url"] = $lasttuple[0];
    	}
	$this->locrow[] = array(NULL,$infoarr["title_sh"]  );
}

function _firstCol() {
	echo "<td ".$this->col1Opt."><img src=\"". $this->loginUrl."/images/0.gif\" width=1 height=25></td>\n";
}

// show object
function _showObj(&$sql, &$infoarr, $bgcolor) {
	
	$nice_name = $this->tableNice;
	//$color1	   ="#808080";
	$colgray   = "#D0D0D0";
	$retarr    = array();
	
	$tablename = $infoarr["obj_name"];
	if (array_key_exists("obj_id", $infoarr) and $infoarr["show_name"]>0 ) {
		$t_obj_id = $infoarr["obj_id"];
	} else $t_obj_id = 0;
	

	echo "<tr class=yPlugHead>";
	
	echo "<td ".$this->col1Opt." ><img width=\"1\" height=\"21\">". 
		"</td>\n";
	echo "<td  nowrap colspan=\"".$this->colspan."\" >"; 
	
	echo '<span style="color:'.$colgray.';">';
	
	if ($tablename!="") { 
		$icon      = file_exists($_SESSION["s_sessVars"]["loginPATH"]."/images/icon.".$tablename.".gif") ? 
					$this->loginUrl."/images/icon.".$tablename.".gif":
					$this->loginUrl."/images/icon.UNKNOWN.gif";
		echo "<img src=\"$icon\"> $nice_name ";
	}
	
	if ($t_obj_id) {
		// can handle "PROJ" : id = "NULL" !
		$nicopt=array("noID"=>1);
		if ($tablename=="PROJ" and $t_obj_id=="NULL") $nice_get_tmp = 0; 
		else $nice_get_tmp = 1;
		if ($nice_get_tmp) $tmp_name = obj_nice_name($sql, $tablename, $t_obj_id, $nicopt);
		echo  "<B>$tmp_name</B> [ID:$t_obj_id]";
		if ( $infoarr["obj_id2"]!="" )  echo  "[".$infoarr["obj_id2"]."]";
		if ( $infoarr["obj_id3"]!="" )  echo  "[".$infoarr["obj_id3"]."]";
	}  
	if ($infoarr["obj_cnt"]) { 
		echo "&nbsp;&nbsp;&nbsp; No of selected elements: ";
		$retarr["obj_cnt"] = $this->_TabCnt($sql, $tablename);
	}
	if ($infoarr['obj_selinfo']>0 and $infoarr["form_type"]=='list') { 
		// show the selct info of a list view
		echo "&nbsp;&nbsp;&nbsp; <b>SelectInfo:</b> ";
		$tmpinfo = $_SESSION['s_tabSearchCond'][$tablename]["info"];
		if (strlen($tmpinfo)>60) $tmpinfo=substr($tmpinfo,0,60).' ...';
		$tmpinfo=htmlspecialchars($tmpinfo);
		echo $tmpinfo;
	} 
	if ($infoarr["obj_more"]) {
		echo "&nbsp;".$infoarr["obj_more"];
	}
	echo "</span>";
	
	echo "</td></tr>\n";
	
	return ($retarr);
}

function _shLocRow( &$locrow ) {
	
	// echo "<font class=yBrig>Loc:</font> ";
    echo '<span class=yBrig>';
	$tmpsep = "";
	foreach( $locrow as $valarr ) {
	
		echo $tmpsep;
		if ($valarr[0]!="") echo "<a href=\"".$valarr[0]."\" >".$valarr[1]."</a>"; //yBrig class=yBrig2
		else echo $valarr[1]; // "<span class=yBrig>".$valarr[1]."</span>";
		$tmpsep = " <img src=\"".$this->loginUrl."/images/ic.arrow.w.gif\"> ";
	}
	echo "</span>";
	
	
}

/**
 * calculate the online-help url
 * @param  $infoarr : "help_url", 'help_base'
 * @global $_SESSION['globals']['app.helpUrls']
 * @return $outhtml
 */
function _getHelpHtml(&$infoarr) {
	
	$help_base = $this->loginUrl.'/help/robo/';
	switch ($infoarr['help_base'] ) {
		case 'lab':
			$help_base = $this->loginUrl.'/../lab/help/robo/';
			break;
		case 'wiki':
			$help_base = $_SESSION['globals']['app.helpUrls']['wiki'].'/';
			break;
	}
	
	$outhtml  = "<a href=\"javascript:open_info('".$help_base.$infoarr["help_url"]."')\">";
	$outhtml .= '<img src="' .$this->loginUrl. '/res/img/help-circle.svg" style="filter:invert(100%)" TITLE="help"></a>'."\n";
	return $outhtml;
}

/**
 * some checks
 * @param object $sql
 * @param array $infoarr
 */
function _main_infoarr_checks($sql, &$infoarr) {
    if ( $infoarr["checkid"] ) {
        if ( $infoarr["obj_id"]==NULL or $infoarr["obj_name"]==NULL ) {
            echo "<br>";
            htmlFoot("Error", "Missing ID of object or table.");
        }
        if ( !gObject_exists ($sql, $infoarr["obj_name"], $infoarr["obj_id"]) ) {
            htmlFoot("Error", "Object does not exist.");
        }
    }
    if ($infoarr["obj_name"]=="" AND $infoarr["obj_cnt"]>0) {
        echo "<br>";
        htmlFoot("Error", "Missing input parameter: TABLENAME.");
    }
    
    
    if (!$infoarr['modLockChecked']) {
        $this->checkLocked($sql);
    }
}

 
/**
 * print Plugin info
 * * Plugin-ID
 * * Version
 * * role_right_need
 * * onlyAdmin
 * * ureq.info
 */
private function _modal_info_out() {
    
    $infoarr= &$this->infoarr;
   
    $outarr=array();
    // show module ID and Module version
    $mod_tmp='';
    $script_tmp = basename($_SERVER['SCRIPT_FILENAME'],'.php');
    if ($script_tmp=='p') {
        // analyse plugin name
        $mod_tmp = $_REQUEST['mod'];
    } else {
        $mod_tmp = $script_tmp;
        if (!isset($infoarr['version'])) {
            //$infoarr['version'] = self::get_version($_SERVER['SCRIPT_FILENAME']);
        }
    }
    
    if ($infoarr['user.fav']>0 and $infoarr['MXID'] ) {
        $outarr[]=array('Tool favorites', '<a href="p.php?mod=DEF/g.plugin.addfav&id='.$infoarr['MXID'].'&action=add">'.
            '<img src="images/ic.heart_b.png" title="add to favorites"></a> ');
    }
    
    $outarr[]=array('Plugin-ID',$mod_tmp);
    if (isset($infoarr['version'])) {
        $outarr[]=array('Version',$infoarr['version']);
    }
    if (isset($infoarr['version.info'])) {
        $outarr[]=array('Version info',$infoarr['version.info']);
    }
    if ( $this->infoarr2['role_right_need'] ) {
        $outarr[]=array('Required Role-right',$this->infoarr2['role_right_need'] ); 
    }
    if ($this->infoarr2['onlyAdmin']>0) {
        $outarr[]=array('Plugin-access','Only administrator' ); 
    }

    if ( $infoarr['ureq.info'] ) {
        $outarr[]=array('User-Requirement',htmlspecialchars($infoarr['ureq.info']));  
    }
    
   ?> 
    <div id="pluginInfoModal" class="y_modal">
      <!-- Modal content -->
      <div class="y_modal-content">
        <span class="close_x_button" onClick="modal = document.getElementById('pluginInfoModal'); modal.style.display='none';">&times;</span>
        <h2 style="display:inline;">Plugin info</h2>
        <p>
        <?php 
        foreach($outarr as $row) {
            echo '<b>'.$row[0].':</b> &nbsp;'.$row[1]."<br>";
        }
        ?> 
        </p>
      </div>
    </div> 
   <?php
}

/**
 * @deprecated
 * get version of a PLUGIN-module; result e.g. "1.15 2018/01/15"
 * 
 * OLD method; when CVS was used (untill 2018)
 * parse OCDE line:
 * {DOLLAR}Header: /group/it/cvs/partisan/www/pionir/p.php,v 1.15 2018/01/15 08:11:16
 *  0              1                                         2    3          4
 *  @param string $scriptfile
 *  @return string
 */
static function get_version($scriptfile) {
    return; // without result ...
	// if (!file_exists($scriptfile)) return;
}

/**
 * show HTML page body
 * @param object &$sql
 * @param infoarr_STRUCT &$infoarr   
 * @return array $retarr 
 * 		["obj_cnt"] number of objects ("form_type"=='list')
 */
function _startBody( &$sql, &$infoarr ) {
    
    $this->infoarr = $infoarr;
    
    $this->_modal_info_out();
    
	$transGif = 'src="'. $this->loginUrl.'/images/0.gif"';
	$title = $infoarr["title"];
	if ($infoarr["obj_name"]!="") { 
		$tablename=$infoarr["obj_name"];
		$this->tableNice = tablename_nice2($tablename);
		$this->tablename = $tablename;
	}
	if ( $infoarr["title_sh"]=="" ) $infoarr["title_sh"] = $title;
	
	$this->_setInfoRow($infoarr);
	$this->col1Opt="nowrap width=\"34\" ";

	$color_main = '#447799';
	
	//$color_bg1 = "#6699FF";
	//$color_bg2 = $color_main; // "#447799";
	$color_bg3 = $color_main; // "#225588";
	$color_bgInf = "#E0E0F0";
	
	
	$colspan    = 3;
	if ($infoarr["icon"] !="") {
		$colspan = $colspan + 2;
	}
	$this->colspan = $colspan;
	
	echo "<table border=\"0\" cellpadding=\"0\" cellspacing=\"0\" width=\"100%\" class=yPlugHead >\n";
	
	// *** ROW 1
	echo "<tr class=yPlugHead >\n";
	echo "<td ".$this->col1Opt." >\n"; 
	
	if (array_key_exists("back_url", $infoarr)) {
	   $tooltip = array_key_exists("back_txt", $infoarr) ? "Back to ".$infoarr["back_txt"] : "Back";
	   echo '<a href="'.$infoarr["back_url"].'">';
	   echo '<img src="'.$this->loginUrl.'/images/but.back.white.png" height=20 width=20 vspace=6 hspace=8 border="0" TITLE="'.$tooltip.'" ></a>';
     } else {
	   echo '<img width="34" height="24" '.$transGif.'>';
     }
	echo "</td>";
	echo "<td nowrap width=\"99%\">\n";
	$this->_shLocRow($this->locrow);
	if ( isset( $infoarr["locrow2"] ) ) {
		echo "<br>";
		$this->_shLocRow($infoarr["locrow2"]);
	}
	echo "</td>";
	echo "</tr>\n";
	
	// *** ROW 2
	$gbImgTmp = ""; 
	echo "<tr class=yPlugHead >";
	echo "<td ".$this->col1Opt." ><img ".$transGif." width=\"1\" height=\"25\"></td>"; 

	echo '<td width="100%" valign=middle style="padding-bottom: 10px;">'; // ".$gbImgTmp." colspan=1
	
	if ($infoarr["icon"] !="") {
	    $opt_height='';
	    if ($infoarr["icon_y"]) $opt_height=' height='.$infoarr["icon_y"];
	    echo '<img src="'.$infoarr["icon"].'" style="vertical-align:bottom;" '.$opt_height.'>&nbsp;';
	} 
	 
	echo '<span style="font-size:1.4em; font-weight:bold;" >'.$title."</span></td>";
    echo "<td nowrap ".$gbImgTmp.">";

	
	
	if ($infoarr["row2_extTxt"] != "") {
		echo '<span class=yBrig>'.$infoarr["row2_extTxt"].'</span>';
	}
	
	echo '<img src="'.$this->loginUrl.'/res/img/info.svg" style="filter:invert(100%)" '.
	   'title="Show plugin info" onClick="modal=document.getElementById(\'pluginInfoModal\'); modal.style.display=\'block\';" > ';
	if ($infoarr["help_url"] != "") {
          $helpHtml = $this->_getHelpHtml($infoarr);
          echo $helpHtml;
	}
    echo "&nbsp;</td>\n";
	echo "</tr>\n";
	
	$do_show_obj_row=0;
	if ( $infoarr["obj_name"] ) $do_show_obj_row=1;
	if ( $infoarr["obj_more"]!="") $do_show_obj_row=1;
	if ( $infoarr["obj_row"]<0) $do_show_obj_row=0;
	 
	if ( $do_show_obj_row ) {
		$retarr = $this->_showObj($sql, $infoarr, $color_bg3);
	}
	
	if ( array_key_exists("inforow", $infoarr) ) {       
	    echo "<tr>\n"; // bgcolor=\"".$color_bgInf."\" 
		$this->_firstCol();
		echo '  <td nowrap colspan='. ($colspan+1) .'>';
		echo $infoarr["inforow"];
		echo "  </td>";
		echo "</tr>\n";
    }
    
	echo "</table>\n";

	
	// [norm] , slim
	if ($infoarr['design']=='slim') {
        // no UL
	} else {
	    echo "<ul>\n";
	}
	
	// main error checks
	$this->_main_infoarr_checks($sql, $infoarr);

	return ($retarr);
}

/**
 * light body
 */
function _startBodyLight( &$sqlo, &$infoarr ) {
	
	$color_bg1 = "#99CCFF";
	echo "<table border=\"0\" cellpadding=\"0\" cellspacing=\"0\" width=\"100%\" bgcolor=\"".$color_bg1."\" >\n";

	echo "<tr bgcolor=\"".$color_bg1."\" height=30>\n";
	echo "<td ".$this->col1Opt." >\n";
	echo '&nbsp;';
	echo "</td>";
	echo "<td nowrap width=\"99%\">&nbsp;";
	echo $infoarr['title'];
	echo "</td>";
	echo "</tr>\n";
	echo "</table>\n";
}

/**
 * do access check: standard: write-test
 * @param array $act 'tab' => array(rights)
 *					 'obj' => array(rights)
 				leave 'tab' or 'obj' as empty array, if you do not want to check			  
 */
function do_objAccChk(&$sqlo, $act=NULL ) {
	global $error;
	$FUNCNAME = "do_objAccChk";

	$tablename = $this->infoarr['obj_name'];
	$id = $this->infoarr['obj_id'];

	// table rights
	$t_rights = tableAccessCheck( $sqlo, $tablename );
	if ( isset($act['tab']) ) $tRightArr = $act['tab'];
	else $tRightArr = array('write');

	foreach( $tRightArr as $rightx ) {
	
		if ( $t_rights[$rightx] != 1 ) {
			$answer = getTableAccessMsg( $tablename, $rightx );
			$error->set( $FUNCNAME, 1, $answer );
			return;
		}
	}
	

	// object rights
	if (isset($act['obj'])) $oRightArr = $act['obj'];
	else $oRightArr = array('read');
	
	if ( sizeof($oRightArr) ) {  // only when keys are available ...
		$o_rights = access_check($sqlo, $tablename, $id);
		$tabnice  = tablename_nice2($tablename);
		foreach( $oRightArr as $righto ) {
		
			if ( !$o_rights[$righto] ) {
				$error->set( $FUNCNAME, 2, 'no "'.$righto.'" permission on this '.$tabnice.' object!');
				return;
			}
		}
	}
	
	
}

// check if error, if error: show and stop
function chkErrStop() {
	global $error;
	if ( $error->got(READONLY) ) {
		if ($this->tablename!=NULL) {
			$tmptext = 'table:'.$this->tablename;
			if ($this->infoarr["obj_id"] != NULL ) {
				$tmptext .= ' objid:'.$this->infoarr["obj_id"];
			}
			$error->set($this->CLASSNAME, 1, $tmptext);
		}
		
		$err_stack = &$error->getAllNoReset();
		htmlErrorClassBox("Error", 'ERROR', $err_stack);
		$error->save_ErrLogAuto();
		$error->reset();
		htmlFoot('<hr>');
	}
}

/**
 * finish the page
 * @param string $key 
 *    ['NORM'], 
 * 		'WARN', 
 * 		'ERROR',
 * 		'USERERROR',  - user error
 *		'INTERR' - internal error
 *		'DEPRECATED' - user has called a DEPRECATED tool
 * @param string $text normal text
 * @param array $vopt
 *   'add_HR' [1] , -1
 */
function htmlFoot($key=NULL, $text=NULL, $vopt=array() ) {
	
    if ($vopt['add_HR']!=-1) $std='<hr>'."\n";
	$endtxt = $text . $std;
	if ($key==NULL) $key= 'NORM';
	
	if ($this->infoarr['design']!='slim') {
	    echo '</ul>'."\n";
	}
	
	if ($key=='NORM')  htmlFoot($endtxt);
	else {
		htmlFoot($key, $text, "", $std);
	}
}

/**
 * USE later, method tested, but not released
 * log module access of users
 * @swreq SREQ:0003368: g > Modul-Usage-Statistics
 * @param object $sqlo
 */
private function moduleLog(&$sqlo) {
		
	$modulLogLib = new fModulLogC();
	$modulLogLib->logModul($sqlo, NULL, NULL);
	// echo "DEBUG: moduleLog:answer: ".$answer." <br>";
	
}

/**
 * check, if registered in MODULE-table
 * - if LOCKED => stop script here (except user=root)
 * @swreq FS-INT-P02
 * @param object $sqlo
 */
private function checkLocked(&$sqlo) {
	
	if (!is_object($sqlo)) return; // not given ...

	$script_tmp  = $_SERVER['PHP_SELF'];
	$docroot     = $_SESSION['s_sessVars']['DocRootURL'];
	if ( $docroot=='/.' ) $docroot='/';
	$docroot_len = strlen($docroot); // no SLASH
	$script_base = substr($script_tmp, $docroot_len );       // no slash on START
	
	if ( substr($script_base, 0,1)=='/' ) $script_base = substr($script_base, 1); // remove slash
	
	//$base=NULL;
	$key='pionir/';
	if (substr($script_base,0,7)==$key) {
		$script_base = 'DEF' . substr($script_base,6);
	} else {
		$key='lab/';
		if (substr($script_base,0,4)==$key)  {
			$script_base = 'LAB' . substr($script_base,3);
		}
	}
	
	$this->oMODULE_feats = glob_elemDataGet3( $sqlo, 'MODULE', 
		array('LOCATION'=>'MOD:'.$script_base, 'TYPE'=>3), array('MXID', 'LOCKED') );
	
	// module found, check if LOCKED. only root can perform the action ...
	if ($this->oMODULE_feats['LOCKED']>0) {
		if ($_SESSION['sec']['appuser']!='root') {
			echo "<br>\n";
			$this->htmlFoot('ERROR', 'Module "'.$script_base.'" is LOCKED by system-configuration. Ask the admin to UNLOCK.' );
			return;
		} else {
			echo '<span style="color:red;">WARNING: This module is LOCKED by system config for normal users!</span>'."<br>\n";
		}
	}
	
}


/**
 * main page start API
 */
function startPage(&$sql, &$infoarr) {
    
    $title_html_page =  $infoarr["title"];
    if ($infoarr["obj_id"]) $title_html_page = '['.$infoarr["obj_id"].'] '.$title_html_page; // add object ID to page <head> title ...
    $this->_PageHead ( $title_html_page,  $infoarr );
	$retarr = $this->_startBody( $sql, $infoarr );
	
    return (  $retarr );
}

// no fancy CSS or JS includes ...
function PageHeadLight ($title) {
  echo $this->getHtmlDocType();
  ?>
  <html>
  <head>
    <title><?echo $title?></title>
    <meta http-equiv="expires" content="0">
  </head>
  <body text="#000000" bgcolor="#ffffff">
  <?
}     

/**
 * main page start API
 */
function startPageLight( &$sqlo, &$infoarr) {
	$this->PageHeadLight ( $infoarr["title"] );
	$retarr = $this->_startBodyLight( $sqlo, $infoarr );
    return (  $retarr );
}

/**
 * page head for Non-Html-pages
 * - use this instead of startPage()
 * - at least log the usage or other activities
 */
public function startPageNoHtml() {
	$sqlo_dummy = NULL;
	$this->moduleLog($sqlo_dummy);

}   

/**
 * adequate to _startBody, but no special checks
 * @param object $sql
 * @param array $infoarr
 */
public function startPageNoHtml2( &$sql, &$infoarr ) {
    $this->infoarr = $infoarr;
    $this->_main_infoarr_checks($sql, $infoarr);
}

}

class gHtmlMisc {
    
    /**
     * add a function to the FUNCTION history (func clipboard) and decide, if to show left frame
     * OLD-name: htmlFrFuncAdd
     * GLOBALS:
     *   $_SESSION['s_funcclip'] : array of function history
     *      array("k"=>$key, "t"=>$title, "u"=>$url, "x"=>$opt['xtra'] )
     *   $_SESSION['s_sessVars']["g.funcclip"] : current key of functionm
     * @param string $key "obj.TABLE.FUNC"
     * @param string $title "The nice tool"
     * @param string $url  "obj.TABLE.FUNC.php" path from .../pionir
     * @param string $opt
     *    "noShow" : 0,1
          "xtra"   : any extra info
     */
    static function func_hist(   $key, $title,   $url, $opt=array()   ) {

        
        $doRefresh = 1;
        $_SESSION['s_sessVars']["g.funcclip"] = $key;
        
        if ( !empty($_SESSION['s_funcclip']) ) {
            //reset($_SESSION['s_funcclip']);
            $funcPoi = &$_SESSION['s_funcclip'];
            
            foreach( $funcPoi as $pos=>$onefunc) {
                $tmp_key  = $onefunc["k"];
                if ($key==$tmp_key) {
                    if ($onefunc["u"]!=$url) {
                        $_SESSION['s_funcclip'][$pos]=array("k"=>$key, "t"=>$title, "u"=>$url, "x"=>$opt['xtra']);
                    }
                    $doRefresh = 0;
                    
                    if ( !$opt["noShow"] ) htmlShowHistory();
                    break;
                }
            }
            
        }
        
        if ( $doRefresh ) {
            
            $tmparr = array("k"=>$key, "t"=>$title, "u"=>$url, "x"=>$opt['xtra'] );
            if ($_SESSION['s_funcclip']!=NULL) array_unshift( $_SESSION['s_funcclip'], $tmparr);
            else $_SESSION['s_funcclip'][] = $tmparr;
            if (sizeof($_SESSION['s_funcclip'])>7) array_pop($_SESSION['s_funcclip']); // remove one function
            if ( !$opt["noShow"] ) htmlShowHistory();
        }
    }
}


/**
 * SINGLETON for HTML-templates
 * HTML-templates in html_tpl/main/
 * call:
 *  $html_obj = &gHtmlTmpl::getInstance();
 *  $content=array();
 *  $html_obj->out('one_page.html', $content);
 *
 */
class gHtmlTmpl {

    private $base_dir;
    private static $instance = null;
    
    // The constructor is private
    // to prevent initiation with outer code.
    private function __construct(){
        $this->base_dir = dirname(__FILE__).'/../html_tpl/main';
    }
    
    // The object is created from within the class itself
    // only if the class has no instance.
    public static function getInstance()
    {
        if (self::$instance == null)
        {
            self::$instance = new gHtmlTmpl();
        }
        return self::$instance;
    }
    
    /**
     * show error to user
     * @todo save  $internal_text in Log-file
     * @param string $user_text
     * @param string $internal_text
     */
    static function error(string $user_text, $internal_text=NULL) {
        echo 'INTERNAL-ERROR: '.$user_text.'<br>'."\n";
    }
    
    /**
     * output the HTML template
     * @param string $html_file
     * @param array $g -- the content array
     */
    function out(string $html_file, array $g) {
        $html_full = $this->base_dir.'/'.$html_file;
        if (!file_exists($html_full)) {
            self::error('Template-file not found.', 'File:'.$html_full);
            return;
        }
        include ($html_full);
    }
}

