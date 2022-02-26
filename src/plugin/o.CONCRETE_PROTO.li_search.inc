<?php
/**
 * search protocols by parameters
 * GLOBAL:  $_SESSION["s_formState"]['o.CONCRETE_PROTO.li_search'] li_search_conf_STRUCT
 *   'pra_ids' : array
 *   'steps: array 
 *       [STEP_NR][TYPE]=array(
 *         'c': condition
 *         'v': value
 *      )
 * @package o.CONCRETE_PROTO.li_search.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param $_REQUEST['go'] 
 *    0, 1
 * @param $action
 *   'sel_pra'
 *   'sel_steps'  : select steps
 *   'sel_params' : define search parameters
 *   'run'
 * @param $_REQUEST['parx']
 *   'pra_ids': array of pra_ids
 *   'pra_id' : single PRA_ID
 * @param $_REQUEST['x']
 *   selected steps
 *   [stepnr][$type][v] = val
 */
require_once ("glob.obj.update.inc");
require_once ("javascript.inc");

require_once 'f.sql_query.inc';
require_once 'func_form.inc';
require_once 'o.ABSTRACT_PROTO.stepx.inc';
require_once 'o.EXP.proto.inc';
require_once 'o.EXP_TMPL.proto.inc';
require_once 'o.AS_HAS_PR.subs.inc';
require_once 'o.CS_HAS_PR.subs.inc';
require_once 'gui/o.CONCRETE_PROTO.li_show1.inc';

class o_ABSTRACT_PROTO_qs {
    
    private $pra_all_steps; 
    public  $settings;   // li_search_conf_STRUCT
    
    /**
     * array 
     *   'st'=>, 'name'=>name, 'ty'=> type
     **/
    private $steps_ser;
    
    const types_map   = array(
        's'=>array('col'=>'CONCRETE_SUBST_ID', 'form_type'=>'dblink', 'nice'=>'material', 'c'=>array('=', 'is NULL', '!=') ), 
        'q'=>array('col'=>'QUANTITY', 'form_type'=>'text', 'nice'=>'quantity', 'c'=>array('=', '>=', '<=', '!=', 'is NULL' ) ),
        'n'=>array('col'=>'NOTES',    'form_type'=>'text', 'nice'=>'notes', 'c'=>array('like', '=', 'is NULL' ) ), 
        'd'=>array('col'=>'DEV_ID',   'form_type'=>'dblink', 'nice'=>'device', 'c'=>array('=', 'is NULL', '!=') ), 
        
    );
    
    
    function __construct($sqlo, $pra_id, $mod_id) {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        $this->apid = $pra_id;
        
        $this->settings = $_SESSION["s_formState"]['o.CONCRETE_PROTO.li_search'];
        
        if ($pra_id) {
            $this->settings['pra_ids'] = array($pra_id);
            $this->save_settings();
        }
        
        $pra_id = current($this->settings['pra_ids']);
        if (!$pra_id) {
            throw new Exception('Input PRA-ID missing.');
        }
        
        $this->_mod = $mod_id;
        

        $pra_lib = new oABSTRACT_PROTO_stepx($pra_id);
        
        $pra_steps= $pra_lib->get_steps($sqlo );
        $pra_lib->build_index($sqlo);
        
        $this->pra_all_steps=array();
        foreach($pra_steps as $step_nr) {
            $step_feats = $pra_lib->step_info( $sqlo, $step_nr );
            $this->pra_all_steps[$step_nr] = $step_feats;
        }
        
        
        $this->set_steps_ser();
        
        if ( $_SESSION["userGlob"]["g.debugLevel"]>0 ) {        
            debugOut(' settings:'.print_r( $this->settings,1), $FUNCNAME, 1);
        }
        
        $this->numDetails = 4;
        
    }
    
    function get_pra_id() {
        $pra_id=0;
        if (!empty($this->settings['pra_ids'])) {
            $pra_id = current($this->settings['pra_ids']);
        }
        return $pra_id;
    }
    
    private function set_steps_ser() {
        $this->steps_ser = array();
        
        if (!empty($this->settings['steps'])) {
            $this->steps_ser = array();
            
            foreach($this->settings['steps'] as $step_nr => $row) {
                foreach($row as $type=>$type_info) {
                    $out = array('st'=>$step_nr, 'ty'=>$type);
                    $this->steps_ser[] = $out;
                }
            }
            
        }
    }
    
    function _checkboxOut( $key, $stepnr ){
        $ischecked = "";
        
        $user_def =  $this->settings["steps"][$stepnr][$key];
        
        if ( is_array($user_def) ) $ischecked = " checked";
        return ( "<input type=checkbox name=x[".$stepnr."][".$key."] value=1".$ischecked.">" );
    }
    
