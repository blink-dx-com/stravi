<?php
require_once 'gui/glob.objtab.columns.inc';
require_once 'f.sql_sel2.inc';
require_once 'func_form.inc';
require_once 'lev1/f.search_ulib.inc';

/**
 * - alternative table FILTER GUI
 * - supports FILTER + SORT definition
 * - FORM-variable: $filt : f.sql_sel2.inc:sql_filter_STRUCT 
 * @package glob.objtab.filter_GUI2.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @var uses glob.objtab.columns.inc:objtab_column_STRUCT
 */
class g_objtab_filter_GUI2 {
    
    private $sqlAfter = NULL;
    private $sqlAfterSort = NULL;
    private $_col_rows; // array objtab_column_STRUCT
   
    /**
     * @var array see o.LINK.list_user.inc:DOC_config_STRUCT
     * 'sort' : [
     *       {'col':'', 'dir':ASC,DESC }
     *   ]
     * 'filt' => ...
     */
    private $json_arr;
    
    // construct
    function __construct( $sqlo, $tablename, &$json_arr, $url_normal) {
        $this->tablename  = $tablename;
        $this->json_arr   = &$json_arr;
        $this->url_normal = $url_normal;
        $this->get_all_user_columns($sqlo);
        $this->sql_file_lib = new fSql_sel2($tablename);
    }
    
    // return array objtab_column_STRUCT
    private function get_all_user_columns($sqlo) {
        if (empty($this->_col_rows)) {
            $tab_col_lib = new gObjtab_column($this->tablename);
            $this->_col_rows = $tab_col_lib->get_all_user_columns($sqlo);
        }
        return  $this->_col_rows;
    }
    
    function set_columns($sqlo, $columns) {
        $tab_col_lib = new gObjtab_column($this->tablename);
        $this->_col_rows = $tab_col_lib->get_all_columns($sqlo, $columns);
    }
    
    function js()  {
        ?>
        <script>

        function add_col_deact_val(value) {
            
        	var e = document.getElementById("add_column");
        	/* deactivate this option*/
        	options = e.getElementsByTagName("option");
        	cnt= options.length;
        	for (i = 0; i < cnt; i++) {
        	    if ( value == options[i].value ) {
        	    	options[i].disabled = true;
        	    	break;
        	    }    
        	}
        }
        
        function add_columnx() {
        	var e = document.getElementById("add_column");
        	var col_code = e.options[e.selectedIndex].value;
        	var textx = e.options[e.selectedIndex].text;

        	add_col_deact_val(col_code);

        	var tablex = document.getElementById("all_columns");

        	i = tablex.getElementsByTagName("tr").length - 1;
        	
        	var ori_row = document.getElementById("tmpl_row");
        	var new_row = ori_row.cloneNode(true);   

        	var cols = new_row.getElementsByTagName("td");

     	    var td0  = cols[0];
        	var inp1 = td0.getElementsByTagName("input")[0]; 
        	inp1.name  = 'filt['+i+'][col]';
        	inp1.value = col_code;
        	var inp2   = td0.getElementsByTagName("input")[1]; 
        	inp2.name='filt['+i+'][act]';
        	td0.innerHTML = td0.innerHTML + textx;
        	
        	var inp3 = cols[1].getElementsByTagName("select")[0]; 
        	inp3.name='filt['+i+'][con]';
        	var inp4 = cols[2].getElementsByTagName("input")[0]; 
        	inp4.name='filt['+i+'][val]';
        	
        	new_row.style.display='';
        	new_row.id=null;

        	tablex.appendChild(new_row);
        }
        </script>
        <?php
    }
    
    
    
    /**
     * get column features
     * @param string $col_code
     * @param array $col_rows
     * @return array ONE objtab_column_STRUCT
     */
    private function column_features(string $col_code) {
        
        $found=0;
        foreach($this->_col_rows as $col_row) {
            if ( $col_code==$col_row['c'] ) {
                $found=1;
                break;
            }
        }
        if (!$found) {
            throw new Exception('colcode "'.$col_code.'" not found');
        }
        return $col_row;
    }
    
