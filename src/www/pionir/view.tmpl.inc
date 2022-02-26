<?php
require_once("class.history.inc");
require_once('o.CCT_ACCLOG.subs.inc');
require_once('db_x_obj.inc');
require_once('glob.obj.conabs.inc');
require_once('o.S_VARIO.subs.inc');

require_once('lev1/f.search_ulib.inc');
require_once 'export/view.tmpl.extra.inc';

/**
 *  sub routines for view.tmpl.php and other plugins
 * 
 * - can be EXTENDED
 * - prepare searches
 * - show class selection form
 * - print one element line
 * - used also in f.tsearch.inc
 *
 * @package view.tmpl.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @var selectCols_STRUC $selectCols
 *   "x.".$colName 	=> $column_code ,  // TBD: introduce a rela KEY_>VALUE pair, e.g. 'fullcol'=>, 'col'=>
 	     "show"		=> 0|1,
	     "nice"		=> "nice name"    (of column)
 	     "key"		=> $ret_imp_name,
	     "tab"		=> $linked_tab, 
	 ["fk_pname"]	=> f_primary_name
	 	 "tdAdd"	=> additiv for <td> cell-description
	 	 "for"      => SQL-formatting string, e.g. TO_CHAR(x.CREA_DATE,'YYYY-MM-DD')
 */
class viewSubC {

    public $access_id_has;
    public $pk_name;
    public $tablename;
    
    var $foreign_keys;
    var $prColpos;    /* @var array $prColpos of key,val-pairs for REMOTE-selection
    			       "cctgobacol" */
    var $viewmode;
    var $modebase;    // "lean"
    var $view_opt;	  /* @var view_opt_STRUC :  options from User
    	"colCoNa"
    	"CelSize"
    	"CelSizeOk"
    	
    		*/
    var $internViewOpt; /* fix intern view options
    	"checkbox"  0,1 show checkbox ?
    	*/
    var $rowSettings; /* row-settings:
    					'auditColor'=>1 color row by audit-setting
    					*/
    var $roleCache;   // array of checked role-rights
    
    var $selectCols;  // array of columns-names, index: virtual POS, created in colinfoget()
    var $class_params_show; // array( $classname,  $c_attrib, $tmp_sel_col );
    var $colNames_show; /* array of column-inf, which will shown as table-header 
        - created in headRowPrep()
    	array ( 
    		"cc"=>$column_code, 
    		"ce"=>$column_ext_name,
    		"show" => $show_flag, 
    		"nice" => $niceCol, 
    		"app" => $app_type, 
    		'tdAdd'=>$valarr['tdAdd'] 
    		);
    */

    private $_tab_cols_pref; // user prefered columns
    
    var $userSqlObj; /**
      @var object $userSqlObj class myquery
    */
    
    var $xcols; // columns from file "obj.".$tablename_l.".xfunc.inc";


    /**
     * 
     * @param string $tablename
     * @param array $options
     *   'cols_DEFAULT' => [1] , -1
     */
    function __construct($tablename, $options=array() ) {
        global $varcol;
        $varcol = & Varcols::get();
        
    	$this->tablename=$tablename;
    	$this->primas 	     = primary_keys_get2($tablename);
    	$this->pk_name 		 = $this->primas[0]; // main primary key
    	$this->access_id_has = cct_access_has2($tablename);
    	$this->AuditLib = new oAccLogC();
    	$this->VarioLib = new oS_VARIO_sub($tablename);
    	
    	$this->_tab_cols_pref=array(); // array( COL => $SHOW_FLAG: 0,1 )
    	$this->_tab_cols_pref_sort=array();
    	
    	if ( $options['cols_DEFAULT']>=0 ) {
        	if ($_SESSION['userGlob']['o.'.$tablename.'.viewcols']!=NULL) {
        	    $prefs = unserialize($_SESSION['userGlob']['o.'.$tablename.'.viewcols']);
        	    $this->set_tab_cols_pref( $prefs );   
        	}
    	}
    	
    	$classes    = $varcol->get_class_names($tablename);
    	$this->class_tab_has = sizeof($classes);
    }
    
