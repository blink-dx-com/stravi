<?php
/**
 * base classes for the plugin manager page
 * HTML-methods: see func_head2.inc
 * @package f.plugin.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */

require_once ("visufuncs.inc");

// ---------------------------------------

/**
 * manage object-list options
 * @author steffen
 *
 */
class gPlugListC {
	var $CLASSNAME='gPlugListC';
	var $obj_cnt;
	
	function  __construct($tablename) {
		require_once("sql_query_dyn.inc");
		
		$this->tablename = $tablename;
		$sqlopt=array();
		$sqlopt["order"] = 1;
		$this->sqlAfter  = get_selection_as_sql( $tablename, $sqlopt);
		$this->sqlAfterNoOrder  = get_selection_as_sql( $tablename);
		$this->obj_cnt = 0;
	}
	
	// PUBLIC
	function getSqlAfter() {
		return ($this->sqlAfter);
	}
	// PUBLIC
	function getSqlAfterNoOrder() {
		return ($this->sqlAfterNoOrder);
	}
	// PUBLIC
	function getObjCnt() {
		return ($this->obj_cnt);
	}
	
	/**
	 * check, if a selection is active ...
	 * @param $sql
	 * @param $option : "checkNumMax"
	 */
	function initCheck( &$sqlo, $option=NULL ) {
		global $error;
		$FUNCNAME= $this->CLASSNAME.':initCheck';
		
		require_once ("f.visu_list.inc"); 

		$this->obj_cnt = 0;
		
		$tablename   = $this->tablename;
		$listVisuObj = new visu_listC();
		
		$copt=NULL;
		$copt["doCount"] = 1;
		if ($option["checkNumMax"]>0) $copt["checkNumMax"] = $option["checkNumMax"];
		
		$checkResult = $listVisuObj->checkSelection( $sqlo, $tablename, $copt );
		$stopFlag   = $checkResult[0];
		$stopReason = $checkResult[1];
		$this->obj_cnt = $checkResult[2];
		
		if ( $stopFlag<0 ) {
			$error->set( $FUNCNAME, 1, $stopReason.
				' Please select elements from table "'.tablename_nice2($tablename).'"' );
			return;
		}
	}
	
}

/**
 * manage HTML output
 * @author steffen
 *
 */
class gPlugHtmlLib {
	var $_flushLib;
	var $initArr;   // store sub-control initialization
	private $_infoarr; // copy from gPlugin
	
	function __construct($mod, $go) {
		$this->htmlTags = array(
			'BR'=>"<br />\n"
		);
		$this->_mod = $mod;
		$this->_go  = $go;
	}
	
	function set_infoarr($infoarr) {
		$this->_infoarr = &$infoarr;
	}
	
	function htmlKeyOut($key) {
		echo $this->htmlTags[$key];
	}
	function line_out($text) {
		echo $text. "<br />\n";
	}
	function rawHtml($text) {
		echo $text;
	}
	function rawHtmlCR($text) {
		echo $text."\n";
	}
	
	// open a message box
	function messageBox_open($title,$pretext,$key) {
		htmlInfoBox( $title, $pretext, "open", $key );
	}
	
	// close a message box
	function messageBox_close() {
		htmlInfoBox( '', '', "close" );
	}
	
	// easy message box
	function messageBox($title,$pretext,$key, $htmlText) {
		htmlInfoBox( $title, $pretext, "open", $key );
		echo $htmlText;
		htmlInfoBox( '', '', "close" );
	}
	
	// the better message box
	function messageBox2($key, $main_text) {
	    require_once ('f.msgboxes.inc');
	    cMsgbox::showBox($key, $main_text); 
	}
	
	// progress bar : alive method
	function progressAlive($cnt, $force=0) {
		if ( !$this->initArr['progress'] ) 
			sys_error_my( '_flushLib not initialized.' );
		$this->_flushLib->alivePoint($cnt,$force);
	}
	
