<?php
/**
 * - show a HTML search form for a TABLE
 * - create the SQL query based on that
 * @package glob.objtab.searchform.inc
 * 
 * @var array obj_usr_select_STRUCT -- the table select structure
 *    'sort' : {
 *       'col':string,
 *       'ord': string order string
 *    } 
 *    'cond' array (
 *        array('col', 'bool', 'val' )
 *     )
 *     'maxnum' : int max num of entries to show
 */
class gobjtab_searchform {
    
    function __construct($sqlo, $tablename) {
        $this->tablename = $tablename;
        $this->col_defs = array();
        
        $colNames = columns_get2($tablename);
        foreach($colNames as $col) {
            
            $show=0;
            $one_col_feat = colFeaturesGet( $sqlo,  $tablename, $col);
            if ($one_col_feat['VISIBLE']) {
                $show=1;
            }
            if ($col=='CCT_ACCESS_ID')  $show=0;
            if ($col=='EXTRA_OBJ_ID')  $show=0;
           
            
            if ($show) {
                $colcode='x.'.$col;
                $this->col_defs[$colcode]  = $one_col_feat;
            }
        }
    }
    
    private function _sel_sort($preselected) {
        echo '<select name="adv[sort][ord]" required>'."\n";
        echo "<option value=\"\">--- select ---</option>";
        
        $order_arr= array( 'ASC'=>'ASC', 'DESC'=>'DESC' );
        
        foreach( $order_arr as $key=>$val ) {
            $tmpsel = "";
            if ($preselected==$key) $tmpsel = " selected";
            echo "<option value=\"".$key."\" ".$tmpsel.">".$val."</option>";  
        }
        echo "</select>\n";
    }
    
    private function _sel_col($varname, $preselected, $required=0) {
        
        $required_str='';
        if ($required) $required_str=' required';
        
        echo '<select name="'.$varname.$required_str.'">'."\n";
        echo "<option value=\"\">--- select ---</option>";

        foreach( $this->col_defs as $colcode=>$row ) {

            // $col = substr($colcode,2);
            // $col_prefix = substr($colcode,0,1);
            
            $db_show= 1;
            $tmpsel = "";

            $col_nice = $row['NICE_NAME'];
            if ( $row['CCT_TABLE_NAME'] ) {
                $col_nice= "ID of ".$col_nice;
            }
            if ($preselected==$colcode) $tmpsel = " selected";
            
            if ($db_show) {
                echo "<option value=\"".$colcode."\" ".$tmpsel.">".$col_nice."</option>";
            }
        }
        echo "</select>\n";
    }
    
    /**
     * show HTML form
     * - variable name: adv
     * @param string $url
     * @param array $select_arr obj_usr_select_STRUCT
     *    'title' : string
     */
    function form(string $url, array $hiddenarr, array $select_arr, array $options=array() ) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        

        $tablename  = $this->tablename;
        
        echo '<form method="post"  action="'.$url.'" >';
        echo "<table border=0 cellpadding=1 cellspacing=0>\n";
        echo '<tr ><td colspan=3><span>&nbsp;&nbsp;&nbsp;<B>'.$options['title'].'</B></td></tr>'."\n";
        
        
        
        $tmp_arr=array();
        $tmp_arr["="]	="=";
        $tmp_arr["LIKE"]="LIKE";
        $tmp_arr[">"]	="&gt";
        $tmp_arr["<"] 	="&lt;";
        $tmp_arr[">="]	="&gt;=";
        $tmp_arr["<="] 	= "&lt;=";
        $tmp_arr["!="] 	= "!=";
        $tmp_arr["NOT LIKE"]="not LIKE";
        //FUTURE: support this ?
        //$tmp_arr["is NULL"] 	= "NULL";
        //$tmp_arr["is NOT NULL"] = "not NULL";
        // $tmp_arr["inlist"]  = "in list";
        $colsize = 40;
        
        for ( $rowid=0; $rowid<5; $rowid++ ) {
            
            $one_col_selected = $select_arr['cond'][$rowid];
            
            
            $value_ini = '';
            $col_selected = '';
            if (is_array($one_col_selected)) {
                $value_ini = $one_col_selected['val'];
                $col_selected = $one_col_selected['col'];
                $bool_selected = $one_col_selected['bool'];
                // TBD: more
            }
            
            echo "<tr valign=top>";
            echo "<td>";
             
            $this->_sel_col('adv[cond]['.$rowid.'][col]', $col_selected);
            
            
            echo "</td><td>";
            
            $tmp2_arr = $tmp_arr;
            
            
            
            echo '<select name=adv[cond]['.$rowid.'][bool]>'."\n";
            $tmpsel='';
            foreach( $tmp2_arr as $cond_tmp=>$cont_txt) {
                if ($cond_tmp==$bool_selected) $tmpsel = " selected";
                echo "<option value=\"".$cond_tmp."\" ".$tmpsel.">".$cont_txt."\n";
            }
            echo "</select>\n";
            echo "</td><td>";
            
            echo ' <input type=text name="adv[cond]['.$rowid.'][val]" value="'.$value_ini.'" size='.$colsize.'>'. "\n";
            
            echo "</td></tr>\n";
        }
        
        $show_sort=1;
        if ($show_sort) {
            
            $col_selected =  $select_arr['sort']['col'];
            echo '<tr><td>Sort by column:</td><td>';
            $this->_sel_sort( $select_arr['sort']['ord'] );
            echo '</td><td>';
            $this->_sel_col('adv[sort][col]', $col_selected, 1);
            echo "</td></tr>\n";
            echo "</table>\n";
            
        }

        
        echo "<tr><td>&nbsp;</td><td><input type=submit value=\"    Submit    \" class=yButton></td><td>&nbsp;</td></tr>\n";
        
