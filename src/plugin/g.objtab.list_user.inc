<?php
/**
 * show/configure a user defined LIST of objects
 *
 * @package g.objtab.list_user.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param $_REQUEST['id'] ID of config document
 * @param $action
 *    ['show']  : show view
 *    'forward_l': release the query AND forward to default list view 
 *    'doc_crea' : create new doc
 *    'conf_t' : configure table
 *    'conf_col' : configure columns
 *    'conf_filt' : configure filter
 *    'conf_sort' : sort
 *    'conf_misc' : misc
 * @param int $reset_col
 * @param $viewcol
 * @param $parx
 * @param $parx2
 *   action
 */


require_once("sql_query_dyn.inc");
require_once("f.sql_query.inc");
require_once("db_x_obj.inc");

require_once 'o.CCT_TABLE.info.inc';
require_once ("insertx.inc");
require_once 'o.PROJ.addelems.inc';


require_once ("gui/glob.objtab.pref.inc");
require_once  'gui/glob.objtab.columns.inc';
require_once  'gui/glob.objtab.filter_GUI.inc';
require_once ("gui/g.objtab.list_user_lib.inc");
require_once 'o.LINK.list_user.inc';



class g_objtab_list_user_XPL extends gPlugin {
	
	function register() {
	    
	    $tablename=NULL;
	    
	    $doc_name=' to be defined ...';
	    $docid =  $_REQUEST['id'];
	    if ($docid) { 
	        try {
    	        $this->main_lib = new g_objtab_list_user_lib($this->_sqlo, $docid);
    	        $json_arr = $this->main_lib->get_congif();
    	        if (!empty($json_arr)) {
    	            $tablename = $json_arr['t'];
	            }
	        } catch (Exception $e) {
	            // ignore error ...
	            $this->main_lib=NULL;
	        }
	        $doc_name = obj_nice_name ( $this->_sqlo, 'LINK', $docid );
	    }
	    
	    $title_sh   = g_objtab_list_user_lib::TOOL_NAME.': '.$doc_name. ' (Config-ID:'.$docid.')';
	    $title		= $title_sh;
		$this->infoarr			   = NULL;
		$this->infoarr['title_sh']    = $title_sh;
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'tool'; 
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['locrow'] = array ( 
		    array ('home.php', 'home') ,
		   
		);
		
		if ($tablename!=NULL) {
    		$this->infoarr2 = array( 
    			'accCheck' => array('tab'=>array('read') ),
    		);
		}
	}
	
	function config_save($sqlo) {

	    $this->main_lib->config_save($sqlo);
	    
	    $this->_html->line_out('');
	    $this->_html->messageBox2('ok', 'Saved.');
	    $this->_html->line_out('');
	}
	
	// define DOC + TABLE
	function form1_creadoc($sqlo) {
	    
	    
	    $tab_arr = oCCT_TABLE_info::getTablesNiceByType($sqlo, 'BO');
	        
        $initarr   = array();
        $initarr["title"]       = "Create a new Config"; // title of form
        $initarr["submittitle"] = "Create"; // title of submit button
        $initarr["tabwidth"]    = "AUTO";   // table-width: AUTO
        
        $hiddenarr = NULL; // hidden form variables
        $hiddenarr["action"]     = 'doc_crea' ;
        
        // init form, uses class formc
        $formobj = $this->_html->formInit($initarr, $hiddenarr, 0);
        
        $fieldx = array ( // form-field definition
            "title"   => "Config name",
            "name"  => "name",
            "object"=> "text",
            "val"    => '',
            "notes" => "",
            'req'   => TRUE
        );
        $formobj->fieldOut( $fieldx ); // output the form-field
        
        $fieldx = array ( // form-field definition
            "title"   => "table",
            "name"  => "table",
            "object"=> "select",
            //"val"    => '',
            'inits'=> $tab_arr,
            "notes" => "",
            'req'   => TRUE
        );
        $formobj->fieldOut( $fieldx ); // output the form-field
        
        $formobj->close( TRUE ); // close form, sshow submit-button
	    
	}
	
	
	
