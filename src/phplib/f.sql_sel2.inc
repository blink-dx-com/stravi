<?php
require_once 'f.sql_query.inc';

/**
 * SQL select 2: set filter array => produces session query for a table
 * @package f.sql_sel2.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @var sql_filter_STRUCT : list of ..
     *   array(
     *   'col' : column with leading 'x.' or 'a.'
     *   'val' : 
     *   'con' : [=]
     *   'bool': ['AND'], 'OR'
     *   )
 */
class fSql_sel2 {
    
    
    /**
     * allowed conditions for method set_filter()
     * @var array
     *   'extra' allows a special VALUE without addQuotes !
     */
    private $cond_func_ACCEPT = array('=','<','>', '>=', '<=', 'LIKE', 'NOT LIKE', 'extra', 'IN LIST', 'IN FOLDER');
    
    /**
     * allow EXTERN for user
     * @var array $cond_func_ALLOW
     *    'IN LIST' must be translated into IN (...)
     *    'IN FOLDER'  must be translated into IN (select PRIM_KEY from PROJ_HAS_ELEM where PROJ_ID=xxx)
     */
    private $cond_func_ALLOW  = array('=','<','>', '>=', '<=', 'LIKE', 'NOT LIKE', 'IN LIST');
    
    // for special data types
    const cond_func_ALLOW_OBJ  = array('=','<','>', '>=', '<=', 'IN LIST', 'IN FOLDER');
    const cond_func_ALLOW_NUM  = array('=','<','>', '>=', '<=', 'IN LIST');
    const cond_func_ALLOW_STR  = array('=', 'LIKE', 'NOT LIKE', );
    
    function __construct($tablename) {
        $this->tablename=$tablename;
        $this->_info=array();
        $this->buildSqlLib = new fSqlQueryC ($this->tablename);
    }
   
    
    private function _add_info($text) {
        $this->_info[] = $text;
    }
    function get_info() {
        return $this->_info;
    }
    function cleanCond() {
        $this->buildSqlLib->cleanCond(); // clear old condition
    }
    function get_cond_func_ALLOW() {
        return $this->cond_func_ALLOW;
    }
    
 
    /**
     * column is valid ?
     * @param string $col_code
     * @return void|string[]
     */
    function column_is_valid($col_code) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $tablename  = $this->tablename;
        $col_prefix = substr($col_code,0,2);
        $col        = substr($col_code,2);
        
