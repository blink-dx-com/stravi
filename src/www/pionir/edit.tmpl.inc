<?php
/**
 * class for edit.tmpl.php
 * - can be extended by Code-files:
 *    - obj.{tablename}.xedit.inc
 *    - for FEATURE-tab: objlib/o.{tablename}.xedForm.inc
 * @package edit.tmpl.inc
 * @author  Steffen Kube
 *
 * @var array xmodes_STRUCT
 *   array(
 *      'default'=> KEY  [OPTIONAL]
 *      'modes'=> array(
 *        KEY=>array(
 *                --- the main key "features" can be contained, if should be hidden with "sh"=>0
 *                --- a LAB tab KEY starts with "xlab"
 *           'n'=> nice name, [REQUIRED]
 *           'sh'   => 0,1 show/hide the mode-tab [OPTIONAL]
 *           'co'   => color-code [OPTIONAL]
 *           'ic'   => icon file,
 *           'tact' => 0,1 -- has an tab activity code file ?
 *           )
 *      )
 *   )
 *   
 *  o.{TABLE}.edit.inc => STRUCT
 *  INPUT: 
 *     $id, 
 *     &$o_rights
 *     
 */

require_once ('f.objCache.inc');
require_once ('o.CCT_ACCLOG.subs.inc');
require_once 'o.S_VARIO.subs.inc';
require_once 'o.S_VARIO.mod.inc';
require_once ('f.help.inc');
require_once ('func_head2.inc');
require_once ('o.PROJ.paths.inc');
require_once 'o.proj.profile.inc';
require_once ('class.history.inc');


/**
 * extend an object for edit.tmpl.php
 *
 */
class gObj_editx_ABS {
    
    function __construct($tablename, $id) {
        $this->id = $id;
        $this->tablename=$tablename;
    }
    
    // save TAB activity cache
    function &_tab_act_save($sqlo, $mod_date) {
        $x = array('mod_date'=>$mod_date, 'data'=>$this->tab_data );
        $vario_lib = new oS_VARIO_mod();
        $options   = array('noAccCheck'=>1);
        $vario_lib->setObject($sqlo, $this->tablename, $this->id, $options);
        $vario_lib->set_touched(1);
        $vario_lib->updateKeyVal($sqlo, 'gui.cache.tabs', json_encode($x));
        
        return $x;
    }
    function &gui_cache_tabs_get() {
        return $this->tab_data;
    }
    function gui_cache_tabs_set(&$tab_data) {
        $this->tab_data = &$tab_data;
    }
    // can be implemented: function tab_activity_dyn($sqlo) {}
}

/**
 * extend an the OBJECT "feature area" for edit.tmpl.php
 *
 */
class gObj_edit_feat_ABS {
    
    public $argu;
    public $id;
    public $o_rights;
    public $viewmode;
    
    function __construct($id, &$argu, &$o_rights, $viewmode, &$colNames_ori, &$extraobj_o) {
        $this->id   = $id;
        $this->argu = &$argu;
        $this->o_rights = &$o_rights;
        $this->viewmode=$viewmode;
        $this->colNames_ori=&$colNames_ori;
        $this->extraobj_o = &$extraobj_o; 
    }
    
    protected function _sub_title($title) {
        echo '<img src="images/greenball.gif" hspace=5>'.
             '<span style="color:gray;">'.$title.':</span>';
    }

}