    private function _sel_step_one( &$sqlo, $oneStep ) {
        
        $outarr = array();
        
        $stepnr  = $oneStep["STEP_NR"];
        $sua_name=''; 
        if ($oneStep["ABSTRACT_SUBST_ID"]>0) {
            $sua_name = obj_nice_name ( $sqlo, "ABSTRACT_SUBST", $oneStep["ABSTRACT_SUBST_ID"] );
        }
        
        $outarr[] = $oneStep["index"];
        $outarr[] = $oneStep["NAME"];
        $outarr[] = $sua_name;
        
        foreach(self::types_map as $key=>$key_info) {
            $outarr[] = $this->_checkboxOut( $key, $stepnr );
        }

        
        return ($outarr);
    }
    
    /**
     * 
     * 
            "Step-name",
            "Type",
            "Condition",
            "Value",
     * @param array $row
     * @return array
     */
    private function _form_sel_params_one($row) {
        
        $stepnr  = $row["st"];
        
        $setx     = &$this->settings['steps'];
        $pra_step = &$this->pra_all_steps[$stepnr];
        
        $type    = $row["ty"];
        $type_defs = self::types_map[$type];
        $typ_param = $setx[$stepnr][$type];
        
        // echo "DDD: $stepnr:$type:".print_r($typ_param,1)." <br>";
        
        $outarr = array();
  
        $type_conditions = $type_defs['c'];
        $sel_field_in=array();
        foreach($type_conditions as $cond_loop) {
            $sel_field_in[$cond_loop] = $cond_loop;
        }
        $select_str = formc::selectFget( 'x['.$stepnr.']['.$type.'][c]', $sel_field_in, $typ_param['c']);
            
            
        $outarr[] = $pra_step["index"];
        $outarr[] = $pra_step["NAME"];
        $outarr[] = $type;

        $outarr[] = $select_str;
        $outarr[] = '<input type=text size=7 name="x['.$stepnr.']['.$type.'][v]" value="'.$typ_param['v'].'">';
        
        
        return ($outarr);
    }
    
    function form_sel_steps( &$sqlo, $fo_opt=array() ) {

        
        echo "<form style=\"display:inline;\" method=\"post\"  name=\"editform\"  action=\"".$_SERVER['PHP_SELF']."\" >\n";
        echo '<input type=hidden name="mod" value="'.$this->_mod.'">'."\n";
        
        $tabobj = new visufuncs();
        $headOpt = array( "title" => "Select shown step details" );
        $headx  = array (
            "Index",
            "Step-name",
            "Planned Material<br>template",
            "Material",
            "Quantity",
            "Notes",
            "Device",    
        );
        $tabobj->table_head($headx,   $headOpt);

        foreach($this->pra_all_steps as $step_nr => $oneStep) {
           
            $dataArr = $this->_sel_step_one($sqlo, $oneStep);
            $tabobj->table_row ($dataArr);
        }
        
        $dataArr=NULL;
        $dataArr[]="";
        $dataArr[]="";
       
        
        $dataArr = array("<br><input type=submit class=\"yButton\" value=\"Submit\"><br><br>" );
        $copt = array("colspan"=>sizeof($headx), "bgcolor"=>"#E0E0FF", "other"=>"align=center");
        $tabobj->table_row ($dataArr, $copt);
        
        $tabobj->table_close();
        
        
        echo "<input type=hidden name='action' value='sel_steps'>\n";
        echo "<input type=hidden name='go' value='1'>\n";
        
        echo "</form>";
    }
    
    function form_sel_params( &$sqlo, $fo_opt=array() ) {
        
        // $apid = $this->apid;
        $this->set_steps_ser();
        
        $col_offset = 2; // Step-name + Material<br>template
        
        echo "<form style=\"display:inline;\" method=\"post\"  name=\"editform\"  action=\"".$_SERVER['PHP_SELF']."\" >\n";
        echo '<input type=hidden name="mod" value="'.$this->_mod.'">'."\n";
        
        $tabobj = new visufuncs();
        $headOpt = array( "title" => "Select search conditions" );
        $headx  = array (
            "Index",
            "Step-name",
            "Type",
            "Condition",
            "Value",
        );
        $tabobj->table_head($headx,   $headOpt);
        
        foreach($this->steps_ser as $row) {
            
            $dataArr = $this->_form_sel_params_one($row);
            $tabobj->table_row ($dataArr);
        }
        
        $dataArr=NULL;
        $dataArr[]="";
        $dataArr[]="";
        
        
        $dataArr = array("<input type=submit class=\"yButton\" value=\"Submit\">" );
        $copt = array("colspan"=> sizeof($headx), "bgcolor"=>"#E0E0FF", "other"=>"align=center");
        $tabobj->table_row ($dataArr, $copt);
        
        $tabobj->table_close();
        
        
        echo "<input type=hidden name='action' value='sel_params'>\n";
        echo "<input type=hidden name='go' value='1'>\n";
        
        echo "</form>";
    }
    