	function progressAliveAuto($active, $inc_time) {
		if ( !$this->initArr['progress'] )
			sys_error_my( '_flushLib not initialized.' );
		$this->_flushLib->alivePointAuto($active, $inc_time);
	}
	
	// progress bar : init number of objects
	function progressInit($prgopt) {
		
		$this->_flushLib->shoPgroBar($prgopt);
		$this->initArr['progress']=1;
	}
	
	function &progress_getlib() {
	    return $this->_flushLib;
	}
	
	/**
	 * show form-progress
	 * @param $go
	 * @param $goArray
	 * @param $extraParams
	 * @param optional string $altTarget - alternative target link
	 * @param optional string $altText - alternative target text
	 * @return -
	 */
	function GoInfo($goArray, $extraParams=NULL, $altTarget=NULL, $altText=NULL) {
		require_once ("func_form.inc");
		
		$xxText = NULL;
		if ($extraParams!=NULL) $xxText = '&'.$extraParams;
		$extratext = '[<a href="'.$_SERVER['PHP_SELF'].'?mod='.$this->_mod.$xxText.'">Start again</a>]';
		if ($altTarget AND $altText) {
			if ($extraParams!=NULL) $extraParams = '?'.$extraParams;
			$extratext = '[<a href="'.$altTarget.$extraParams.'">'.$altText.'</a>]';
		}
		$formPageLib = new FormPageC();
		$formPageLib->init( $goArray, $extratext );
		$formPageLib->goInfo( $this->_go ); 
	
	}
	
	/**
	 * set GO-variable
	 * @param int $go
	 */
	function set_go($go) {
		$this->_go = $go;
	}
	
	/**
	 * init the form manager, derived from class formc
	 * @see formc
	 * @param $initarr
	 * @param $hiddenarr
	 * @param $go
	 * @return object formc (form-manager handle)
	 */
	function &formInit($initarr, $hiddenarr, $go=0) {
		require_once ("func_form.inc");
		$initarr["action"]      = $_SERVER['PHP_SELF'];
		if ($initarr["tabwidth"]==NULL) $initarr["tabwidth"] = "AUTO";
		
		$hiddenarr['mod'] = $this->_mod;
		$formobj = new formc($initarr, $hiddenarr, $go);
		return $formobj;
	}
	
	/**
	 * preformatted array output
	 * @param $arrvar
	 */
	function _outArray1(&$arrvar, $title) {
		
		if (!is_array($arrvar)) {
			echo 'no data ...';
			return;
		}
		$tabobj3 = new visufuncs();
		
		$headx=array('key','val');
		$tabopt=array('title'=>$title, 'headNoShow'=>1);
		$tabobj3->table_head($headx, $tabopt);
		
		
		foreach( $arrvar as $key=>$val) {
			$dataarr = array( htmlspecialchars($key), htmlspecialchars($val) );
			$tabobj3->table_row ($dataarr);
		}
		
		$tabobj3->table_close();
	}
	
	function _table_out2(&$headx, &$dataArr,  $headOpt=NULL) {
		$tabobj = new visufuncs();
		$tabobj->table_out2($headx, $dataArr,  $headOpt);
	}
	
	// advanced table
	function _table3_init($headx, $headOpt) {
		$this->tabobj3 = new visufuncs();
		$this->tabobj3->table_head($headx, $headOpt);
	}
	function _table3_row($dataArr,$opt=NULL) {	
		$this->tabobj3->table_row ($dataArr,$opt);
	}
	function _table3_close() {	
		$this->tabobj3->table_close();
	}
	
	// get URL of this plugin
	function _plug_get_URL() {
	    return 'p.php?mod='.$this->_mod;
	}