    private function _add_column() {
        
        $sel_array=array();
        foreach($this->_col_rows as $col_row) {
            $col_code=$col_row['c'];
            $col_nice=$col_row['na'];
            $sel_array[$col_code]=$col_nice;
        }
        $options=array(
            'selecttext'=>'--- add Filter ---',
            'id'=>'add_column',
            'selbody'=>'onChange="add_columnx();"'
        );
        echo formc::selectFget('XX2', $sel_array, '', $options);
    }
   
    /**
     * show one row
     * @param int $i
     * @param string $col_code
     * @param array $col_features -- ONE objtab_column_STRUCT
     * @param array $row
     * @param int $hide
     */
    private function _one_row(int $i, string $col_code, array &$col_features, array $row, int $hide=0)  {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        $html_opt='';
        
        $is_object=0;
        if ($col_features['ty']=='obj') {
            $is_object=1;
        }
        $col_nice = $col_features['na'];
        
        // debugOut("(165) col_features:".print_r($col_features,1), $FUNCNAME, 2);
 
        $datatype_RAW='';
        if ($col_features['dt']) {
            $app_id = get_app_data_type_id($col_features['dt']);
            $app_id_features=appDataType_arr($app_id);
            $datatype_RAW = $app_id_features['DATA_TYPE'];
        }
        
        
        
        if ($hide)  $html_opt=' id="tmpl_row" style="display:none;"';  
        echo '<tr '.$html_opt.'><td>'."\n";
        echo '<input type="hidden" name="filt['.$i.'][col]" value="'.$col_code.'">';
        echo '<input type="checkbox" name="filt['.$i.'][act]" value="1" checked>';
        echo $col_nice;
        echo "</td><td>"."\n";
        
        // CONDITIONS
        $cond_func_arr     = $this->sql_file_lib->get_cond_func_ALLOW();
        $use_arr = &$cond_func_arr;
        
        do {
            if ($is_object) {
                $use_arr = fSql_sel2::cond_func_ALLOW_OBJ;
                break;
            }
            switch($datatype_RAW) {
                case 'INT':
                case 'FLOAT':
                    $use_arr = fSql_sel2::cond_func_ALLOW_NUM;
                    break;
                case 'STRING':
                    $use_arr = fSql_sel2::cond_func_ALLOW_STR;
                    break;
                case 'DATE':
                case 'DATE1':
                case 'DATE2':
                    $use_arr = fSql_sel2::cond_func_ALLOW_NUM;
                    break;
                    
            }
        } while (0);
        
        debugOut("(207) DT:".$col_features['dt']." datatype_RAW:$datatype_RAW is_object:$is_object use_arr:".print_r($use_arr,1), $FUNCNAME, 2);
        
        $sel_array=array();
        foreach( $use_arr as $cond_tmp) {
            $cont_txt = $cond_tmp;
            $sel_array[$cond_tmp]=$cont_txt;
        }
        echo formc::selectFget('filt['.$i.'][con]', $sel_array, $row['con']);
        
        echo "</td><td>"."\n";
        
        $val = $row['val'];
        echo ' <input type=text name="filt['.$i.'][val]" value="'.$val.'" size=20> '."\n";

        echo '</td><td style="color:gray; font-size:0.8em;">for LIKE: use % as wildcard</td>'."\n";
        echo "</tr>\n";
    }
    