    function sel_steps_save($x) {
        
        $steps = array();
        
        foreach($x as $step_nr => $type_arr) {
            
            foreach($type_arr as $type => $val) {
                $old_ty_info = $this->settings['steps'][$step_nr][$type];
                if (!$val) continue;
                if (!empty($old_ty_info)) {
                    $steps[$step_nr][$type] = $old_ty_info;
                } else {
                    $steps[$step_nr][$type] = array();
                }
            }
            
        }
        
        $this->settings['steps'] = $steps;
        $this->set_steps_ser();  // recalculate ...
        $this->save_settings();
    }
    
    function sel_params_save(&$x) {
        
        foreach($x as $step_nr => $type_arr) {
            foreach($type_arr as $type => $infarr) {
                $this->settings['steps'][$step_nr][$type] = $infarr;
            }
        }
        $this->set_steps_ser();  // recalculate ...
        $this->save_settings();
    }
    
    function save_settings() {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        debugOut('settings: '.print_r($this->settings,1), $FUNCNAME, 2);
        
        $_SESSION["s_formState"]['o.CONCRETE_PROTO.li_search'] = $this->settings;
    }
}

class o_CONCRETE_PROTO_li_search_XPL extends gPlugin {

	
	function register() {
		$tablename	= 'CONCRETE_PROTO';
		$this->title		= 'search protocol parameters';
		
		$this->infoarr		= NULL;
	
		$this->infoarr['title']    = 'search real protocols by step parameters';
		$this->infoarr['title_sh'] = $this->title;
		$this->infoarr['form_type']= 'list'; 
		
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['obj_cnt']  = 1;

		$this->infoarr2 = array( 
			'accCheck' => array('tab'=>array('read', 'write'), 'obj'=>array() ),
			// 'listChkSelect' =>1
			);
				
		
	}
	
	
	
	
	function form1($pra_id) {
		
		
		
		$initarr   = NULL;
		$initarr["title"]       = "Select a ".tablename_nice2('ABSTRACT_PROTO');
		$initarr["submittitle"] = "Select";
		$initarr["tabwidth"]    = "AUTO";
		$initarr["dblink"] = 1;
	
		$hiddenarr = NULL;
	
		$formobj = $this->_html->formInit($initarr, $hiddenarr);
		
		$fieldx = array ( 
		    "title" => tablename_nice2('ABSTRACT_PROTO'), 
			"name"  => "pra_id",
			"object"=> "dblink",
		    "val"   =>  $pra_id, 
		    "inits" => array( "table"=>'ABSTRACT_PROTO',  "pos" =>"0", "projlink"=> 1),
			"notes" => ""
			 );
		$formobj->fieldOut( $fieldx );
		
		$formobj->close( TRUE );
	}
	
	
	function run_query($sqlo, &$settings) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    $pra_id = current($settings['pra_ids']);
        $steps_ser = array();
        
        foreach($settings['steps'] as $step_nr => $row) {
            foreach($row as $type=>$type_info) {
                $out = array('st'=>$step_nr, 'ty'=>$type);
                $steps_ser[] = $out;
            }
        }
	        