	function creadoc_do($sqlo, $parx) {
	    global $error, $varcol;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    $varcol = & Varcols::get();
	    
	    $parx['name'] = trim($parx['name']);
	    if ($parx['name']==NULL) {
	        $error->set( $FUNCNAME, 10, 'Input NAME missing.' );
	        return;
	    }
	    if ($parx['table']==NULL) {
	        $error->set( $FUNCNAME, 20, 'Input table missing.' );
	        return;
	    }
	    
	    $doc_table='LINK';

	    $class_name = oLINK_list_user::DOC_CLASS;
	    $extra_class_id	= $varcol->class_name_to_id( $doc_table, $class_name );
	    if ($error->Got(READONLY))  {
	        $error->set( $FUNCNAME, 30, 'Class '.$class_name.' not found.' );
	        return;
	    }
	    
	   
	    $insertlib = new insertC();
	    $args= array( 
	        "vals"=>array(
	            'NAME'=>$parx['name']
	        ),
	        "xobj"=>array(
	            'extra_class_id'=> $extra_class_id
	        ) 
	        
	    ) ;
	    $doc_id = $insertlib->new_meta($sqlo, $doc_table, $args);
	    if ($error->Got(READONLY))  {
	        $error->set( $FUNCNAME, 30, 'Config-DOC not created.' );
	        return;
	    }
	    
	    $projid = oLINK_list_user::get_STD_folder($sqlo);
	    $projLib = new oProjAddElem( $sqlo, $projid );
	    $projLib->addObj( $sqlo, $doc_table, $doc_id); 
	    
	    $this->main_lib = new g_objtab_list_user_lib($this->_sqlo, $doc_id);
	    $this->main_lib->config_set_val('t', $parx['table']);
	    $this->config_save($sqlo);
	    
	    return $doc_id;
	}
	
	/**
	 * TBD: ..
	 * use CLASS from glob.objtab.pref.php
	 */
	function form2_conf_col($sqlo, $go, $user_input) { 
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    $json_arr = $this->main_lib->get_congif();
	    $tab_cols_pref = $json_arr['cols'];
	    $tablename = $this->main_lib->tablename;
	    
	    debugOut('(322) tab_cols_pref:'.print_r($tab_cols_pref,1), $FUNCNAME, 1);
	    if (!$go and empty($tab_cols_pref)) {
	        // set init cols
	        $tab_col_lib = new gObjtab_column($tablename);
	        $tab_cols_pref = $tab_col_lib->get_start_columns($sqlo);
	    }
	    
	    debugOut('(328) tab_cols_pref:'.print_r($tab_cols_pref,1), $FUNCNAME, 1);
	    
	    $tablename = $this->main_lib->tablename;
	    
	    $column_conf_lib = new glob_objtab_pref($tablename);
	    $column_conf_lib->set_column_config($tab_cols_pref);
	    
	    if ( $go>0 ) {
	        $column_conf_lib->column_inp2config( $sqlo, $user_input);
	        $tab_cols_pref = $column_conf_lib->column_config_get();
	        
	        debugOut('user_input: '.print_r($user_input,1), $FUNCNAME, 1);
	        debugOut('tab_cols_pref2: '.print_r($tab_cols_pref,1), $FUNCNAME, 1);
	        
	        $this->main_lib->config_set_val('cols', $tab_cols_pref);
	        $this->config_save($sqlo);
	        
	        // reinit
	        $column_conf_lib = new glob_objtab_pref($tablename);
	        $column_conf_lib->set_column_config($tab_cols_pref);
	    }
	    if ($_REQUEST['reset_col']) {
	        $column_conf_lib->col_prefs_reset();
	    }
	    
	    // echo "GO: $go<br>";
	    
	    echo "<br>";
	    
	    echo '<form method="post" action="'.$this->url_normal.'&go=1">'."\n";
	    echo '<input type="hidden" name="action" value="conf_col">';
	    
	    $column_conf_lib->table_start();

	    $xcopt = array("text"=>'[<a href="'. $this->url_normal.'&reset_col=1">Reset to standard</a>]' );
	    $column_conf_lib->showInfoRow( "Shown columns in list view:", $xcopt);
	    $column_conf_lib->sh_DBColumns($sqlo);
	    $xcopt = array("bgcolor"=>"#FFFFFF" );
	    $column_conf_lib->showInfoRow( "&nbsp;", $xcopt); 
	    
	    $column_conf_lib->table_close();
	    

	    echo "</form>\n";
	    echo "<br><br>\n";
	    $column_conf_lib->JS_out();
	}
	
	
	