    /**
     *
     * INPUT: 
     * MAIN COL array
     *  of array(
     *   'col' : column with leading 'x.' or 'a.'
     *   'val' :
     *   'con' : [=]
     *   'bool': ['AND'], 'OR'
     *   'act' : 1 : is active
     *   )
     * @param array $options
     *     'with_order_form' include sort column selection
     * @param object $sqlo
     */
    function form_main($sqlo, $options=array() ) {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $tablename  = $this->tablename;
        $json_arr   = &$this->json_arr;
        $filter_arr = $json_arr['filt'];


        debugOut('(209) filter_arr:'.print_r($filter_arr,1), $FUNCNAME, 1);
        
        if (empty($filter_arr)) $filter_arr=array();
        //echo "DDDX: XX2:".print_r($filter_arr,1)."<br>";
        
        ?>
        <div class="container" style="display: flex; ">
        <div style="width: 80%; flex-grow: 3;">
        <?php

        echo '<table id="all_columns">'."\n";
        
        $row = array();
        $dummy=array();
        // add hidden CLONE row
        // TBD: put datatype info to the hidden row with DATA TYPE info ...
        $this->_one_row(-1, '', $dummy, $row, 1); 
        
        
        $i=0;
        // $row: objtab_column_ONE_STRUCT
        foreach($filter_arr as $row) {
            
            $col_code =  $row['col'];
            if ($col_code==NULL) continue; // error ...
            
            $col_features = $this->column_features( $col_code );
            $this->_one_row($i, $col_code, $col_features, $row);
            
            $i++;
        }
        
       
        
        echo "</table>\n";
        
        ?>
        </div>
        <div style="flex-grow: 1;"> 
        <?php 
        $this->_add_column();
        
        // deactivate option ...
        echo '<script>'."\n";
        foreach($filter_arr as $row) {
            $col_code =  $row['col'];
            echo '  add_col_deact_val("'.$col_code.'");'."\n";
        }
        echo '</script>'."\n";
        ?>
        </div>
        </div>
        <?php
        
        
    }
 
    /**
     * TBD: do tests here
     * just checkings here
     * @param object $sqlo
     * @param array $user_input
     */
    function user_filter_save($sqlo, $input) {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;

        
        $controlled_input = array();
        if (!empty($input)) {
            foreach($input as $i=>$row) {
                if ($i<0) continue;
                if (!$row['col']) continue;
                if (!$row['act']) continue;
                if ($row['val']==='' or $row['val']===NULL) continue; // TBD: please handle the NULL condtion !
                $controlled_input[] = $row;
            }
        }
        debugOut('(290) FILTER:output: '.print_r($controlled_input,1), $FUNCNAME, 1);
        $this->json_arr['filt'] = $controlled_input;
    }
    
    function set_sort($sort_arr) {
        $sort_str = implode(', ',$sort_arr);
        $this->json_arr['sort'] = $sort_str;
    }
  
    
    function create_SQL($sqlo) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $this->sqlAfter = NULL;
        $this->sqlAfterSort = NULL;
        $tablename = $this->tablename;
        
        $user_filter_pre_lib = new f_search_ulib2($tablename);
        $filter_sql_sel2 = $user_filter_pre_lib->filter($sqlo, $this->json_arr['filt']);
        
        debugOut('(323) FILTER_2:'.print_r( $filter_sql_sel2,1), $FUNCNAME, 1);
        
        if ($error->Got(READONLY))  {
            $error->set( $FUNCNAME, 10, 'Error on User-filter-analysis.' );
            return;
        }
        
        $this->sql_file_lib->cleanCond();
        $this->sql_file_lib->set_filter($sqlo, $filter_sql_sel2);
        
        //TBD: if SELECT_COLUMNS define the SQL query ... 
        //$this->sql_file_lib->set_select_cols($sqlo, $user_cols4filter);
        
        $this->sqlAfter = $this->sql_file_lib->get_sql_after();
        if ($error->Got(READONLY))  {
            $error->set( $FUNCNAME, 20, 'Error on Filter-analysis.' );
            return;
        }
        
        $tmpSort='';
        if ($this->json_arr['sort']) {
            $sortcritXText  = $this->sql_file_lib->set_sort_arr( $this->json_arr['sort'] );
            debugOut('(253) SORT_txt:'.$sortcritXText, $FUNCNAME, 1);
            $tmpSort        = $this->sql_file_lib->query_sort_get($sortcritXText);
        }
        $this->sqlAfterSort = $this->sqlAfter . $tmpSort;
    }
    
    function queryRelase() {
        $this->sql_file_lib->queryRelase();
    }
 
    function get_filt_arr() {
        return $this->json_arr['filt'];
    }
    
    function get_sqlAfter() {
        return $this->sqlAfter;
    }
    function get_sqlAfterSort() {
        return $this->sqlAfterSort;
    }
}
