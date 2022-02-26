<?php
require_once 'gui/glob.objtab.columns.inc';
require_once 'f.sql_sel2.inc';
require_once 'func_form.inc';
require_once 'lev1/f.search_ulib.inc';

/**
 * - table FILTER GUI
 * - supports FILTER + SORT definition
 * @package glob.objtab.filter_GUI.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class g_objtab_filter_GUI {
    
    private $sqlAfter = NULL;
    private $sqlAfterSort = NULL;
    
   
    /**
     * @var array see o.LINK.list_user.inc:DOC_config_STRUCT
     * 'sort' : [
     *       {'col':'', 'dir':ASC,DESC }
     *   ]
     * 'filt' => ...
     */
    private $json_arr;
    
    // construct
    function __construct( $tablename, &$json_arr, $url_normal) {
        $this->tablename  = $tablename;
        $this->json_arr   = &$json_arr;
        $this->url_normal = $url_normal;
    }
    
    function get_filter_arr() {
        return  $this->json_arr['filt'];
    }
    
    function get_config_full() {
        if ( $_SESSION["userGlob"]["g.debugLevel"]>=2 ) {
            $FUNCNAME= __CLASS__.':'.__FUNCTION__;
            debugOut('(42) json_arr: '.print_r($this->json_arr,1), $FUNCNAME);
        }
        return  $this->json_arr;
    }
    
    // return array objtab_column_STRUCT
    private function get_all_user_columns($sqlo) {
        if (empty($this->_col_rows)) {
            $tab_col_lib = new gObjtab_column($this->tablename);
            $this->_col_rows = $tab_col_lib->get_all_user_columns($sqlo);
        }
        return  $this->_col_rows;
    }
    
    /**
     * show select box for CLASSES
     * @param  $sql
     * @param  $tablename
     * @param  $classname
     * @param  $class_tab_has
     * @return -
     */
    private function column_get_classes( &$sql ) {
        
        $tablename  = $this->tablename;
        $sqlsel="NAME, NICE_NAME from extra_class where table_name='".$tablename."' order by UPPER(NICE_NAME)";
        $sql->Quesel($sqlsel);
        $outarr = array();
        while ( $sql->ReadRow() ) {
            $class_name_tmp=$sql->RowData[0];
            $class_nice   = $sql->RowData[1];
            $outarr[$class_name_tmp]=$class_nice;
        }
        
        return $outarr;
    }
    
    function add_col($new_col) {
        $this->json_arr['filt'][] = array('col'=>$new_col );
    }
    
    function del_col($col_index) {
        if ( !is_numeric($col_index) ) {
            throw new Exception('Input: col_index is not numeric.');
        }
        unset($this->json_arr['filt'][$col_index]);
    }
    
    /**
     * get column features
     * @param string $col_code
     * @param array $col_rows
     * @return string
     */
    private function column_features(string $col_code, array &$col_rows) {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        $found=0;
        foreach($col_rows as $col_row) {
            if ( $col_code==$col_row['c'] ) {
                $found=1;
                break;
            }
        }
        if (!$found) {
            debugOut('(106) col_rows:'.print_r($col_rows,1), $FUNCNAME, 1);
            throw new Exception('colcode "'.$col_code.'" not found');
        }
        return $col_row;
    }
    
    
    private function column_select($this_col, $col_rows, $varname) {
        
        $sel_array=array();
        foreach($col_rows as $col_row) {
            // array( 'c'=>'a.CREA_DATE',   'na'=>'creation date', 'no'=>'creation date',  'ty'=>'acc');
            $col_code = $col_row['c'];
            $col_nice = $col_row['na'];
            
            // 	        if ($col_row['fk_tab']) {
            // 	            $col_nice = 'ID of '.$col_nice;
            // 	        }
            if ( $col_code=="a.DB_USER_ID" ) {
                $col_code = "a.user_name";
            }
            if ( $col_code=='x.EXTRA_OBJ_ID') {
                $col_code = "c.NAME";
            }
            
            $sel_array[$col_code]=$col_nice;
        }
        $sel_opt = array('req'=>1);
        return formc::selectFget($varname , $sel_array, $this_col, $sel_opt);
    }
    
    function form2_1_column($sqlo, $new_i) {

        $col_rows = $this->get_all_user_columns($sqlo);
        
        echo '<h2>Select a column</h2><br>';
        echo '<form method="post" action="'.$this->url_normal.'&go=1">'."\n";
        echo '<input type="hidden" name="action" value="conf_filt">';
        echo '<input type="hidden" name="parx2[action]" value="add_col">';
        
        echo $this->column_select('', $col_rows, 'parx2[col]');
        
        echo '<input type=submit value="Add" class="yButton">';
        echo '</form>';
    }
    
    private function _form_row($x, $y, $z) {
        echo '<tr><td>';
        echo $x;
        echo '</td>';
        echo '<td>';
        echo $y;
        echo '</td>';
        echo '<td>';
        echo $z;
        echo "</td>";
        echo '<td style="padding-left:20px;">';
        
        echo '</td>';
        echo "</tr>\n";
    }
    
    private function sort_row($sqlo) {
        
        //   {'col':'', 'dir':ASC,DESC }
        if (!empty($this->json_arr['sort'])) $first_sort = $this->json_arr['sort'][0];
        else $first_sort = array();
        
        $col_selected =  $first_sort['col'];
        $col_rows = $this->get_all_user_columns($sqlo);
        
        $col_sel_html = $this->column_select($col_selected, $col_rows, 'parx2[sort][col]') ;
        
        $order_arr= array( 'ASC'=>'Ascending', 'DESC'=>'Descending' );
        $dir_selected = $first_sort['dir'];
        $dir_sel_html = formc::selectFget('parx2[sort][dir]' , $order_arr, $dir_selected);
 
        $this->_form_row('<b>Order by: *</b>', $col_sel_html, $dir_sel_html);
        
    }
    
    /**
     *
     * INPUT: $_REQUEST['parx2']
     *    action =
     *      add_col
     *      del_col
     * MAIN COL array
     *  of array(
     *   'col' : column with leading 'x.' or 'a.'
     *   'val' :
     *   'con' : [=]
     *   'bool': ['AND'], 'OR'
     *   )
     * @param array $options
     *     'with_order_form' include sort column selection
     * @param object $sqlo
     */
    function form_main($sqlo, $user_input, $options=array() ) {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $tablename  = $this->tablename;
        $json_arr   = &$this->json_arr;
        $filter_arr = $json_arr['filt'];
        
        
        $col_rows = $this->get_all_user_columns($sqlo);
        
        $sql_file_lib  = new fSql_sel2($tablename);
        $cond_func_arr = $sql_file_lib->get_cond_func_ALLOW();
        $cond_func_OBJ_arr = fSql_sel2::cond_func_ALLOW_OBJ;
        
        debugOut('(209) filter_arr:'.print_r($filter_arr,1), $FUNCNAME, 1);
        
        if (empty($filter_arr)) $filter_arr=array();
        //echo "DDDX: XX2:".print_r($filter_arr,1)."<br>";
        
        echo '<form method="post" action="'.$this->url_normal.'&go=1">'."\n";
        echo '<input type="hidden" name="action" value="conf_filt">';
        
        
        
        echo "<table>\n";
        
       
        
        $i=0;
        // $row: objtab_column_ONE_STRUCT
        foreach($filter_arr as $row) {
            
            $col_code =  $row['col'];
            if ($col_code==NULL) continue; // error ...
            
            $col_features = $this->column_features( $col_code, $col_rows);
            $col_nice = $col_features['na'];
            $is_object=0;
            if ($col_features['ty']=='obj') {
                $is_object=1;
            }
            
            echo '<tr><td>';
            echo '<input type="hidden" name="parx['.$i.'][col]" value="'.$col_code.'">';
            echo $col_nice;
            echo "</td><td>";
            
            // CONDITIONS
            $sel_array=array();
            $use_arr = &$cond_func_arr;
            if ($is_object) $use_arr = &$cond_func_OBJ_arr;
            
            foreach( $use_arr as $cond_tmp) {
                $cont_txt = $cond_tmp;
                $sel_array[$cond_tmp]=$cont_txt;
            }
            echo formc::selectFget('parx['.$i.'][con]', $sel_array, $row['con']);
            
            echo "</td><td>";
            
            $val = $row['val'];
            switch ($col_code) {
                case 'c.NAME':
                    $sel_array = $this->column_get_classes($sqlo);
                    echo formc::selectFget('parx['.$i.'][val]', $sel_array, $val);
                    break;
                default:
                    echo ' <input type=text name="parx['.$i.'][val]" value="'.$val.'" size=20> '."\n";
            }
            
            
            echo "</td>";
            
            echo '<td style="padding-left:20px;">';
            echo '<a href="'.$this->url_normal.'&action=conf_filt&parx2[action]=del_col&parx2[col_i]='.$i.'"><img src="images/ic.del.gif" title="Delete row"></a>';
            echo '</td>';
            
            echo "</tr>\n";
            
            $i++;
        }
        
        if ( $options['with_order_form'] ) {
            $this->sort_row($sqlo);
        }
        
        echo "</table>\n";
        
        echo '<input type=submit value="Save" class="yButton">';
        echo '</form>';
        
        echo '<br>[<a href="'. $this->url_normal.'&action=conf_filt&parx2[action]=add_col">ADD condition</a>]<br>';
    }
    
 
    /**
     * TBD: do tests here
     * just checkings here
     * @param object $sqlo
     * @param array $user_input
     */
    function save_filter($sqlo, $user_input) {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        debugOut('(290) FILTER:user_input: '.print_r($user_input,1), $FUNCNAME, 1);
        $controlled_input = $user_input;
        $this->json_arr['filt'] = $controlled_input;
    }
    
    /**
     * TBD: do tests here
     * just checkings here
     * @param object $sqlo
     * @param array $user_input --- can be empty
     *   array ( array('col', 'dir') )
     */
    function save_sort($sqlo, $user_input) {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        debugOut('(294) save SORT:user_input:'.print_r($user_input,1), $FUNCNAME, 1);
        
        if (!empty($user_input)) {
            if (!is_array($user_input)) {
                throw new Exception('Input: Sort-input must be [] of {} (ERR:1).');
            }
            if (!is_array($user_input[0])) {
                throw new Exception('Input: Sort-input must be [] of {}  (ERR:2).');
            }
        }
        
        $this->json_arr['sort'] = $user_input;
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
        
        $sql_file_lib = new fSql_sel2($tablename);
        $sql_file_lib->cleanCond();
        $sql_file_lib->set_filter($sqlo, $filter_sql_sel2);
        
        //TBD: if SELECT_COLUMNS define the SQL query ... 
        //$sql_file_lib->set_select_cols($sqlo, $user_cols4filter);
        
        $this->sqlAfter = $sql_file_lib->get_sql_after();
        if ($error->Got(READONLY))  {
            $error->set( $FUNCNAME, 20, 'Error on Filter-analysis.' );
            return;
        }
        
        $sortcritXText  = $sql_file_lib->set_sort_arr( $this->json_arr['sort'] );
        debugOut('(253) SORT_txt:'.$sortcritXText, $FUNCNAME, 1);
        $tmpSort        = $sql_file_lib->query_sort_get($sortcritXText);
        $this->sqlAfterSort = $this->sqlAfter . $tmpSort;
    }
    
    /**
     * get NICE presentaion of condition
     * @param object $sqlo
     * @return void|string
     */
    function get_cond_nice($sqlo) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $colvals=array();
        $select_arr = $this->json_arr['filt'];
        
        if (empty($select_arr)) {
            return('no condition.');
        }
        
        $tab_col_lib = new gObjtab_column($this->tablename);
        $col_rows = $tab_col_lib->get_all_user_columns($sqlo);
        
        foreach($select_arr as $row) {
            
            $col_code = $row['col'];
            $val      = trim($row['val']);
            $bool     = $row['con'];
            if ($bool==NULL) $bool='=';
            
            // $col_pure = substr($col_code,2);
            $prefix   = substr($col_code,0,1);
            $col_features = $this->column_features($col_code, $col_rows);
            $col_nice = $col_features['na'];
            
            switch ($prefix) {
                case 'x':
                    break;
                case 'a':
                    
                    break;
                default:
                    $error->set( $FUNCNAME, 5, 'Col-Prefix "'.$prefix.'" not defined' );
                    return;
            }
            
            $colvals[] = $col_nice .' '. $bool . ' "'.$val.'"';
        }
        
        $output = implode('; ',$colvals);
        
        return $output;
    }
    
    function get_sqlAfter() {
        return $this->sqlAfter;
    }
    function get_sqlAfterSort() {
        return $this->sqlAfterSort;
    }
}