        switch($col_prefix) {
            case 'x.':
                if (!glob_column_exists($tablename, $col)) {
                    $error->set( $FUNCNAME, 5, 'Column "'.$col_code.'" unknown.' );
                    return;
                }
                break;
            case 'a.':
                if (!glob_column_exists('CCT_ACCESS', $col)) {
                    $error->set( $FUNCNAME, 5, 'Column "'.$col_code.'" unknown.' );
                    return;
                }
                break;
            default:
                $error->set( $FUNCNAME, 10, 'Column-Code "'.$col_code.'" unknown.' );
                return;
        }
        return array('col'=>$col, 'prefix'=>$col_prefix);
    }
    
    private static function _colcode_decode($col_code) {
        $col_prefix = substr($col_code,0,2);
        $col = substr($col_code,2);
        return array($col_prefix, $col);
    }
    
    /**
     * - set filter 
     * - do NOT release the query
     * @param object $sqlo
     * @param array $filter sql_filter_STRUCT
     */
    function set_filter($sqlo, $filter) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $tablename  = $this->tablename;
        $cond_allow = $this->cond_func_ACCEPT; 
        
        foreach($filter as $row) {
            
            $col_code = $row['col'];
            $val = $row['val'];
            $val = trim($val);
            $cond='=';
            if ($row['con']) {
                $cond=$row['con'];
            }
            $col_prefix = substr($col_code,0,2);
            $col = substr($col_code,2);
            
            if (!in_array($cond, $cond_allow)) {
                $error->set( $FUNCNAME, 5, 'Condition "'.$cond.'" unknown.' );
                return;
            }
            
            do {
                if ($cond=='IN LIST') {
                    $cond='IN';
                    //check values ??? 
                    $valarr=explode(',',$val);
                    foreach($valarr as $valx) {
                        if(!is_numeric($valx)) {
                            $error->set( $FUNCNAME, 10, 'Filter "'.implode(' ',$row).'" is invalid. VAl-list contains NON-numeric values.' );
                            return;
                        }
                    }
                }
                if ($cond=='IN FOLDER') {  
                    if($col_prefix!='x.' ) {
                        $error->set( $FUNCNAME, 15, 'Condition '.$cond.' is only allowd for "x" Columns.' );
                        return;
                    }
                    if(!is_numeric($val)) {
                        $error->set( $FUNCNAME, 20, 'Filter "'.$val.'" is invalid. VAL must be numeric.' );
                        return;
                    }
                    $colInfos  = colFeaturesGet($sqlo, $tablename, $col);
                    $fk_table='';
                    if ($colInfos['CCT_TABLE_NAME']!=NULL) $fk_table=$colInfos['CCT_TABLE_NAME'];
                    if ($fk_table==NULL and $colInfos['PRIMARY_KEY']==1) $fk_table=$tablename;
                    if ($fk_table==NULL) {
                        $error->set( $FUNCNAME, 22, 'Condition '.$cond.' is not allowd for this column ".$col_code.".' );
                        return;
                    }
                        
                    $cond='IN';
                    $val = 'select PRIM_KEY from PROJ_HAS_ELEM where PROJ_ID='.$val.' AND TABLE_NAME='.$sqlo->addQuotes($fk_table);
                    
                }
            } while(0);
            
            
            
            
            switch($col_prefix) {
                case 'x.':
                    if (!glob_column_exists($tablename, $col)) {
                        $error->set( $FUNCNAME, 25, 'Column "'.$col_code.'" unknown.' );
                        return;
                    }
                    break;
                case 'a.':
                    if (!glob_column_exists('CCT_ACCESS', $col)) {
                        $error->set( $FUNCNAME, 30, 'Column "'.$col_code.'" unknown.' );
                        return;
                    }
                    $this->buildSqlLib->addJoin('CCT_ACCESS');
                    break;
                    
                    
                case 'c.':
                    $this->buildSqlLib->addJoin('EXTRA_CLASS'); 
                    $this->buildSqlLib->addJoin('EXTRA_OBJ');
                      
                    if ( $col_code!='c.NAME') {
                        $error->set( $FUNCNAME, 40, 'Column "'.$col_code.'" not allowed. Prefix "c." only "c.NAME" allowed.' );
                        return;
                    }
                    break;
                
                default:
                    $error->set( $FUNCNAME, 50, 'Column-Code "'.$col_code.'" unknown.' );
                    return;
            }
           
            
            if ($val==NULL) {
                $error->set( $FUNCNAME, 60, 'Col: "'.$col.'": no Value given.' );
                return;
            }
            $tbool='';
            if($row['bool']!=NULL) {
                $tbool = $row['bool'];
                if ($tbool!='AND' and $tbool!='OR' ) {
                    $error->set( $FUNCNAME, 70, 'Bool: "'.$tbool.'": not valid.' );
                    return;
                }
            }
            
            $this->_add_info('- Add filter "'.columnname_nice2($tablename, $col) .':'.$val);
            
            $this->buildSqlLib->add_column_cond($sqlo, $col_code, $cond, $val, $tbool );
        } 
    }
    
    /**
     * manage SQL construct by selected columns
     * - add joins
     * @param object $sqlo
     * @param array $user_cols array of column_codes
     */
    function set_select_cols(object $sqlo, array $user_cols) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        foreach($user_cols as $col_code) {
            
            list($prefix, $colname) = self::_colcode_decode($col_code);
            switch ($prefix) {
                case 'x.':
                    break;
                case 'a.':
                    $this->buildSqlLib->addJoin('CCT_ACCESS');
                    break;
                case 'c.':
                    $this->buildSqlLib->addJoin('EXTRA_CLASS');
                    $this->buildSqlLib->addJoin('EXTRA_OBJ');
                    break; 
                default:
                    $error->set( $FUNCNAME, 10, 'Column-Code "'.$col_code.'" unknown.' );
                    return;
            }
        }
    }
    
    function addJoin($usejoin) {
        $this->buildSqlLib->addJoin($usejoin);
    }
    function queryRelase() {
        $this->buildSqlLib->queryRelase();
    }
    
    /** 
     * get SQL afterwithout order by ...
     * a filter must be set ...
     * @param object $sqlo
     * @return void|sql-string
     */
    function get_sql_after() {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;

        $info = $this->buildSqlLib->get_sql_info();
        if ($info==NULL) {
            $error->set( $FUNCNAME, 1, 'No search filter set!' );
            return;
        }
         
        $sql_after = $this->buildSqlLib->get_sql_after();
        return $sql_after;
    }
    
    function query_sort_org() {
        $this->buildSqlLib->_query_sort_org();
    }
    function get_sql_info() {
        return $this->buildSqlLib->get_sql_info();
    }
    function query_sort_get( $sortcrit ) { 
        return $this->buildSqlLib->_query_sort_get($sortcrit);
    }
    
    /**
     * 
     * @param array $sort_arr
     *   [
     *     {'col':'', 'dir':ASC,DESC }
     *   ]
     * @return string
     */
    function set_sort_arr( $sort_arr ) {
        $sort_str_arr=array();
        if (!is_array($sort_arr)) $sort_arr=array();
        foreach($sort_arr as $row) {
            $sort_str_arr[]=$row['col'].' '.$row['dir'];
        }
        $out=implode(', ',$sort_str_arr);
        return $out;
    }
}