        foreach($hiddenarr as $key=>$val) {
            echo '<input type=hidden name="'.$key.'" value="'.$val.'">'."\n";
        }
        
        echo "</form>\n";
    }
    
    function get_cond_nice(array $select_arr) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $colvals=array();
        
        if (empty($select_arr)) {
            return('no condition.');
        }
        
        foreach($select_arr['cond'] as $row) {
            
            $col_code = $row['col'];
            $val      = trim($row['val']);
            $bool = $row['bool'];
            if ($bool==NULL) $bool='=';
            
            $col_pure = substr($col_code,2);
            $prefix   = substr($col_code,0,1);
            
            $one_col_def = $this->col_defs[$col_code];
            
            $col_nice = $one_col_def['NICE_NAME'];
            if ( $one_col_def['CCT_TABLE_NAME'] ) {
                $col_nice= "ID of ".$col_nice;
            }
            
            switch ($prefix) {
                case 'x':
 
                    break;
                case 'a':
                    
                    break;
                default:
                    $error->set( $FUNCNAME, 5, 'Col-Prefix "'.$prefix.'" not defined' );
                    return;
            }
            
            $colvals[] = $col_nice . $bool . '"'.$val.'"';
        }
        
        $output = implode('; ',$colvals);
        
        return $output;
        
    }
    
    /**
     * repair FORM output
     * @param array $select_arr
     * @return void|array[]
     */
    function form_params_repair(array &$select_arr) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $new_sel_arr = array();
        $new_sel_arr['cond']=array();
        
        $tablename  = $this->tablename;
        
        if (!is_array($select_arr['sort']) ) {
            $error->set( $FUNCNAME, 1, 'Sort info missing.' );
            return;
        }
        
        foreach($select_arr['cond'] as $row) {
  
            
            $col_code = $row['col'];
            if ($col_code==NULL) {
                break; // stop it
            }
            
            $row['val'] = trim($row['val']);
            $bool     = $row['bool'];
            if ($bool==NULL) $bool='=';
            $val = $row['val'];
            
            
            
            if ($val==='') {
                $error->set( $FUNCNAME, 2, 'Condition for Column "'.$col_code.'" contains no value.' );
                return;
            }
            
            $col_pure = substr($col_code,2);
            $prefix   = substr($col_code,0,1);
            switch ($prefix) {
                case 'x':
                    
                    if ( !glob_column_exists($tablename, $col_pure) ) {
                        $error->set( $FUNCNAME, 3, 'Column "'.$col_code.'" not defined' );
                        return;
                    }
                    break;
                case 'a':
                    if ( !glob_column_exists('CCT_ACCESS', $col_pure) ) {
                        $error->set( $FUNCNAME, 4, 'Column "'.$col_code.'" not defined' );
                        return;
                    }
                    break;
                default:
                    $error->set( $FUNCNAME, 5, 'Col-Prefix "'.$prefix.'" not defined' );
                    return;
            }
            
            $new_sel_arr['cond'][] = $row;
            
            
        }
        
        if ($select_arr['sort']['col']==NULL ) {
            $error->set( $FUNCNAME, 6, 'Sort column missing.' );
            return;
        }
        
        $new_sel_arr['sort']=$select_arr['sort'];
        
        return $new_sel_arr;
    }
    
    function get_order_str(array &$select_arr) {
        
        if ( !is_array($select_arr['sort']) ) return;
        
        $order  = $select_arr['sort']['col'] .' '. $select_arr['sort']['ord']; 
        return $order;
    }
    
    /**
     * build SQL string
     * - only BOs supported
     * @param array $select_arr obj_usr_select_STRUCT
     */
    function get_sql( object $sqlo, array $select_arr ) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $tablename  = $this->tablename;
        
        if (!is_array($select_arr['cond'])) {
            $error->set( $FUNCNAME, 2, 'Table condition missing.' );
            return;
        }
        if ($select_arr['sort']==NULL) {
            $error->set( $FUNCNAME, 2, 'Table sort missing.' );
            return;
        }
        
        $colvals = array();
        
        foreach($select_arr['cond'] as $row) {
            
            $col_code = $row['col'];
            $val      = trim($row['val']);
            $bool = $row['bool'];
            if ($bool==NULL) $bool='=';
            
            if ($col_code==NULL) {
                break; // stop it
            }
            
            if ($val==='') {
                $error->set( $FUNCNAME, 2, 'Condition for Column "'.$col_code.'" contains no value.' );
                return;
            }
            
            $col_pure = substr($col_code,2);
            $prefix   = substr($col_code,0,1);
            switch ($prefix) {
                case 'x':
                    
                    if ( !glob_column_exists($tablename, $col_pure) ) {
                        $error->set( $FUNCNAME, 3, 'Column "'.$col_code.'" not defined' );
                        return;
                    }
                    break;
                case 'a':
                    if ( !glob_column_exists('CCT_ACCESS', $col_pure) ) {
                        $error->set( $FUNCNAME, 4, 'Column "'.$col_code.'" not defined' );
                        return;
                    }
                    break;
                default:
                    $error->set( $FUNCNAME, 5, 'Col-Prefix "'.$prefix.'" not defined' );
                    return;
            }
            
            $colvals[] = '('.$col_code . $bool . $sqlo->addQuotes($val).')';
        }
        
        $where = implode(' and ',$colvals);
        
        $sqlAfter = $tablename.' x join CCT_ACCESS a on x.CCT_ACCESS_ID=a.CCT_ACCESS_ID where '.$where;
        
        
        
        return $sqlAfter;
    }
    
}