	/**
	 * forward to an other url
	 * @param $url
	 * @param $link_text
	 * @param int $delay in milliseconds
	 */
	function pageForward( $url, $link_text, $delay = NULL ) {
		require_once ( "javascript.inc" );
		$stop=NULL;
		js__location_replace($url, $link_text, $stop, $delay);
	}
	
	
	/**
	 * open url in new window
	 * @param $text
	 */
	function windowOpen( $url, $link_text) {
		
		
		echo 'Please klick, if window does not open: [<a href="'.$url.'" target=_new>'.$link_text.'</a>] <br>';
		echo '<script type="text/javascript"><!--',"\n";
		// ALL parameters MUST be set, otherwise gives javascript-problems ...
		
		echo 'InfoWin  = window.open( "'.$url.'");',"\n";
		echo 'InfoWin.focus();',"\n";

		echo '//--></script>',"\n";
		return;
	}

	
	/**
	 * add plugin to FUnction Clipboard (Left frame of application)
	 * @param string $title_new - alternative Plugin title
	 */
	function add_FuncClipboard($title_new=NULL) {
		
		$title = $this->_infoarr['title'];
		if ($title_new!=NULL) $title = $title_new;
		gHtmlMisc::func_hist( 'plugin.'.$this->_mod, $title, $this->_plug_get_URL() );
	}
}

/**
 * PLUGIN base class
 * @author steffen
 *
 */
class gPlugin {
	/**
	 * first-level plugin-description
	 * @var $infoarr
	 * @see gHtmlHead#$infoarr
	 */
	var $infoarr;
	
	/** 
	 * @var array $infoarr2 (infoarr2_STRUCT) second-level infoarr<pre>
	 * 	'accCheck'		: array() array of rights to check
	 *  'role_right_need' : check for a special role-right
	 *  'onlyAdmin'     : 0,1
	 *  'listChkSelect' : 0,1 : do a list select check
	 *  'listCheckNumMax': int : check for MAX allowed number
	 *  'progressBar'   : 0,1 : init a progress bar ?
	 *  'noHtmlHead'    : 0,1 : do NOT show initial HTML-elements
	 *  'noHtmlBodyTable'  : 0,1 : only show HTML-Head, not the plugin-Header-Table
	 * </pre>
	 */
	var $infoarr2; // array of settings for initial checks
	var $listLib;  // list-function library, set if $infoarr2['listChkSelect']=1
	
	// PUBLIC vars
	var $_mod;   // module name
	var $_sqlo;  // database handle
	var $_sqlo2; // 2nd database handle
	var $_sqlo3; // 3rd database handle
	var $_html;  // html lib
	var $_go;    // form-progress var
	private $oMODULE_feats; // features of plugin-MODULE-table-object (e.g. MXID)
	
	
	function __construct(&$sqlo) {
		$this->_sqlo = &$sqlo;
		$this->_sqlo2= NULL;
		$this->_sqlo3= NULL;
		$this->_mod = $_REQUEST['mod'];
		$this->_go  = $_REQUEST['go'];
		
		$this->_html = new gPlugHtmlLib($this->_mod, $this->_go);
		
	}
	
	// set $infoarr
	function register() {}
	
	/**
	 * start page head
	 
	 */
	function startHead() {
		
		$infoarr = &$this->infoarr;
		$this->_html->set_infoarr($this->infoarr);
		
		$this->pagelib = new gHtmlHead();
		$this->pagelib->set_infoarr2($this->infoarr2);
		
		if ($this->infoarr2['progressBar']>0) {
			require_once('f.progressBar.inc');
			
			$flushLib = new fProgressBar( ); 
			$infoarr['css'] = $infoarr['css'] . $flushLib->getCss();
			$infoarr['javascript'] = $infoarr['javascript'] . $flushLib->getJS(); 
			$this->_html->_flushLib = &$flushLib;
		}
		
		$show_startPage=1; // show full plugin header?
		
		if ($this->infoarr2['noHtmlHead']>0) {
			// no html-head
			$show_startPage=0;
		}
		
		if ($this->infoarr2['noHtmlBodyTable']) {
			// show <head> but not statndard-plugin-table after <body>
			$show_startPage=0;
			$this->pagelib->_PageHead ( $infoarr["title"],  $infoarr );
		}	
		 
		
		
		if ($show_startPage) {
			$this->pagelib->startPage($this->_sqlo, $infoarr);
		} else {
		    $this->pagelib->startPageNoHtml2($this->_sqlo, $infoarr);
		}
		
		if ($this->infoarr['active']<0) {
			$this->pagelib->htmlFoot('INFO', 'This module is deactivated.');
		}
		
		$this->__advancedCheck($this->infoarr2);
		
		if ($infoarr["form_type"]=='list' and $infoarr["obj_name"]!=NULL) {
			
			$tablename = $infoarr["obj_name"];
			$this->listLib = new gPlugListC($tablename);
			if ($this->infoarr2['listChkSelect']>0) {
			    
			    $list_opt=array();
			    if ($this->infoarr2['listCheckNumMax'] > 0 ) {
			        $list_opt['checkNumMax'] = $this->infoarr2['listCheckNumMax'];
			    }
			    $this->listLib->initCheck($this->_sqlo, $list_opt); 
			}
		}

	}
	