        $cond_arr=array();
        foreach($steps_ser as $row) {
            
            $step_nr = $row['st'];
            $type = $row['ty'];
            $ty_info = $settings['steps'][$step_nr][$type];
            $cond = $ty_info['c'];
            $val  = $ty_info['v'];
            $val = trim($val);
            
            $key_info= o_ABSTRACT_PROTO_qs::types_map[$type];
            $colname  = $key_info['col'];
            $val_sql  = $val;
            $use_condition = 1;
            
            switch ($type) {
                
                case 'q':
                    
                    if ($val==='') {
                        $use_condition = 0;
                        break;
                    }
                    if (!is_numeric($val)) {
                        $error->set( $FUNCNAME, 1, 'Quantity "'.$val.'" must be numeric.' );
                        return;
                    }
                    
                    
                    break;
                
                case 's':
                    
                    if ($val==='') {
                        $use_condition = 0;
                        break;
                    }
                    
                    if (!is_numeric($val)) {
                        $error->set( $FUNCNAME, 1, 'Material-ID "'.$val.'" must be numeric.' );
                        return;
                    }
                    break;
                    
                case 'd':

                    if ($val==='') {  
                        $use_condition = 0;
                        break;
                    }
                    
                    if (!is_numeric($val)) {
                        $error->set( $FUNCNAME, 1, 'Device-ID "'.$val.'" must be numeric.' );
                        return;
                    }    
                    break;
                    
                case 'n':
                    if ($val==='') {
                        $use_condition = 0;
                        break;
                    }
                    $val_sql  = $sqlo->addQuotes($val);
                    break;
            }
            
            if (!$use_condition) continue;
            if ($cond==NULL)     continue;
            
            
            if ($cond=='like') $cond = ' '.$cond.' ';
            if ($cond=='is NULL') {
               
                $cond = ' '.$cond;
                $val_sql = NULL;
            }
            
            $sql_cond = 'CONCRETE_PROTO_ID in ('.
                'select CONCRETE_PROTO_ID from CONCRETE_PROTO_STEP where ABSTRACT_PROTO_ID='.$pra_id.' and STEP_NR='.$step_nr.' and '.$colname . $cond . $val_sql .')';
           
            
            // echo "DDDX: $sql_cond <br>";
            
            $cond_arr[] = $sql_cond;
        }
        
        if (empty($cond_arr)) {
            $this->_html->messageBox2('warning', 'No conditions given.');
            return;
        }
        
        $sql_str = 
             "x.CONCRETE_PROTO_ID in (\n".
             "  select CONCRETE_PROTO_ID from CONCRETE_PROTO_STEP where \n". implode("\n and ", $cond_arr) ."\n group by CONCRETE_PROTO_ID\n".
             ")";
        
        debugOut('Sql-Query: <pre>'.$sql_str.'</pre>', $FUNCNAME, 2);
       
        
        
        $utilLib = new fSqlQueryC('CONCRETE_PROTO');
        $utilLib->cleanCond();
        $utilLib->_WhereAdd($sql_str, 'AND', 'selected protocol-parameters');
        $sqlAfter = $utilLib->get_sql_after( );
        
        $sqlsel = 'count(1) from '.$sqlAfter;
        $sqlo->Quesel($sqlsel);
        $sqlo->ReadRow();
        $cnt = $sqlo->RowData[0];
        
        if ($error->Got(READONLY))  {
            $error->set( $FUNCNAME, 1, 'Error on Parameter-SQL-query.' );
            return;
        }
        
        if (!$cnt) {
            $this->_html->messageBox2('warning', 'No protocols for these parameters found.');
            return;
        }
        
        $utilLib->queryRelase();
        