	/**
	 * 
	 * @param string $col_code
	 * @param array $col_rows
	 * @return string
	 */
	private function column_nice(string $col_code, array &$col_rows) {
	    $nice='';
	    foreach($col_rows as $col_row) {
	        if ( $col_code==$col_row['c'] ) {
	            $nice = $col_row['na'];
	            break;
	        }
	    }
	    //echo "XXXCODE:: $col_code nice:$nice<br>";
	    //echo "XXXCODE:: ".print_r($col_rows,1)."<br>";
	    return $nice;
	}

	/**
	 * 
	 * @param object $sqlo
	 * @param int $go
	 * @param array $user_input
	 *   'col'
	 *   'dir'
	 */
	function form2_conf_sort($sqlo, $go, $user_input) {
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    $tablename  = $this->main_lib->tablename;
	    $json_arr   = $this->main_lib->get_congif();
	    $col_arr  = $json_arr['cols'];
	    $sort_all_arr = $json_arr['sort'];
	    
	    $tab_col_lib = new gObjtab_column($tablename);
	    $col_rows = $tab_col_lib->get_all_user_columns($sqlo);
	    
	    if ($go) {
	        
	        debugOut('user_input:'.print_r($user_input,1), $FUNCNAME, 1); 
	        $sort_arr= array(array('col'=>$user_input['col'], 'dir'=>$user_input['dir']));
	        debugOut('SORT:'.print_r($sort_arr,1), $FUNCNAME, 1);
	        $this->main_lib->config_set_val('sort', $sort_arr);
	        $this->config_save($sqlo);
	        
	        return;
	    }
	    
	    echo '<form method="post" action="'.$this->url_normal.'&go=1">'."\n";
	    echo '<input type="hidden" name="action" value="conf_sort">';
	    
	    $sort_col=$sort_all_arr[0]['col'];
	    $sort_dir=$sort_all_arr[0]['dir'];
	    
	    $sel_arr=array();
	    foreach($col_arr as $col=>$flag) {
	        $col_nice = $this->column_nice($col, $col_rows);
	        $sel_arr[$col]=$col_nice;
	    }
	    
	    $sort_array=array('DESC'=>'DESC', 'ASC'=>'ASC');
	    
	    echo "<table>\n";
	    echo '<tr>'."\n";
	    echo "<td>";
	    echo formc::selectFget('parx[col]',  $sel_arr,    $sort_col);
	    echo formc::selectFget('parx[dir]', $sort_array, $sort_dir);
	    echo "</td>";
	    echo '</tr>'."\n";
	    echo "</table>\n";
	    echo '<input type=submit value="Save" class="yButton">';
	    echo '<form>';
	}
	