class fEditLibC {

var $_CLASSNAME = "fEditLibC";
var $object_is_bo;
var $mothertable;
var $tablename;
var $id;
private $ori_data; // original object data array: COL=>VAL
var $has_single_pk;
var $is_assoc; // 0,1
var $pkids; // [1] = second PK [2] = third PK
var $moObj; // array of mother object: 't', 'id'
private $tabmode;
/**
 * 
 * @var array $infox
 *   "obj_name"
 *   out_id
 *   'is_fav' 0,1 is favorite ?
 */
var $infox;



/**
 * array 
 *   'mod_date': date
 *   'data' : array
 **/
private $gui_cache_tabs=array();
private $objXtendCls=NULL; // objects extender classes

public $access_data=NULL;

function __construct($tablename, $id, $primasid) {
	$this->tablename = $tablename;
	$this->id = $id;
	$this->pkids = $primasid;
	$this->nicename = tablename_nice2($tablename);
    $this->objformXlib = new fObjFormSub();
    $this->objformXlib->setObj( $tablename, $id );
    
    $this->moObj   = $_REQUEST['moObj'];
    $this->tabmode = $_SESSION['s_sessVars']['o.'.$this->tablename.'.tabmode'];
    $this->_tool_buttons_vm='ALL'; // can be manipulated by set_tool_button_META()
    $this->back_url=urlencode('edit.tmpl.php?t='.$tablename.'&id='.$id);
}

function init($sqlo) {
	$this->object_is_bo   = (cct_access_has2($this->tablename)) ? 1 : 0;
	$this->mothertable    = mothertable_get2($this->tablename);
	$this->pk_arr         = primary_keys_get2($this->tablename); /* get primary keys */
	$this->has_single_pk  = empty($this->pk_arr[1]);
	$this->is_assoc       = $this->pk_arr[1] != "" ? 1 : 0;
	$this->obj_TABLE_TYPE = globTabMetaByKey($this->tablename, 'TABLE_TYPE'); // BO, BO_ASSOC, SYS
	
	$this->htmlHeadObj = new gHtmlHead();
	
	
	if ($this->is_assoc) $toolOpt = array(
			    "mother" =>$this->mothertable,
				"isAssoc" =>$this->is_assoc,
				"pk1"=>$this->pkids[1], 
				"pk2"=>$this->pkids[2] 
				);
	$this->objHeadLib = new fObjToolbar($this->tablename, $this->id, "single", $toolOpt);
	
	$this->infox = array();
	$this->infox["obj_name"] = "";
	$this->infox["out_id"]   = "";
	
	$this->headinf=NULL;
	
	$this->page_title = "Single object ID: ".$this->id. " table: ".$this->nicename;
	
	// include extension files
	$tablename_l= strtolower($this->tablename);
	$xtra_file  = "obj.".$tablename_l.".xedit.inc";
	
	$tmp_DEF_cls_exists=0;
	if ( file_exists($xtra_file) ) {
	    require_once($xtra_file);       // include object oriented extra functions
	    $tmp_cls_name = 'o_'.$this->tablename.'_editx';
	    if (class_exists($tmp_cls_name)) {
	        $tmp_DEF_cls_exists=1;   
	    }
	} 
	$scripLabPath = $_SESSION['s_sessVars']['AppLabLibDir'];
	$lab_special_file = $scripLabPath.'/obj.'.$tablename_l.'.xedit.inc';
	
	if ( $scripLabPath!=NULL) {
	    if ( file_exists($lab_special_file) ) {
	       require_once($lab_special_file);
	       $tmp_LAB_cls_name = 'o_'.$this->tablename.'_editxLAB';
	       if (class_exists($tmp_LAB_cls_name)) {
	           $this->objXtendCls = new $tmp_LAB_cls_name($this->tablename, $this->id);
	       }
	    }
	}
	if ( $tmp_DEF_cls_exists and  $this->objXtendCls==NULL) {
	    $this->objXtendCls = new $tmp_cls_name($this->tablename, $this->id);
	}
	
	if ($this->has_single_pk and is_numeric($this->id)) {
	    // load vario var ... 'gui.cache.tabs'
	    $tmp = oS_VARIO_sub::getValByTabKey($sqlo, $this->tablename, $this->id, 'gui.cache.tabs');
	    if ($tmp!=NULL) {
	        $this->gui_cache_tabs = json_decode($tmp, TRUE);
	    }
	}
	
	
}

function set_moObj($moObj) {
    $this->moObj= $moObj;
}
function set_tool_button_META() {
    $this->_tool_buttons_vm='meta';
    
}

function _infoout($text) {
	echo '<i>Info: '.$text.'</i>'."<br />\n";
}

function getEditMode() {
	$editmode = $_SESSION["s_sessVars"]["o.".$this->tablename.".editmode"];
	if ( $editmode == NULL ) $editmode = 'view';
	return $editmode;
}

/**
 * error message and exit
 * @param  $text
 * @param  $openbody
 */
function errorstop($text, $openbody=0) {
	
	echo "<ul><br><br>";
	htmlInfoBox( "Single object sheet Error", 
		$text , "", "ERROR" );
	echo "<br><br><a href=\"javascript:history.back()\">Back</a>";
	echo "</ul>\n";
	$this->htmlHeadObj->htmlFoot();
	exit;
}

function &getPkArr() {
	return ($this->pk_arr);
}

/**
 * check, if ID is valid and object exists
 * @param $sqlo
 * @return array( $ok, $errorstring)
 * 	 $ok > 0 : o.k.
 * 	     <=0 : error
 */
function checkID_ofObj(&$sqlo) {
	$tablename    = $this->tablename;	
	$idname 	  = PrimNameGet2($tablename);
	
	$dataType     = appDataTypeGet2($tablename, $idname);
	$ok = 0;
	$errorstring  = 'unknown';
	
	// check second, third primary keys
	if ( ($this->pk_arr[1]!=NULL and $this->pkids[1]==NULL) or 
	     ($this->pk_arr[2]!=NULL and $this->pkids[2]==NULL)    ) {
	    return array(-3, "A primary key is not given.");
	}
	
	if (empty($this->id)) { // do not check ID after this
		return array(-2, 'You must provide an ID');
	} 
	
	if ($dataType==2 or $dataType==6) {
		// is string, no check
		$ok = 1;
	} else {
		if (!is_numeric($this->id)) {
			$errorstring='ID must be numeric!';
			$ok = -1;
		} else $ok=1;
	}
	return array($ok,$errorstring);
}

function getColMode() {
	return $this->objformXlib->getColMode();
}

/**
 * create SQL query
 * @return 
 * @param object $pk_sel
 * @param object $primasid
 * @global $this->select_str
 * @global $this->colNames_ori
 */
function makeQueryStr( &$sqlo, &$pk_sel, &$primasid ) {
	$tablename    = $this->tablename;	
	$idname 	  = PrimNameGet2($tablename);
	// pk_arr exists !!!
	$select_str   = primary_select_str( $pk_sel, $primasid,  $idname, $this->id  ); 
	$this->colNames_ori = columns_get_pos ($tablename); // must be connected to $arguByCol

	$select_str2 = " where " . $select_str;
	
	// handle special data-types
	$validLib = new fValidate();
	$dtypes = $validLib->val_getTypes($tablename, $this->colNames_ori);
	$komma  = '';
	$sel_str_tmp = NULL;
	foreach( $this->colNames_ori as $col ) {
		if ($dtypes[$col]=='DATE1') {
			$addcol = $sqlo->Sql2DateString ( $col, 4 ). ' '.$col;
		} else $addcol = $col;
		$sel_str_tmp .= $komma . $addcol;
		$komma  = ', ';
	}
	
	$this->sqls_main   = "select ".$sel_str_tmp." from " . $this->tablename . $select_str2;
	$this->select_str = $select_str;
	
	return ($this->sqls_main);
}

/**
 * - get the main object features
 * - get $this->ori_data and $this->access_data
 * @return array $arguByCol
 * @param $sql
 * @todo generate CODE-column-names ! e.g. DATE for RESX : problem !!!
 */
function init_obj_data( &$sqlo ) {
    
    $this->access_data = NULL;
	$sqlo->query( $this->sqls_main );
	$sqlo->ReadArray();
	$this->ori_data = $sqlo->RowData;
	
	if ($this->object_is_bo) {
	    $CCT_ACCESS_ID = $this->ori_data['CCT_ACCESS_ID'];
	    $this->access_data   = access_data_getai($sqlo, $CCT_ACCESS_ID);
	}
	
	// analyse TAB activity
	if ( $this->objXtendCls!= NULL ) {
	    
	    if ( method_exists($this->objXtendCls, 'tab_activity_up') ) {
	        // analyse $this->gui_cache_tabs
	        $do_ana=1;
	        if ($this->gui_cache_tabs['mod_date'] == $this->access_data['mod_date']) {
	            $do_ana=0;
	        }  
	        if ($do_ana) {
	            $this->objXtendCls->tab_activity_up($sqlo);
	            $this->gui_cache_tabs = $this->objXtendCls->_tab_act_save($sqlo, $this->access_data['mod_date']);
	        } else {
	            $this->objXtendCls->gui_cache_tabs_set($this->gui_cache_tabs['data']);
	        }
	    }
	    if ( method_exists($this->objXtendCls, 'tab_activity_dyn') ) {
	        $this->objXtendCls->tab_activity_dyn($sqlo);
	        $this->gui_cache_tabs['data'] = $this->objXtendCls->gui_cache_tabs_get();
	    }
	}
	
	
	
	
	return $this->ori_data;
}

function getColNames() {
	return ($this->colNames_ori);
}

function mapArguToKey( &$colNames_ori, &$argu_load) {
	if ( !sizeof($colNames_ori) ) return;
	
	$arguByKey = NULL;
	foreach( $colNames_ori as $id=> $key ) {
		$arguByKey[$key] = $argu_load[$id];
	}
	return ($arguByKey);
}

/**
 * manage right mask
 * @param $t_rights
 * @param $o_rights
 * @return array ('ok'=>0,1, 'err'=>errortext, 'x'=>$x_rights)
 */
function rightMaskMng($t_rights, $o_rights) {
	$mothertable  = $this->mothertable;
	$object_is_bo = $this->object_is_bo;
	
	$this->x_rights=array();
	
	$x_rights = $o_rights;  // copy right array
	$answer   = array();
	$access_reason = NULL; // will not be analysed this time
	
	if ( !$o_rights["read"] ) {
		$answer = array('ok'=>0, 'err'=>"NO READ PERMISSION !<br>".
			"No read permission for this Object. Please ask the object-owner." , 1);
		return $answer;
	}
	
	if ($mothertable!=NULL && !$object_is_bo) {
		// inherit rights from mother 
		$x_rights["insert"] = $o_rights["insert"];
		$x_rights["write"]  = $o_rights["insert"];
		$x_rights["delete"] = $o_rights["insert"];
	}
	
	$access_reason="not owner";   
	
	if ( $t_rights["write"] != 1 ) {
		$x_rights["write"] = 0; // has higher priority
		$access_reason = "no role right";
	}
	
	if ( $t_rights["read"] != 1 ) {
		$infoTable = getTableAccessMsg( $this->nicename, "read" );
	    $answer = array('ok'=>0, 'err'=>$infoTable);
	    return $answer;
	} 
	
	/* for enhanced debug
	if ( $_SESSION["userGlob"]["g.debugLevel"]>2 )  {
		debugOut("some right-masks", "rightMaskMng" );
		echo "t_rights: ". print_r($t_rights,1) . "o_rights: ". print_r($o_rights,1) . "x_rights: ". print_r($x_rights,1).
			"<br>\n";
	}
	*/
	
	$this->x_rights = $x_rights;
	
	return array('ok'=>1, 'x'=>$x_rights);
}


/**
 * - search an object in the projects
   - show only names of project, maximum: 3 projects
 * @param object $sql
 * @param int $obj_id
 * @param array $except_ids -- except these PROJ_IDs
 * @return void|string
 */
private function _showProjsShort ( &$sql, $obj_id, $except_proj_ids=array()  ) {
	
	$table   = $this->tablename;	
	$projarr = array();
	
	$sql_add = '';
	if (isset($except_proj_ids)) {
    	if ( sizeof($except_proj_ids) ) {
    	    $sql_add = ' and PROJ_ID not in ('.implode(", ",$except_proj_ids).')';
    	}
	}
	
	$sqls = "select PROJ_ID from PROJ_HAS_ELEM where TABLE_NAME='".$table."' AND PRIM_KEY=".$obj_id. $sql_add . " order by PROJ_ID"; // " order by IS_LINK ASC";
	$sql->query($sqls);
	$cnt=0;
	while ( $sql->ReadRow() ) {
		$cnt++;
		if ($cnt>3) {
			break;
		} 
		$projarr[]   = $sql->RowData[0];
	}
	
	if (!sizeof($projarr)) return;
	
	$text = "";
	foreach( $projarr as $projid ) {
		$projname = obj_nice_name ( $sql, "PROJ", $projid );
		$text .= "<img src=\"images/icon.PROJ.gif\" hspace=2><a href=\"edit.tmpl.php?t=PROJ&id=".$projid."\">".$projname."</a>&nbsp;&nbsp;";
	}
	if ($cnt>3) $text .= "<img src=\"images/icon.PROJ.inact.gif\" hspace=2> <font color=gray> ... more </font>";
	
	return ($text);
}

/**
 * show project path(s)
 * SQL_QCOST: calls: minimum=6  time: searchProject() takes a long time ...
 * @param object $sql
 * @return void|string|void
 */
function showPath( &$sql ) {

	$infox3 = '';
	$projPathObj = new oPROJpathC ();
	// $typex = glob_elementDataGet( $sql, 'PROJ', 'PROJ_ID', $projidx, 'TYPEX');
	$need_br=0;
	
	if ($this->tablename =='ABSTRACT_SUBST') {
	    
	    // analyse categories here ...
	    
	    $sea_opt = array('non_cat'=>1);
	    list($projidx, $projexists) = $projPathObj->searchProject ( $sql, $this->id, $this->tablename, $sea_opt );
	    if ($projexists==1) {
	        $infox3 .= "<img src=\"images/icon.PROJ.gif\" hspace=2> ".$projPathObj->showPathSlim($sql, $projidx, "edit.tmpl.php?t=PROJ&id=", 0, 4); 
	        $need_br=1;
	    }

        // search for categories ...
        $cat_proj_ids     = $projPathObj->search_cat ( $sql, $this->id, $this->tablename );
        $cat_stop_proj_id = intval(glob_elementDataGet( $sql, 'GLOBALS', 'NAME', 'o.PROJ.category.start', 'VALUE'));
        $newline='';
        $cat_opt = array('ROOT_TXT'=>'/');
        if ($need_br) $newline="<br>\n";
        
        foreach($cat_proj_ids as $cat_proj) {
            $infox3 .= $newline . "<img src=\"images/icon.PROJ.cat.gif\" hspace=2 title='category'> ".
                $projPathObj->showPathSlim($sql, $cat_proj, "edit.tmpl.php?t=PROJ&id=", $cat_stop_proj_id, 4, $cat_opt);
            $newline="<br>\n";
            $need_br=1;
        }
	    
	    
        if ($projexists>1) {
            if ($need_br) $infox3 .= "<br>\n";
            $infox3 .= $this->_showProjsShort($sql, $this->id, $cat_proj_ids);
        }
  
	    
	} else {
	    
	    // no category analysis ...
	    
	    list($projidx, $projexists) = $projPathObj->searchProject ( $sql, $this->id, $this->tablename );
	    if (!$projexists) return;
	    
	    if ($projexists==1) {
	       $infox3 .= "<img src=\"images/icon.PROJ.gif\" hspace=2> ".$projPathObj->showPathSlim($sql, $projidx, "edit.tmpl.php?t=PROJ&id=", 0, 4);  
	    }
	    if ($projexists>1) {
	       $infox3 .= $this->_showProjsShort($sql, $this->id, $cat_proj_ids);
	    }
	    
	}

	if ($infox3!=NULL) {
	    $infox3 = '<p style="margin-top:4px; margin-bottom:0px;"> '.$infox3.'</p>'."\n";
	}
	
	return ($infox3);
}

// object is a favorite ?
function _obj_is_favorite($sqlo) {
    $help_lib = new oPROJ_bookmarks();
    $this->infox['is_fav'] = $help_lib->obj_is_in_proj($sqlo, $this->tablename, $this->id );
}

function historycheck() {
	if ($this->object_is_bo OR $this->has_single_pk) {
	    $hist_obj = new historyc();
		$hist_obj->historycheck( $this->tablename, $this->id );
		$this->historyDone=1;
	}
}

/**
 * get parent object (if one exists)
 * - support for ABSTRACT_PROTO, EXP and other 
	1. check history-array for related table
    2. if not found: full scan !
 * @param object $sqlo
 */
function getParentInfo( &$sqlo ) {

    $t = $this->tablename;
    $mo_table='';
    $chi2mo_arr= array(
        'ABSTRACT_PROTO' => array('AS_HAS_PR', 'EXP_TMPL_HAS_PROTO'),
        'CONCRETE_PROTO' => array('CS_HAS_PR','EXP_HAS_PROTO'),
        'EXP'=> 'EXP',
    );
    
    if (empty($chi2mo_arr[$t]) ) return;
    
    $mo_table=$chi2mo_arr[$t];

    if ($mo_table=='EXP') {
        if ($this->ori_data['MO_EXP_ID']) {
            $moObj = array('t'=>'EXP', 'id'=>$this->ori_data['MO_EXP_ID'], 'found'=>1);
            $this->set_moObj($moObj);
        }
        return;
    }
    
    $ObjCacheLib = new fObjCache();
    $moObj = $ObjCacheLib->guessMotherOneTab($sqlo, $this->tablename, $this->id, $mo_table);
   
    $this->set_moObj($moObj);

}

function listNextButton( &$sql ) {
	// FUNCTION: show next buttons
	
	$info = query_get_info($this->tablename);
	if ($info=="") return;
	
	echo "<a href=\"f.objnext.php?t=".$this->tablename."&id=".$this->id."&dir=-1\"><img src=\"images/but.arrow.prev.svg\" style=\"padding-right:5px;\" TITLE=\"previous\" height=18></a>"; 
	echo "<a href=\"f.objnext.php?t=".$this->tablename."&id=".$this->id."&dir=1\"><img src=\"images/but.arrow.next.svg\"  style=\"padding-right:5px;\" TITLE=\"next\" height=18></a>"; 
}

public function set_tabmode($tabmode) {
    $this->tabmode = $tabmode;
}

/**
 * get the destination code-file for the xmode extension ...
 * @param array $xmodes xmodes_STRUCT
 * @return array($filename_inc, $xmodeClass)
 */
function manageXmode($xmodes) {
    $tablename    = $this->tablename;
    $filename_inc = NULL;
    $xmodeClass   = NULL;
    $tablename_l  = strtolower($tablename);
    
    if ( empty($xmodes) ) {
        $currMode = 'features';
    } else {
        $currMode = $this->get_current_xmode($xmodes);
    }
        
    if ($currMode=='features') { 
        $filename_tmp = 'obj.'.$tablename_l.'.xmode.features.inc';
        if ( file_exists($filename_tmp) ) {
            require_once($filename_tmp);
        }
        return array(NULL, NULL);
    } 
    
    if ($currMode) {
        
        //  show an alternative page

        $xmodeFiBase = 'obj.'.$tablename_l.'.xmode.'. $currMode;
        $xmodeClass  = 'o'.$tablename.'_xmode_'. $currMode;
        $filename_inc= $xmodeFiBase.".inc";
        if ( substr($currMode,0,4)=="xlab" ) {
            $scripLabPath = $_SESSION['s_sessVars']['AppLabLibDir'];
            $filename_inc= $scripLabPath."/" . $filename_inc; // take lab specific
        }
    }

    return array($filename_inc, $xmodeClass);
}

/**
 * - get the xmodes
 * - manage tab activity 
 * @return array xmodes_STRUCT
 *
 */
function getXmodes($sqlo) {
    
    $edit_tabs = function_exists('x_modes_get') ? x_modes_get() : array(); // get main view modes, controlled by riders
    
    if ( $_SESSION['globals']["lab_path"]!="" ) {
        // OLD: x_modes_getLab()
        $file_lab = "../".$_SESSION['globals']["lab_path"]. "/config_lab/o.".$this->tablename.".edit.inc";
        if ( file_exists($file_lab) ) {
            include($file_lab);   		// object dependend menu in lab
        }
    } 
    return $edit_tabs;
}

/**
 * get current xmode; default: "features"
 * INPUT:  $this->tabmode; can be set by  set_tabmode()
 * @param string $xmodes
 */
private function get_current_xmode(&$xmodes) { 
    $now_mode = $this->tabmode;
    $default_mode='features';
    if ($xmodes['default']!=NULL) {
        $default_mode = $xmodes['default'];
    }
    if ($now_mode==NULL) {
        $now_mode=$default_mode;
    } else {
        // check allowed the "sh" flag ...
        if (is_array($xmodes['modes'][$now_mode])) {
            if ( array_key_exists('sh', $xmodes['modes'][$now_mode]) and $xmodes['modes'][$now_mode]['sh']==false) {
                $now_mode=$default_mode; // not allowed => take default 
            }
        }
    }
    return $now_mode;
}

/**
 * show object tabs (reiter) $xmodes
 * @param array $xmodes xmodes_STRUCT
 */
private function _objTabs(&$xmodes) {

	$tablename= $this->tablename;
	$id = $this->id;
	$color_def = '#3366cc';
	
	$now_mode = $this->get_current_xmode($xmodes);
	
	$has_tab_activity=0;
	if ( !empty($this->gui_cache_tabs['data']) ) $has_tab_activity=1;
	
	echo '<td bgcolor=white style="padding-left:8px;"></td>';
	
	$xmodes_p = $xmodes['modes'];
	if (!array_key_exists("features", $xmodes_p)) {
	    $xmodes_p = array_merge( array("features"=>array('n'=>"features")), $xmodes_p  );  // to front 
	}
	foreach( $xmodes_p as $xmode => $xm_feats ) {
	    
	    $show_flag = $xm_feats['sh'] ?? 1;
	    $color     = $xm_feats['co'] ?? $color_def;
	   
	    
	    if ($show_flag) { // a tab can be hidden, e.g. the feature tab ... ...
	        
	        $tab_text=$xm_feats['n'];
	        if (isset($xm_feats['ic'])) {
	            $tab_text=  '<img src="images/'.$xm_feats['ic'].'" style="vertical-align: text-bottom;" vspace=1 height=20> '.$tab_text;
	        }
	        
    	    if ($now_mode==$xmode) {
    	        echo '<td bgcolor='.$color.'><span style="color:white">&nbsp;<B>'.$tab_text."</B>&nbsp;</span></td>";
    		} else {
    		 
    		  if ($has_tab_activity) {
    		     if ($this->gui_cache_tabs['data'][$xmode]>0) {
    		         $tab_text.=' <span class="yBadge" data-badge=""></span>';
    		     }
    		  }
    		  echo '<td>&nbsp;<a href="edit.tmpl.php?t='.$tablename.'&id='.$id. '&xmode='.$xmode.'" style="color:'.
        		  $color.'">'.$tab_text.'</a>&nbsp;</td>';
    		}
    		echo "<td bgcolor=white>&nbsp;</td>";
	    }
	} 
	
}

function setInfoxName(&$sql) {
	if ($this->is_assoc AND $this->mothertable!="") $usetable=$this->mothertable;
	else $usetable=$this->tablename;
	$this->infox["obj_name"] = obj_nice_name($sql, $usetable, $this->id); // TBD: get name again? check if object was updated ...
	$this->infox["out_id"]   = $this->id;
}

/**
 * show <head> and <body> tags
 */
function showHead() {
	if ($this->has_single_pk) {
		$obj_name = $this->infox["obj_name"];
		if (strlen($obj_name)>40) $headTitleName = substr($obj_name,0,40)." ...";
		else $headTitleName = $obj_name;
		$headTitleName = htmlspecialchars($headTitleName);
	} else {
		$headTitleName = $this->pk_arr[1].":".$this->pk_arr[2];
	}	
	
	$styleCss = "<style type=\"text/css\">\n";
	$styleCss .= $this->objformXlib->datatab_css();
	$styleCss .= "</style>\n";
	// $incCss = $this->htmlHeadObj->getIncludeCss();
	
	$pgopt = array('logModulExt'=>'t='.$this->tablename);
	$this->htmlHeadObj->_PageHeadStart('['.$this->id.'] '.$headTitleName.' -- '.$this->nicename, $pgopt);
	
	// . $incCss
	echo $styleCss  .
	'<link rel="stylesheet" type="text/css" href="res/css/glob.menu2.css?d=1" />'."\n".
	'<script type="text/javascript" src="res/js/jquery-3.5.1.min.js"></script>'."\n";
	
	// Older or non-needed versions of JS addons
	//jQuery UI:  '<link type="text/css" href="res/css/redmond/jquery-ui-1.8rc3.custom.css" rel="stylesheet" />'."\n".
	//DATEPICKER: '<link type="text/css" href="res/css/dp_custom.css" rel="stylesheet" />'."\n".
	// '<script type="text/javascript" src="res/js/jquery-ui-1.8rc3.custom.min.js"></script>'."\n".
	//'<script type="text/javascript" src="res/dp_conf.js"></script>'."\n";
	//echo '<link rel="stylesheet" href="https://ajax.googleapis.com/ajax/libs/jqueryui/1.11.2/themes/smoothness/jquery-ui.css">'."\n";
	//echo '<link rel="stylesheet" href="https://netdna.bootstrapcdn.com/bootstrap/3.1.1/css/bootstrap.min.css">'."\n";
	//echo '<script src="https://netdna.bootstrapcdn.com/bootstrap/3.1.1/js/bootstrap.min.js"></script>'."\n";
	
	
	echo '<link rel="stylesheet" type="text/css" href="res/css/jquery-ui.1.12.1.css" />'."\n"; // needed for datepicker : date fields
	echo '<script type="text/javascript" src="res/js/jquery-ui.1-12.1.js"></script>'."\n";     // needed for datepicker : date fields
	echo '<script type="text/javascript" src="res/js/underscore-min.js"></script>'."\n";       // needed for o.PRA_editor.js
	echo '<script type="text/javascript" src="res/dp_conf.js?dummy=1"></script>'."\n";  // config for datepicker
	$this->htmlHeadObj->_PageHeadEnd();
	

	js_formAll();
	js__open_info();
}




/**
 * check, if SUA has a version
 * @param object $sqlo
 * @return $headString
 */
private function _oSUAgetVers( &$sqlo ) {
	if (!glob_column_exists('ABSTRACT_SUBST', 'VERSFLAG')) {
		return;
	}
	$sua_feats = glob_elemDataGet3( $sqlo, 'ABSTRACT_SUBST', array('ABSTRACT_SUBST_ID'=>$this->id),
			 array('VERSFLAG') );
			 
	if ($sua_feats['VERSFLAG']<1) return; // not a version
	
	if ($sua_feats['VERSFLAG']==1) {
		return '&nbsp;&nbsp;&nbsp;VersionMaster';
	}
	
	$retval = NULL;
	
	$vers_feats = glob_elemDataGet3( $sqlo, 'SUA_VERS', array( 'CHILD_ID'=>$this->id), array('XVERS') );
	if ($vers_feats['XVERS']>0) {
		$retval = '&nbsp;&nbsp;&nbsp;<span style="color:#999999;">Version:</span> '.$vers_feats['XVERS'];
	}
	return $retval;
}

/**
 * prepare toolbar
 * @global $this->headinf['editBut'] (OUTPUT)
 * @param array $infox2
 *   'slim' => string
 *   'NamePost'=> string
 * return: &$info_tab, &$infox2
 */
function toolbarPrep(&$sql, &$x_rights, &$access_data, $formslim, &$info_tab, &$infox2 ) {
	
	$tabx = $this->tablename;
	
	if ( $formslim != 1 ) {
		if ( $access_data['owner'] ) {	
			$info_tab = '<span style="color:#999999;"><img src="images/ic.usergr2.gif" title="owner" border=0 hspace=2> '.$access_data['owner'].
				' <img src="images/ic.cread.gif" title="creation" border=0> '.$access_data['crea_date'];
			if ($access_data['mod_date'])   $info_tab .= ' <img src="images/ic.modd.gif" title="modification" border=0> '.$access_data['mod_date'];
			if ($access_data['modifier'])   $info_tab .= ' <img src="images/ic.usergr3.gif" title="modifier" border=0> '.$access_data['modifier'];
			$info_tab .= '</span>';
			
			if ($access_data['h_log_name']) {
				//$auditLib = new oAccLogC();
				$AudColors= oH_ALOG_ACT_subs::$auditColor;
				$theColor = $AudColors[$access_data['h_log_name']];
				if ($theColor!=NULL) $theColor='background-color:'.$theColor;
				$info_tab .= ' <img src="images/ic.accAudit.gif" title="audit status" border=0> '.
					'<span style="'.$theColor.'">&nbsp;'.$access_data['h_log_name'].'&nbsp;</span>';
			}
			if ($access_data['wflow_id']) {
				$info_tab .= ' <img src="images/icon.MODULE.gif" title="workflow active" border=0>';
			}
			
		}
		
		if ($tabx=="ABSTRACT_PROTO" or $tabx=="CONCRETE_PROTO" or $tabx=="EXP") {
		    $this->getParentInfo( $sql );
		}
		
		if ( $this->object_is_bo ) {
			$info_tab .= $this->showPath( $sql );
			$this->_obj_is_favorite($sql);
		}
		// if ($tabx=="CONCRETE_PROTO") {
		// 	$info_tab .= $this->getParentInfo( $sql );
		// }
		
		if ($tabx=="ABSTRACT_SUBST") {
			$infox2['NamePost'] = $this->_oSUAgetVers( $sql );
		} 
		
		
	} else { 
		// nothing
	}	
	
	$editmode = $this->getEditMode();
	if ( $editmode == "edit" ) {
		$but_view="images/but.view.gif";
		$but_edit="images/but.edit.ac.gif?dx";	
		if (!$x_rights["write"]) $but_edit="images/but.edit.ins.gif";	// only insert possible
	} else {
		$but_view="images/but.view.ac.gif?dx";
		$but_edit="images/but.edit.gif";
	}

	if ( $editmode == "edit" and !$x_rights["write"] and !$x_rights["insert"]) {
		$but_edit="images/but.edit.no.gif";
	}
	
	$this_obj = fObjFormSub::urlGetPrimkeys( $this->id, $this->pk_arr, $this->pkids ) ;
	
	$this->headinf['editBut']  = 
		'<a href="edit.tmpl.php?t='.$tabx.'&editmode=view&'.$this_obj.'" >' .
	 	'<img src="'.$but_view.'" TITLE="view" border=0></a>' .
		'<a href="edit.tmpl.php?t='.$tabx.'&editmode=edit&'.$this_obj.'" >'.
		'<img src="'.$but_edit.'" TITLE="edit" border=0></a>';
							
}

private function _meta_buttons() {
    echo '</td><td bgcolor=white style="padding-left:12px;" ></td><td style="padding-left:5px; padding-right:5px;">';
    echo '<a href="sessVars_set.php?variable=o.'.$this->tablename.'.tabmode&val=0meta&backurl='. $this->back_url .'">';
    echo '<img src="images//ic.meta.svg" TITLE="META info" height=20 ></a>';
    echo ' <a href="glob.obj.access.php?t='.$this->tablename."&id=".$this->id.'">';
    echo '<img src="images/but.access.gif" TITLE="access permissions"></a>';
    
}

/**
 * show slim object-HEAD
 * @return 
 * @param object $infox2
 * @param object $xmodes xmodes_STRUCT
 * @global $this->headinf['editBut'] (INPUT)
 */
function objHeadSlim(&$infox2, &$xmodes) {

	
    $this->objHeadLib->FSlim( $this->infox["obj_name"], $this->infox["out_id"], NULL, $xmodes );
	$this->objHeadLib->toolbar_end();
	
	echo "<table cellpadding=0 cellspacing=0 border=0 bgcolor=#efefef><tr>";
	echo '<td bgcolor=#C7D0D9 width=25><a href="preferences.php?m=g.sof.opt&var=formslim&backurl='.$this->back_url.'&val=">'.
	   	'<img src="res/img/chevron-right.svg" vspace=2 height=20 TITLE="full view"></a></td>'; // images/ic.arig.gif
	echo "<td>";
	echo "<img src=\"images/0.gif\" height=22 width=1>&nbsp";
	echo $this->headinf['editBut']; 
	echo "&nbsp;&nbsp;";
	echo "<a href=\"view.tmpl.php?t=".$this->tablename."\">".
		"<img src=\"images/but.list2.gif\" TITLE=\"list\" border=0 vspace=2></a>&nbsp;";
	echo "</td>";
	
	if ( $this->object_is_bo and $this->_tool_buttons_vm=='ALL') {
	    $this->_meta_buttons();
	}
	if ( sizeof($xmodes) ) {
		$this->_objTabs($xmodes);
	}
	echo "</tr></table>\n";    
	
}

/**
 * show FULL object-HEAD
 * @param object $sql
 */
function objHeadFull (&$sqlo, $CCT_ACCESS_ID, &$infox, &$xmodes, $info_tab, &$x_rights, &$o_rights) {
    
    $hasManiRights=0;
    // put query here to prevent the query on SLIM-view
    if ($CCT_ACCESS_ID) $hasManiRights = access_hasManiRights( $sqlo, $CCT_ACCESS_ID );
    $tolopt = array(
        "name_obj"=> $this->infox["obj_name"],
        "obj_id"  => $this->infox["out_id"],
        "menushow"=> 1,
        "NamePost"=> $infox["NamePost"]
    );
    
    
    if ( is_array($this->moObj) ) {
       
        if ($this->moObj['id']) {
            $moObj = $this->moObj;
            $moTable2 = mothertable_get2($moObj['t']);
            if ($moTable2!=NULL) {
                $moObj['t'] = $moTable2; // overwrite mother table
            }
            $nice = obj_nice_name ( $sqlo, $moObj['t'], $moObj['id'] ); 
            $moObj['nice']=$nice;
            $tolopt['mo_obj'] = $moObj;
            
            
        }
    }
    
    
    $this->objHeadLib->toolbar_show($sqlo, $info_tab, $tolopt );
    $this->objHeadLib->toolbar_end();
    $this->toolButtons( $sqlo, $this->pkids, $x_rights, $o_rights, $xmodes, $hasManiRights );

}

// 
/**
 * show Toolbar buttons and navi-tabs
 * INPUT: $this->_tool_buttons_vm : 
 *    ['ALL'] or
 *    ['meta']
 * @param $sql
 * @param array $primasid primary keys
 * @param $x_rights table-rigths
 * @param $o_rights object rigths
 * @param $xmodes array xmodes_STRUCT
 * @param boolean $hasManiRights 0,1 manipulation rights
 * @return -
 */
function toolButtons( &$sql, &$primasid, &$x_rights, &$o_rights, 
	&$xmodes, $hasManiRights ) {
	
	$tablename = $this->tablename;
	$id = $this->id;

	echo "<table cellpadding=0 cellspacing=0 border=0 bgcolor=#efefef><tr>";
	echo '<td bgcolor=#C7D0D9><a href="preferences.php?m=g.sof.opt&var=formslim&val=1&backurl='.$this->back_url.'">'.
	   	'<img src="res/img/chevron-up.svg" hspace=4 vspace=0 height=20 TITLE="slim view"></a></td>'; //  images/ic.arup.gif
	echo "<td valign=middle>";
	echo "<img src=\"images/0.gif\" height=22 width=1>&nbsp";
	if ($this->_tool_buttons_vm=='ALL')  {
	   echo $this->headinf['editBut'];
	   echo '&nbsp;&nbsp;&nbsp;';
    }
	
	echo "<a href=\"subs/clipboard.php?action=copy&tablename=".$tablename."&ida=".$id.
		 "&idb=".$primasid[1]."&idc=".$primasid[2]."\">";
	echo "<img src=\"images/but.copy.gif\" TITLE=\"copy\" border=0></a>";
	
	if (!empty($x_rights['delete'])) {
		echo '<a href="glob.obj.delete.php?tablename=',$tablename,'&id[0]=',$id;
		if (isset($primasid[1])) echo '&id[1]=',$primasid[1];
		if (isset($primasid[2])) echo '&id[2]=',$primasid[2];
		echo '"><img src="images/but.delete.gif" TITLE="delete from database" border="0"></a>';
	} else {
		echo '<img src="images/but.delete.low.gif" TITLE="delete" border="0">';
	}
	echo "&nbsp;";
	
	// check rights
	if ( $this->object_is_bo ) {
		$hasEntail = 0;
		$auditActUrl="<a href= \"glob.obj.acclock.php?t=".$tablename."&id=".$id."\">";
		if ( $o_rights["entail"] ) $hasEntail = 1;
		if ($hasManiRights) {
			if ( $hasEntail ) $accLock= $auditActUrl."<img src=\"images/but.lock.do.gif\" TITLE=\"lock object\" border=0></a>";
			else  $accLock   = $auditActUrl."<img src=\"images/but.lock.un.gif\" TITLE=\"is unlocked\" border=0></a>"; 
		} else {
			$accLock=$auditActUrl."<img src=\"images/but.lock.in.gif\" TITLE=\"is locked\" border=0></a>"; 
		}
		echo $accLock;
		
		$fav_ico = 'images/but.heart.png'; 
		$tmp_text='add to favorites';
		$tmp_a = 'p.php?mod=DEF/g.obj.favact&t='.$tablename.'&id='.$id.'&act=add';
		if ($this->infox['is_fav']) {
		    $fav_ico = 'images/but.heart_red.png'; 
		    $tmp_text='remove from favorite list';
		    $tmp_a = 'p.php?mod=DEF/g.obj.favact&t='.$tablename.'&id='.$id.'&act=del';
		}
		echo '&nbsp<a href="'.$tmp_a.'"><img src="'.$fav_ico.'" height=20 TITLE="'.$tmp_text.'" border=0></a>'; 

		
	}
	echo "&nbsp;&nbsp;";
	echo "<a href=\"view.tmpl.php?t=$tablename\"><img src=\"images/but.list2.gif\" TITLE=\"list\" border=0 ></a>&nbsp;"; // vspace=2
	$this->listNextButton($sql);
	
	if ( $this->object_is_bo and $this->_tool_buttons_vm=='ALL') {
	    $this->_meta_buttons();
	}
	
	echo "</td>\n";
	
	/* RIDER */  
	if ( sizeof($xmodes) ) {
		$this->_objTabs($xmodes);
	}
	echo "</tr></table>\n";    
}