        return $cnt;
	}


	function startMain() {
		global $error;
		
		$FUNCNAME='startMain';
		
		
		$sqlo = &$this->_sqlo;
		$sqlo2= &$this->_plug_regSql2();
		$go   = $_REQUEST['go'];
		$parx = $_REQUEST['parx'];
		$action = $_REQUEST['action'];
		

		$settings_tmp  = $_SESSION["s_formState"]['o.CONCRETE_PROTO.li_search'];
		//debugOut('(416) IN:settings: '.print_r($this->settings,1), $FUNCNAME, 2);
		
		// $sqlAfter = $this->listLib->getSqlAfter();
		if ($action=='settings') {
		   
	        if (!$go) {
	            $hiddenarr=array( 'mod'=>$this->_mod );
	            $tmp = o_CONCRETE_PROTO_li_show1::settings_get();
	            $step_no=0;
	            o_CONCRETE_PROTO_li_show1::form_settings($tmp, $step_no, $hiddenarr);
	            return;
	        } else {
	            o_CONCRETE_PROTO_li_show1::settings_save($parx);
	        }
		    $go = 0;
		    $action='run';
		}
		$tool_opts = o_CONCRETE_PROTO_li_show1::settings_get();
		
		$pra_id = NULL;
		if ($parx['pra_id']) {
		    // set pra_ids
		    $pra_id = $parx['pra_id'];
		    $settings_tmp['pra_ids']=array($pra_id);  // temporary save
		    $action='sel_steps';
		    $go=0;
		}
		
		if ($action=='sel_pra' or empty($settings_tmp['pra_ids'])) {
			$this->form1(0);
			return;
		}
		
		$pra_lib  = new o_ABSTRACT_PROTO_qs($sqlo, $pra_id, $this->_mod);
		$pra_id   = $pra_lib->get_pra_id(); // urrent($thics->settings['pra_ids']);
		$settings = &$pra_lib->settings;    // reload settings ...
		
		$header = 
		    '[<a href="'.$this->_plug_get_URL().'&action=sel_pra">Select Protocol template</a>] '.
		    '[<a href="'.$this->_plug_get_URL().'&action=sel_steps">Select steps</a>] '.
		  		  '[<a href="'.$this->_plug_get_URL().'&action=sel_params">Select parameters</a>] '.
		  		  '[<a href="'.$this->_plug_get_URL().'&action=run">Search!</a>]';

		$this->_html->line_out($header);
		$this->_html->line_out('');
		$html_tmp = fObjViewC::bo_display( $sqlo, 'ABSTRACT_PROTO', $pra_id );
		$this->_html->line_out( tablename_nice2('ABSTRACT_PROTO').': '. $html_tmp);
		$this->_html->line_out('');
		
		
		
		
		gHtmlMisc::func_hist($this->_mod, $this->title, $this->_plug_get_URL() );
		
		
		//TBD: 
		if ($action=='' and !empty($this->settings['steps'])) {
		    $action='sel_params';
		    $go = 0;
		}
		
		
		
		if ($action=='sel_steps') {
		    if (!$go) {
		        $pra_lib->form_sel_steps($sqlo, $sqlo2);
		        return;
		    } else {
		        
		        $pra_lib->sel_steps_save($_REQUEST['x']);
		        $go = 0;
		        $action='sel_params';
		    }
		}
		debugOut('(607) action:$action', $FUNCNAME, 1);
		
		
		if ($action=='sel_params') {
		    if (!$go) {
		        $pra_lib->form_sel_params($sqlo);
		        return;
		    } else {
		        $pra_lib->sel_params_save($_REQUEST['x']);
		        
		        $go = 0;
		        $action='run';
		    }
		}
		
		if ($action=='run') {
		    
		    $pra_lib->form_sel_params($sqlo);
		    echo "<br>";
		    
		    $cnt = $this->run_query($sqlo, $settings);
		    
		    if ($cnt) {
		        
		      
		        $exa_cnt = oEXP_TMPL_protoC::exp_tmpl_has_proto($sqlo, $pra_id);
		        if ($exa_cnt) {
		            $mo_table='EXP';
		        }
		        $sua_cnt = oAS_HAS_PR_subs::suas_have_proto($sqlo, $pra_id);
		        if ($sua_cnt) {
		            $mo_table='CONCRETE_SUBST';
		        }
		        
		        
		        $utilLib = new fSqlQueryC('CONCRETE_PROTO');
		        $sqlAfter = $utilLib->get_sql_after( );
		        
		        $sqlsel = "x.CONCRETE_PROTO_ID from ".$sqlAfter.' order by x.CONCRETE_PROTO_ID DESC';  // Descending ...
		        $proto_arr = array();
		        $mo_arr    = array();
		        
		        //TBD: what, if too many objects
		        $MAX_OBJECTS=200;
		        
		        $cnt=0;
		        $sqlo2->Quesel($sqlsel);
		        while ( $sqlo2->ReadRow() ) {
		            
		            if ($cnt>$MAX_OBJECTS) {
		                
		                $this->_html->line_out('');
		                $this->_html->messageBox2('warning', 'Too many objects found. Will show only '.$cnt.' objects.');
		                $this->_html->line_out('');
		                break;
		            }
		            
		            $prc_id = $sqlo2->RowData[0];
		            $proto_arr[] = $prc_id;
		            
		            // search 
		            if ($mo_table=='EXP') {
		                $mo_id   = oEXPprotoC::get_one_exp_by_proto($sqlo, $prc_id);
		            }
		            if ($mo_table=='CONCRETE_SUBST') {
		                $mo_id   = oCS_HAS_PR_subs::get_SUC_by_PRC($sqlo, $prc_id);
		            }
		            $mo_arr[] = $mo_id;
		            $cnt++;
		        }
		       
		        
		        $show_objects = array('mo_arr'=>&$mo_arr,  'mo_table'=>$mo_table);

		        if(!is_array($tool_opts)) $tool_opts=array();
		        $tool_opts['form_url'] = $this->_plug_get_URL();
		       
 		        $prot_edit_lib = new o_CONCRETE_PROTO_li_show1($sqlo, $show_objects, $tool_opts);
 		        $prot_edit_lib->open_table($sqlo);
 		        $prot_edit_lib->show_one_pra($sqlo, $proto_arr);
 		        $prot_edit_lib->table_close();
 		        
 		        
		    }
		}
	}
	
}