    public function set_tab_cols_pref(&$prefs) {
        $this->_tab_cols_pref = &$prefs;
        $i=0;
        foreach($this->_tab_cols_pref as $col_l=>$val_l) {
            $this->_tab_cols_pref_sort[$col_l]=$i;
            $i++;
        }
    }


/**
 * set view options
 * @param array $params
 *   "viewmode"
	 "modebase"
	 "view_opt" see view_opt_STRUC
	 'internOpt' e.g. checkbox
 * @return -
 */
function prViewModeSet(&$params) {
	// set view-mode parameters
	$this->viewmode = $params["viewmode"];
	$this->modebase = $params["modebase"];
	$this->view_opt = $params["view_opt"];
	$this->internViewOpt=$params['internOpt'];
	
	
	// set default value ...
	$this->view_opt["CelSizeOk"] = $this->view_opt["CelSize"];
	if ( ($this->view_opt["CelSize"]=="") OR ($this->view_opt["CelSize"]<5) ) $this->view_opt["CelSizeOk"] = 30;
}

public function set_view_opt($key, $val) {
    $this->view_opt[$key] = $val;
}
public function get_view_opt($key) {
    return $this->view_opt[$key];
}

/**
 * - get the COLUMN-section of the SQL-SELECT command
 * - columns-positions are sorted
 * - define output format (optional)
 * @return $selectstr
 * @param object $selectCols
 */
function sqlColString(&$selectCols) {
 
	// ksort ($selectCols); // OLD: sort columns by position flag
	$komma_str= "";
	$cntcol   = 0;
	$selectstr='';
	foreach( $selectCols as $colFeat) {
	
		$db_key = key($colFeat);
		if ($colFeat['for']!=NULL) { // add format-string
			$db_key = $colFeat['for']; // TBD: if you need the column name use :  $colFeat['for'].' as '.$db_key;
		}
		$selectstr = $selectstr . $komma_str . $db_key;
		$komma_str =",";
		
		$cntcol++;
	}
	if ( $selectstr == "")  $selectstr="*";
	
	return $selectstr;

}

/**
 *  get class params
 *  OUTPUT:  $this->class_params_show
	//		  $this->c_attrib
 * @param object $sql
 * @param string $classname
 */
function getClassParams(&$sql, $classname) {
	
	$c_attrib=array(); 
	$sql->query('SELECT y.extra_attrib_id, y.map_col, y.APP_DATA_TYPE_ID FROM extra_class x, extra_attrib y '.
				'WHERE x.name='.$sql->addQuotes($classname).' AND x.extra_class_id = y.extra_class_id order by y.POS');
	$cnt=0;
	$tmpcomma="";
	$tmp_sel_col = ""; // SQL select string
	while ( $sql->ReadRow() ) {
		$c_attrib[$cnt][0]=$sql->RowData[0];
		$c_attrib[$cnt][1]=$sql->RowData[1];
		$c_attrib[$cnt][2]=appDataTypeNameGet2( $sql->RowData[2] );
		$tmp_sel_col = $tmp_sel_col .$tmpcomma. $sql->RowData[1];
		$tmpcomma=",";
		$cnt++;
	}
	$this->c_attrib = &$c_attrib;
	$this->class_params_show = array( $classname,  $c_attrib, $tmp_sel_col );
}

/**
 * manage extra columns for print_row()
 * OUTPUT: 
 *		$this->xcols
 *      $this->xcol_def 
 * @param object $sql
 */
public function xColsManage( &$sql ) {
	// global 	 $error;
	
    $this->xcols = array();
	$tablename = $this->tablename;
	
	$tablename_l = strtolower($tablename);
	$fileXfunc   = dirname(__FILE__)."/obj.".$tablename_l.".xfunc.inc";
	if (file_exists($fileXfunc)) {
	    
	    // include TABLE dependend columns 
		require_once($fileXfunc);
		
		$tmp_func= "c".$this->tablename;
		$this->objFuncLib = new $tmp_func();
		
		
		if (is_array($this->objFuncLib->xcolarr)) {
		    if (sizeof($this->objFuncLib->xcolarr)) {
		        
		        foreach( $this->objFuncLib->xcolarr as $th1 ) {
		            $col_text = current($th1);
		            $col_code = key($th1);
		            $viewcol_name="_xcol." .$col_code;
		            if ( $this->_tab_cols_pref[$viewcol_name] > 0 ) {
		                $this->xcols[$col_code] = $col_text;
		            }
		        }
		    }
		}
	}
	
	$extra_lib       = new view_tmpl_extras($tablename);
	if ($this->access_id_has) {
	    
	    $this->xcol_def = $extra_lib->get_xcol_def();
	    foreach($this->xcol_def as $col_code=>$row) {
	        $viewcol_name="_xcol." .$col_code;
	        $col_text = $row[0];
	        if ( $this->_tab_cols_pref[$viewcol_name] > 0 ) {
	            $this->xcols[$col_code] = $col_text;
	        }
	    }
	    // standard extended columns
// 	    $xcols_new = $extra_lib->get_xcols_user_set();  //TBD:
// 	    if (!empty($xcols_new)) {
// 	        if (!is_array($this->xcols)) $this->xcols=array();
// 	        $this->xcols = array_merge($this->xcols, $xcols_new);
// 	    }
	    
	    
	}
	
	$this->userSqlObj = NULL; // @swreq: UREQ:0000983:xSQL DEPRECATED

}



function searchPrepare(&$sql, $tablename, &$searchArr) {
	return f_search_ulib::searchPrepare($sql, $tablename, $searchArr);
}

function prSearchPrepMain( &$sql,  &$searchArr, $searchOp_i, $oldSearchInfo) {
	$search_lib = new f_search_ulib($this->tablename);
	return $search_lib->prSearchPrepMain($sql, $searchArr, $searchOp_i, $oldSearchInfo);
}

function searchQuick( &$sqlo,  $idx) {
    $search_lib = new f_search_ulib($this->tablename);
    $search_lib->quick_search($sqlo, $idx);
}

// check, if the FULL search form was used => switch to it ...
function  searchCheckFullFlag( $searchArr ) {
    $active_keys=array("alias", "cond", "column", "stext", "op");
    $l_found=0;
    foreach($active_keys as $key) {
        if ($searchArr[$key]!='') $l_found=1;
    }
    if ($l_found) {
        $sess_f_settings = $_SESSION['s_formState'][$this->tablename.'.view.search.f'];
        if (!is_array($sess_f_settings)) $sess_f_settings=array();
        $sess_f_settings['gui']='full';
        $_SESSION['s_formState'][$this->tablename.'.view.search.f'] = $sess_f_settings;
    }
}

/**
 * prepare column positions
 * @return $this->prColpos
 * @param object $sortcritX
 * @global $this->view_opt, $this->colNames_show, $this->primast
 */
function colPosPrep( ) {
		
	$primast       = &$this->primast;
	$colNames_show = &$this->colNames_show;
	
	$colpos = NULL; // array of positions
			  		// "extra_obj", ....
	$colpos["nameid"]  = -1; /* id of typical name */
	
	$cnt     = 0;
	$importantCol  = importantNameGet2($this->tablename);
	$primast = NULL;
	$primast[0]=-1;
		
	foreach( $colNames_show as $tmpColSh ) {
    
		$tmp_col_pure = $tmpColSh["cc"];
		$tmp_col_ext  = $tmpColSh["ce"];
		// $show_flag    = $tmpColSh["show"];
			
		if ($tmp_col_ext == "x.".$importantCol) { 
			$colpos["nameid"] = $cnt; /* get id of important col */
		}
		if ($tmp_col_ext == "x.EXTRA_OBJ_ID") { 
			$colpos["extra_obj"] = $cnt; /* get id of extra_obj col */
		}
		if ($tmp_col_ext == "x.CCT_ACCESS_ID") { 
			$colpos["CCT_ACCESS_ID"] = $cnt;
		}
		if ($tmp_col_ext == "a.H_ALOG_ACT_ID") { 
			$colpos["H_ALOG_ACT_ID"] = $cnt;
		}
		if ($tmp_col_ext == "x.".$this->cctgobacol) {
			$colpos["cctgobacol"] = $cnt;
		} 
		
		if (isset($this->primas[0]) and ($tmp_col_ext == 'x.'.$this->primas[0])) {
			$primast[0]=$cnt; /* repair position of FIRST PRIMARY key,*/
		}
		if (isset($this->primas[1]) and ($tmp_col_pure == $this->primas[1])) {
			$primast[1]=$cnt; /* repair position of 2. PRIMARY key,*/
		}
		if (isset($this->primas[2]) and ($tmp_col_pure == $this->primas[2])) {
			$primast[2]=$cnt; /* repair position of 3. PRIMARY key */
		}
		
		
		$cnt++;
	} 
	
	$this->primast = $primast;
	if (!$colpos["cctgobacol"]) $colpos["cctgobacol"] = $primast[0]; // column ID needed for print_row()
	
	$this->prColpos = $colpos;
	
}



/**
 * prepare $foreign_keys and $colNames_show
 * @return array($colNames_show, $foreign_keys)
 * @param object $selectCols
 */
function headRowPrep() {
	$tablename   = $this->tablename;
	$selectCols = &$this->selectCols;
	$colNames_show=array();
	$cnt=0;
	$foreign_keys=array();
	// take names from selectstring !!!
	
	foreach( $selectCols as $valarr ) {
	
		
	    $column_ext_name = key($valarr);
	    $column_code     = current($valarr);
	    reset($valarr);
	    
	    $show_flag       = $valarr["show"];
	    $niceCol	     = $valarr["nice"]; // nice name 
	    $app_type_id     = appDataTypeGet2($tablename, $column_code);
	    $app_type        = appDataTypeNameGet2($app_type_id);
	    $colNames_show[] = array ( "cc"=>$column_code, "ce"=>$column_ext_name,
		   "show" => $show_flag, "nice" => $niceCol, "app" => $app_type, 'tdAdd'=>$valarr['tdAdd'] );
	
	    $foreign_imp_name= $valarr["key"];
	    $foreign_table   = $valarr["tab"];
	    $foreign_pkey    = $valarr["fk_pname"];
	    $foreign_keys[$cnt][0]= $foreign_imp_name;
	    $foreign_keys[$cnt][1]= $foreign_table;
	    $foreign_keys[$cnt][2]= $foreign_pkey; 
	
	    $cnt++;
	}
	
	
	return array($colNames_show, $foreign_keys);
}



/**
 * - init GO_BACK variables
 * - set $this->roleCache
 * @param object $sqlo
 * @param array $formback
 * @global array $this->roleCache
 * 
 */
function prSetPar1( &$sqlo, $formback ) {

	$this->cctgoba    = $formback['fid'];
	$this->cctgobacol = $formback['xcol'];
	
	$this->roleCache['useRelease'] = 0;
	if ( $this->tablename=='CONCRETE_SUBST' ) {
		$role_right_name = "o.CONCRETE_SUBST.useRelease";
		$role_right      = role_check_f($sqlo, $role_right_name);
		if ( $role_right=="execute" ) {
			$this->roleCache['useRelease'] = 1;
		} else $this->roleCache['useRelease'] = 0;
		if ( $_SESSION['sec']['appuser']=='root' ) $this->roleCache['useRelease'] = 0;
	}
}

/**
 * set important parameters for print_row()
 * @param array $foreign_keys
 * @param array $colNames_show
 */
function prSetPar2( &$foreign_keys,  &$colNames_show ) {
	$this->primasNum  = sizeof($this->primas);
	$this->foreign_keys = &$foreign_keys;
	$this->colNames_show = &$colNames_show;
}


/**
 * - show the select ARROW
 * - for 'CONCRETE_SUBST' : analyse audit-status !
 * @param int $access_id can be NULL
 */
function _selShowOne( &$sqlo, $nameIn, $tmp_id, $access_id ) {
	
    $allow = 1;
    // may be later 
	//if ( $this->tablename=='CONCRETE_SUBST' ) {
	//	$allow = $this->_anaAuditState( $sqlo, $access_id );
	//} else $allow = 1;
	
	if ($allow) {
		$tmp_name=str_replace ("\\", "\\\\", $nameIn );
		$tmp_name=str_replace ("'", "\'", $tmp_name );
		$tmp_name= urlencode($tmp_name); // handle the PROCENT-string-problem
		echo "<a href=\"javascript:writeit('". $this->cctgoba."','" .$tmp_id. "','". $tmp_name . 
							"' );\" ><img src=images/leftsel.but.gif hspace=3 border=0></a>\n";
	} else {
		echo '<img src="images/but.selDeny.gif" border=0 hspace=3 title="no released-state">';
	}
}





/**
 * get $exp_raw_desc_id
 * - check, if this is an experiment result table (or view)
 * @return $EXP_RAW_DESC_ID
 * @param object $sqlo
 * @param object $mother_idM (ID of mother-object)
 */
function getDynaColMother(&$sqlo, $mother_idM, $exp_raw_desc_id=0) {
	$tablename = $this->tablename;
	
	$expResultTabs = array("EXP_RAW_RESULT", "SPOT_RESULT", "EXP_CHAN_RES", "EXP_SPOT_RESULT", "EXP_RAW_SPOT_RESULT");
	
	if ( in_array( $tablename, $expResultTabs ) ) {
		// get $EXP_RAW_DESC_ID
		$tmpexpid = $mother_idM;
		if (!$tmpexpid) $tmpexpid = historyc::last_bo_get("EXP");
		if (empty($exp_raw_desc_id) AND $tmpexpid) {
			$sqlo->query('SELECT h_exp_raw_desc_id FROM exp WHERE exp_id = '.
				$sqlo->addQuotes($tmpexpid));
			$sqlo->ReadRow();
			$exp_raw_desc_id = $sqlo->RowData[0]; 
		}
	} else {
		$exp_raw_desc_id = NULL;
	} 
	return ($exp_raw_desc_id);  
}

/**
 * show for EXTRA_OBJ => Atribute header
 * @return array $xobArr array('na'=>$theName, 'map'=>$theMap)
 * @param object $sql2
 * @global $this->c_attrib	
 */
function xobj_para_prep( &$sql2 ) {		
    
	$c_attrib = &$this->c_attrib;
	
	if ( !is_array($c_attrib) ) { // EXTRA_OBJ parameters ?
	    return;
	}
	
	if ( !sizeof($c_attrib) ) {
		return;
	}
	
	$lener=sizeof($c_attrib);
	$i=0; 
	$xobArr = array();
	
	while ($i<$lener) {
		$sqls="select name, nice_name, map_col from extra_attrib where extra_attrib_id=" .$c_attrib[$i][0] ;
		$sql2->query($sqls);
		$sql2->ReadRow();    
		$theName = $sql2->RowData[0];    
		$theMap  = $sql2->RowData[2];
		if ( $sql2->RowData[1]!="" ) $theName = $sql2->RowData[1];
		if ( $this->view_opt["colCoNa"]  ) $theName = "c:".$sql2->RowData[0];  // raw name for export
		
		$xobArr[] = array('na'=>$theName, 'map'=>$theMap);
		$i++;
	}
    return $xobArr;
}

/**
 * get extra global functions, defined by 
 * @param object $sqlo
 * @param string $colname
 * @param array $idarr
 * @return string void|string|unknown_type
 */
function xcol_funcs (&$sqlo, $colname, $idarr )
{
    $output = "";
  
    if ( $colname == 'proj_show' ) {
 
        $obj_id = $idarr[0];
        $tmplib = new oPROJpathC();
        $answer = $tmplib->searchProject ( $sqlo, $obj_id, $this->tablename );
        if ($answer[0]) {
            $tmp_proj_id = $answer[0];
            $output = $tmplib->getPathText($sqlo, $tmp_proj_id );
            if ($answer[1]>1) $output .= ' [and more folders]';
        }
    }
    if ( $colname == 'vario_show' ) {
        
        $obj_id = $idarr[0];
        $all = $this->VarioLib->getAllVals($sqlo, $obj_id);
        // reduce length
        $MAX_PER_VAL=200;
        $komma='';
        foreach($all as $key=>$val) {
            if (strlen($val)>$MAX_PER_VAL) $val=substr($val,0,$MAX_PER_VAL).'..';
            $output .= $komma . $key.':'.$val;
            $komma='; ';
        }
    }
    
    return ( $output );
}

/**
 * - create information about shown columns
 * - create main struct $this->selectCols
 * - allow root to see all columns except PASS_WORD
 * @param object $sql
 * @param string $tablename
 * @param string $colNames
 * @param int    $access_id_has 0,1
 * @param int    $class_tab_has 0,1
 * @param string $classname
 * @param int   $exp_raw_desc_id
 * @param int    $showselected show only $_SESSION['userGlob'] selected ones
 * @return array($this->selectCols, $useJoin)
 *    where $useJoin: "CCT_ACCESS," (with komma)
 * @global $this->selectCols
 */
function colinfoget( &$sql,  $tablename, $colNames, $access_id_has, $class_tab_has, $classname, $exp_raw_desc_id, $showselected) {
    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
    
    $cct_acc_cols=array(
        'a.CREA_DATE' => array( "a.CREA_DATE" => "CREA_DATE", "show"=>1, "nice"=>"creation date"),
        'a.MOD_DATE'  => array( "a.MOD_DATE" => "MOD_DATE", "show"=>1, "nice"=>"modified date",),
        'a.DB_USER_ID'=> array( "a.DB_USER_ID" => "DB_USER_ID", "show"=>1, "nice"=>"user",
            "key"=>"NICK", "tab"=>"DB_USER", "fk_pname" =>"DB_USER_ID"),
        'a.H_ALOG_ACT_ID' =>  array(
            "a.H_ALOG_ACT_ID" => "H_ALOG_ACT_ID",
            "show"	  =>1,
            "nice"	  =>"audit status",
            "key"     =>"NAME",
            "tab"     =>"H_ALOG_ACT",
            "fk_pname"=>"H_ALOG_ACT_ID",
            'tdAdd'   =>'class=importx',
        )
        
    );
    
    $ret_primary_name=NULL;
    $ret_imp_name = NULL;
	$this->rowSettings=NULL;
	
	$selectCols= array();
	$cnt       = 1;  // col counter 
	$useJoin_arr   = array();
	
	$user_prefs_exists = 0;
	if (!empty($this->_tab_cols_pref)) $user_prefs_exists = 1;
     
	// the main COLUMNS are the "x." columns ...
	foreach( $colNames as $colName ) {
	    
        $showcol      = 1;
        $x_col_code='x.'.$colName;

        if ($showselected && $user_prefs_exists) {
            $showcol      = 0;  // default
            if ( $this->_tab_cols_pref[$x_col_code]>0  ) 
                $showcol = 1;	
        }
        
	    $colInfos = colFeaturesGet($sql, $tablename, $colName, $exp_raw_desc_id); 
	    
	    if ( ($colInfos['VISIBLE']=="0") && ($_SESSION['sec']['appuser']!="root") ) {
	       $showcol = 0;  
	    }
		
		do {
		    if ($x_col_code=="x.PASS_WORD") {
	        	$showcol = 0; // never show
	        	break;
			}
	        
			if ( $x_col_code=="x.CCT_ACCESS_ID" && $access_id_has) {

			    if ($user_prefs_exists) {
    			    foreach( $cct_acc_cols as $cct_col => $sub_row ) {
    			        
    			        if ( $this->_tab_cols_pref[$cct_col]> 0  ) {
    			            $useJoin_arr['CCT_ACCESS']=1;
    			            $sub_row['s'] = $this->_tab_cols_pref_sort[$cct_col];
    			            $selectCols[] =  $sub_row;
    			            $cnt++;
    			        }
    			    } 
			    } else {
			        // default column
			        $cct_col='a.DB_USER_ID';
			        $useJoin_arr['CCT_ACCESS']=1;
			        $selectCols[] =  $cct_acc_cols[$cct_col];
			        $cnt++;
			    }
		  	    $showcol=0;
				break;
		    }
	
		    if (strstr($colName,"EXTRA_OBJ_ID") != '') {
		      if (!$class_tab_has)
			    $showcol=0; /* do not show if no extra_class */
		      else {
			    if ( $classname ) { /* classname was selected  */
			        $selectCols[] =  array('c.NAME' => 'CLASS_NAME', 'show' => $showcol, 'nice' => 'class',
			             's'=> $this->_tab_cols_pref_sort[$x_col_code]);	
			        $cnt++;
			        $useJoin_arr['EXTRA_CLASS']=1; // if one class name is selected, do a JOINED search 
			        
			        $showcol=0;
			    }
		      }
			  break;
		    }
		    
		} while (0);

	    // if ( $colName == "NAME" ) $cnttmp=1.0001; 		/* bring NAME column to front */

	    $colNicename = $colInfos['NICE_NAME'];
	    $linked_tab  = fk_check2($colName, $tablename, $ret_primary_name, $ret_imp_name); 
	    if ( $linked_tab ) {
	        $selectCols[] =  array( "x.".$colName => $colName , "show" =>  $showcol, "nice"=>$colNicename,  "key"=>$ret_imp_name,
		        "tab"=>$linked_tab, "fk_pname" => $ret_primary_name, 's'=> $this->_tab_cols_pref_sort[$x_col_code] );	
	    } else {
	        
	    	$fullQualiCol="x.".$colName;
	    	$tmpColFeat = array( $fullQualiCol => $colName, "show" =>  $showcol, "nice"=>$colNicename, 's'=> $this->_tab_cols_pref_sort[$x_col_code] );
	    	$columnType_app = appDataTypeNameGet2( $colInfos['APP_DATA_TYPE_ID'] );
	    	if ($columnType_app=='date') { // formatting to short date-format
	    		$tmpColFeat['for'] = $sql->Sql2DateString($fullQualiCol,5);
	    	}
	    	$selectCols[] =  $tmpColFeat;
	    }
	    $cnt++;
     }
	 
     $this->selectCols=array();
     $useJoin = NULL;
     $useJoin = implode(',',array_keys($useJoin_arr));
     
     if ( $_SESSION["userGlob"]["g.debugLevel"]>0 ) {
       
         debugOut('(761) selectCols:'.print_r($selectCols,1), $FUNCNAME,1);
         debugOut('(764) useJoin:'.$useJoin.' useJoin_arr:'.print_r($useJoin_arr,1), $FUNCNAME,1);

     }
     
     // RESORT by user preferences ...
     if ($user_prefs_exists) {
         
         $sort_arr=array();
         foreach($selectCols as $ind=>&$row) {
             $sort_arr[$ind]=$row['s'];
         }
         asort($sort_arr);
         
         debugOut('(775) sort_arr:'.print_r($sort_arr,1), $FUNCNAME,1);
         
         foreach( $sort_arr as $ind => $sortval ) {
             $this->selectCols[] = $selectCols[$ind];
         }
         
     } else {
         $this->selectCols = $selectCols;
     }

     
	 
     return array($this->selectCols, $useJoin);
}




/**
 *  get columns of sort from SQL-text
 *  set $this->sortcritX
 *  @param $sortcritSQL string like: x.NAME ASC, a.DB_USER_ID DESC, ...
 *  @return  array $sortCondArr [COLNAME] = array (
 *		["dir"] = ["DESC"], "ASC";   //  DEFAULT
 *		["pos"] = 1,2,3
 */
function getSortMatrix( $sortcritSQL ) {

	$this->sortcritX = NULL;
	if ( $sortcritSQL == "" ) {
		return;
	}
	
	$orderSplitarr = explode( "," , $sortcritSQL ); 
	
	$pos = 1;
	$sortCondArr=array();
	foreach( $orderSplitarr as $orderOne ) {
		
		$ColDirectArr = explode(" ", $orderOne ); 
		$colname   = $ColDirectArr[0];
		$direction = $ColDirectArr[1];
		$sortCondArr[$colname] = array("dir"=>$direction, "pos"=>$pos);
		 
		$pos++;
	}
	
	$this->sortcritX = $sortCondArr;
	
	return ($sortCondArr);
}




function getRoleCache() {
	return ($this->roleCache);
}

}