	/**
	 * MISC settings
	 * @param object $sqlo
	 * @param int $go
	 * @param array $user_input
	 *   'showsets'
	 *   
	 */
	function form4_conf_misc($sqlo, $go, $user_input) {
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    // $tablename  = $this->main_lib->tablename;
	    
	    $json_arr   = $this->main_lib->get_congif();
	    $conf_options  = $json_arr['opt'];
	    
	    if ($go) {
	        
	        debugOut('user_input:'.print_r($user_input,1), $FUNCNAME, 1);
	        
	        if ($user_input['showsets'] > g_objtab_list_user_lib::MAX_SHOWSET) {
	            $user_input['showsets'] =  g_objtab_list_user_lib::MAX_SHOWSET;
	        }
	        
	       
	        debugOut('SORT:'.print_r($user_input,1), $FUNCNAME, 1);
	        $this->main_lib->config_set_val('opt', $user_input);
	        $this->config_save($sqlo);
	        
	        return;
	    }
	    
	    
	    
	    $initarr   = array();
	    $initarr["title"]       = "Misc options"; // title of form
	    $initarr["submittitle"] = "Submit"; // title of submit button
	    $initarr["tabwidth"]    = "AUTO";   // table-width: AUTO
	    
	    $hiddenarr = NULL; 
	    $hiddenarr["id"]     =  $this->doc_id;
	    $hiddenarr["action"] = 'conf_misc'; 
	    
	    
	    $formobj = $this->_html->formInit($initarr, $hiddenarr, 0);
	    
	    $fieldx = array ( 
	        "title"   => "Number of SHOWN objects",
	        "name"  => "showsets",
	        "object"=> "text",
	        "val"    => $conf_options['showsets'],
	        "notes" => "typical: 20"
	    );
	    $formobj->fieldOut( $fieldx ); 
	    
	    $fieldx = array (
	        "title"   => "Show Reference IDs",
	        "name"  => "listShowFK",
	        "object"=> "checkbox",
	        "val"    => $conf_options['listShowFK'],
	        "notes" => "Shows IDs of referenced elements in list view"
	    );
	    $formobj->fieldOut( $fieldx ); 
	    
	    $fieldx = array (
	        "title"   => "Shown characters per cell",
	        "name"  => "CelSize",
	        "object"=> "text",
	        "val"    => $conf_options['CelSize'],
	        "notes" => "Number of characters of cells (default: max 30 characters per cell)"
	    );
	    $formobj->fieldOut( $fieldx );
	    

	    $formobj->close( TRUE ); // close form, sshow submit-button
	 
	   
	}
	