	/**
	 * - perform advanced checks
	 * - exit page on error
	 * - this function can be called separatly in the plugin
	 * @param array $infoarr2 description of checks
	 * @return ---
	 */
	function __advancedCheck(&$infoarr2) {
		// do an access check ?
		if ($infoarr2['accCheck']!=NULL) {
			$this->pagelib->do_objAccChk($this->_sqlo, $infoarr2['accCheck']);
			$this->pagelib->chkErrStop();
		}
		
		if ( $infoarr2['role_right_need'] ) {
		    $answer  = role_check_f( $this->_sqlo, $infoarr2['role_right_need'] );
		    if ($answer!='execute') {
		        $this->pagelib->htmlFoot('USERERROR', 'You can not execute this plugin due to missing role-right "'. $infoarr2['role_right_need'] .'".');
		    }
		}
		
		// admin-check
		if ($infoarr2['onlyAdmin']>0) {
			if ( !glob_isAdmin() ) {
			     $tmptext =  "Only root can execute this!".
			     	"For security reason it is not allowed for common users";
			     $this->pagelib->htmlFoot('USERERROR', $tmptext);
			}
		}
	}
	
	function __getInfoarr() {
		return $this->infoarr;
	}
	
	function _set_mod_features($mod_features) {
		$this->oMODULE_feats = $mod_features;
		if ($this->oMODULE_feats['user.fav.allow']>0) {
			$this->infoarr['user.fav'] = 1;
			$this->infoarr['MXID'] = $this->oMODULE_feats['MXID'];
		}
	}
	
	function _role_right_get(&$sqlo, $right_name ) {
	    $answer  = role_check_f( $sqlo, $right_name );
	    return $answer;
	}
	
	// MXID exists optional
	function _plug_get_MXID() {
		return $this->oMODULE_feats['MXID'];
	}
	
	// get URL of this plugin
	function _plug_get_URL() {
		return 'p.php?mod='.$this->_mod;
	}
	
	function startMain() {}
	
	// register the second SQL-handle
	function &_plug_regSql2() {
		if ($this->_sqlo2==NULL) {
			$this->_sqlo2  = logon2(  );
		}
		return $this->_sqlo2;
	}
	
    // register the third SQL-handle
	function &_plug_regSql3() {
		if ($this->_sqlo3==NULL) {
			$this->_sqlo3  = logon2(  );
		}
		return $this->_sqlo3;
	}
	
	/**
	* end plugin 
	* @param string $key 
	* ['NORM'],
	* 'WARN',
	* 'ERROR',
	* 'USERERROR',  - user error
	* 'INTERR' - internal error
	* 'DEPRECATED' - user has called a DEPRECATED tool
	* @param $text - just text
	*/
	function htmlFoot($key=NULL, $text=NULL) {
	    
		if ($this->infoarr2['noHtmlHead']>0) {
			// no html-head
		} else {
		    $vopt=array();
		    if ($this->infoarr2['noHtmlBodyTable']) {
		        $vopt['add_HR']=-1;
		    }
		    $this->pagelib->htmlFoot($key, $text, $vopt);
		}
	}
}