  // look for advanced columns
function hasAdvCols(&$sql) {
	$sqls = "select count(1) from CCT_COLUMN where TABLE_NAME='".$this->tablename."'".
				" AND VISIBLE=2"; 
	$sql->query($sqls);
	$sql->ReadRow();
	return ($sql->RowData[0]);
}



/**
 * get one mother object of this object/element
 * @return array $moObj ('t', 'id', 'found' )
 */
function hasMother( &$sqlo ) {
	$moObj     = $this->moObj;
	$moTable   = $this->moObj['t'];
	$moId	   = $this->moObj['id'];
	$found     = $this->moObj['found'];
	$tablename = $this->tablename;
	$id 	   = $this->id;
	
	if ( $moTable!=NULL and !glob_table_exists($moTable) ) {
		$moTable = NULL;
		$moId    = NULL;
	}
	
	$moObj = NULL;
	$ObjCacheLib = new fObjCache();
	if ( $tablename=='CONCRETE_PROTO' ) {
		$moObj = $ObjCacheLib->guessMother( $sqlo, $tablename, $id, $moTable, $moId, $found );
	}
	return ($moObj);
}

/**
 * manage dynamic columns
 * @param object $sql
 * @return -
 */
function manageDyn(&$sqlo, $editAllow, $hasAdvCols) {
	require_once ('gui/o.S_VARIO.ed.inc');
	require_once ('o.S_VARIO.mod.inc');
	
	$url = 'glob.obj.S_VARIO.php';
	$tablename = $this->tablename;
	$id = $this->id;


	$hiddenarr=NULL;
	$hiddenarr['go']=1;
	$formLib = new globObj_S_VARIO_ed($tablename, $id);
	$formLib->initForm($editAllow, $url, $hiddenarr);
	$formLib->openTable($hasAdvCols);
	$valArr = $formLib->getData($sqlo, $id);
	$prefArr=NULL;
	$formLib->showFields( $sqlo, $valArr, $prefArr);
	$formLib->close();
	
	echo '<br /><ul>';
	$helpLib = new fHelpC();
	$hlpopt=NULL;
	$helpText = $helpLib->link_show("o.S_VARIO.html", "help", $hlpopt);
	$this->_infoout('This form shows VARIO-attributes of this object. ['.$helpText.']');
	$this->_infoout('The Admin can define new VARIO-attributes.');
	echo ' [<a href="view.tmpl.php?t=S_VARIO_DESC&searchMothId='.$tablename.'">'.
		'Defined Vario-Attribute-Keys for this table</a>]<br>'."\n";
	echo "</ul>\n";
}

/**
 * manage attachments
 * @param object $sqlo
 * @return -
 */
function manageAttach(&$sqlo, $editAllow, $hasAdvCols) {
    require_once ('gui/o.SATTACH.ed.inc');
    
    $tablename = $this->tablename;
    $id = $this->id;
    
    
    
    $formLib = new o_SATTACH_ed($tablename, $id);
    //$formLib->initForm($editAllow, $url, $hiddenarr);
    $formLib->openTable($hasAdvCols);
    $formLib->table_close();
    
    $formLib->show_form($sqlo, $tablename, $id, $editAllow);
    
}

/**
 * extract columns for colmode=NORM or 2
 * @param $colNames_ori
 * @param $arguByCol
 * @return array
 */
function adjustColMode($colNames_ori, $arguByCol) {
	$tablename = $this->tablename;
	$colNamesNew  = NULL;
	$arguByColNew = NULL;
	$idname    = PrimNameGet2($tablename);
	
	$expectViewVal=1;
	if ($this->getColMode()!='NORM') {
		$expectViewVal=2;
	}
	
	foreach( $colNames_ori as $colName ) {
	// foreach( $colNames_ori as $dummy=>$colName) {
		$colFeatures = colFeaturesGet2($tablename, $colName );
		$useCol=0;
		if ($colFeatures['VISIBLE']==$expectViewVal) $useCol=1;
		if ($idname==$colName) $useCol=1; // always show the ID
		if ($useCol) {
			$colNamesNew[]=$colName;
			if (isset($arguByCol[$colName])) $arguByColNew[$colName]=$arguByCol[$colName];
		}
	}
	
		
	return array($colNamesNew, $arguByColNew);
}

function pageEnd() {
	echo '<br><hr noshade size=1>'."\n";
	echo "</body>\n</html>\n";
	exit;
}

function &get_access_data() {
    return $this->access_data;
}

}