	function startMain() {
        global $error;
        $FUNCNAME='MAIN';
        
        $sqlo   = &$this->_sqlo;
        $sqlo2 = &$this->_plug_regSql2();
        
        $docid  =  $_REQUEST['id'];
        $action =  $_REQUEST['action'];
        $go     =  $_REQUEST['go'];
        $parx   =  $_REQUEST['parx'];
        
        $this->doc_id = $docid;
        
        if ($this->main_lib==NULL and $docid) {
            // provoke the previous error ...
            $this->main_lib = new g_objtab_list_user_lib($this->_sqlo, $docid);
            $json_arr = $this->main_lib->get_congif();
            if (!empty($json_arr)) {
                $tablename = $json_arr['t'];
            }
        }
        
        debugOut("ACTION: $action", $FUNCNAME, 1);

   
        if (!$docid) {
            if (!$go) {
                $this->form1_creadoc($sqlo);
                return;
            } 
            $docid = $this->creadoc_do($sqlo, $parx);
            if ($error->Got(READONLY))  {
                $error->set( $FUNCNAME, 10, 'Error on config create.' );
                return;
            }
            
            $action = 'conf_col';
        } 
        
        $o_rights = access_check($sqlo,'LINK', $docid);
        $doc_can_edit=0;
        if ( $o_rights["write"]) {
            $doc_can_edit=1;
        }
        
        
        /**
         * 'conf_t' : configure table
         *    'conf_col' : configure columns
         *    'conf_filt'
         * @var Ambiguous $header
         */
        $this->url_normal=$this->_plug_get_URL().'&id='.$docid;
        $spacer='&nbsp&nbsp;&nbsp;';
        if ($doc_can_edit ) {
            $header = ''.
                '<a href="'.$this->url_normal.'&action=conf_col"><img src="res/img/align-justify.svg" title="Shown columns"> Columns</a> '.$spacer.
                ' <a href="'.$this->url_normal.'&action=conf_filt"><img src="res/img/filter.svg" title="Filter settings"> Filter</a> '.$spacer.
                ' <a href="'.$this->url_normal.'&action=conf_sort"><img src="res/img/arrow-up.svg" title="Sort"><img src="res/img/arrow-down.svg" title="Sort"> Sorting</a> '.$spacer.
                ' <a href="'.$this->url_normal.'&action=conf_misc"><img src="res/img/settings.svg" title="Other"> Misc</a> '.$spacer;
        } else {
            echo '<span class="yGgray">Config changes: you have no write-permission.</span>';
        }
        
        $header .=' <a href="'.$this->url_normal.'&action=show"><img src="res/img/play-circle.svg" title="RUN!"> RUN</a>';
 
        
        $obj_link = fObjViewC::	bo_display( $sqlo, 'LINK', $docid);
        $header .= ' &nbsp&nbsp;&nbsp; Config-DOC: '.$obj_link;
        $this->_html->line_out($header);
        $this->_html->line_out('');
        
        // check 
        $tablename=NULL;
        $json_arr = $this->main_lib->get_congif();
        if (!empty($json_arr)) {
            $tablename = $json_arr['t'];
        }
        if ($tablename==NULL) {
            $this->htmlFoot('ERROR', 'No table defined.');
        }
        
        $this->main_lib->init_table($sqlo);
        
        
        
        switch ($action) {
            case 'conf_col':
                $this->form2_conf_col($sqlo, $go, $_REQUEST['viewcol']); 
                break;
            case 'conf_filt': 
                
                $filt_gui_lib = new g_objtab_filter_GUI($tablename, $json_arr, $this->url_normal);
                $filter_arr=$json_arr['filt'];
                if(empty($filter_arr)) $filter_arr=array();
                
                $parx2  = $_REQUEST['parx2'];
                if (is_array($parx2)) {
                    
                    switch ($parx2['action']) {
                        case 'add_col':   
                            if (!$go) {
                                $new_i = sizeof($filter_arr);
                                $filt_gui_lib->form2_1_column($sqlo, $new_i);
                                return;
                            } else {
                                $go=0;
                                $filt_gui_lib->add_col($parx2['col']);
                                // continuem but do not SAVE !
                            }
                            break;
                            
                        case 'del_col':   
                            $filt_gui_lib->del_col($parx2['col_i']);
                            $filter_arr = $filt_gui_lib->get_filter_arr();
                            $this->main_lib->config_set_val('filt', $filter_arr);
                            $this->main_lib->config_save($sqlo);
                            break;
                    }
                }
                
                if (!$go) {
                    $filt_gui_lib->form_main($sqlo, $_REQUEST['parx']);
                } else {
                    $filt_gui_lib->save_filter($sqlo, $_REQUEST['parx']);
                    $controlled_input = $filt_gui_lib->get_filter_arr();
                    $this->main_lib->config_set_val('filt', $controlled_input);
                    $this->config_save($sqlo);
                }
                break;
            case 'conf_sort':
                $this->form2_conf_sort($sqlo, $go, $_REQUEST['parx']);
                break;
            case 'conf_misc':
                $this->form4_conf_misc($sqlo, $go, $_REQUEST['parx']);
                break;
                
            case 'forward_l':
                $this->main_lib->release($sqlo);
                if ($error->Got(READONLY))  {
                    return;
                }
                $url='view.tmpl.php?t='.$tablename;
                $this->_html->pageForward($url, 'default list view');
                break;
            case 'show':
                $this->main_lib->show($sqlo, $sqlo2);
                break;
            default:
                echo '<ul><h3>Config home</h3></ul>';
        }

	}
	
}
