<?php
 
/**
 * select user columns for searches/filter
 * @module glob.objtab.columns.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @var array objtab_column_STRUCT
     *   array( -- objtab_column_ONE_STRUCT
     *   'c'=>'a.CREA_DATE',
     *   'na'=>'creation date', NAME
     *   'no'=>'creation date', NOTES
     *   'ty'=> type of data area
     *      ['fea'] : normal table feature
     *      'acc', access
     *      'cls', class NAME
     *      'obj'  object
     *   'dt'=> NAME of APP_DATA_TYPE_ID
     *   'fk_tab' => NAME of linked TABLE
     *   
     *   );
 */
class gObjtab_column {
    
    function __construct($tablename) {
        $this->tablename=$tablename;
    }
    
    private function x_col_allowed($sqlo, $col_name) {
        
        if (!in_array($col_name, $this->colNames)) return 0;
        
        $colInfos  = colFeaturesGet($sqlo, $this->tablename, $col_name, 0);
        $showcol   = 1;
        if ( $colInfos['VISIBLE']=="0"  ) {
            $showcol = 0;
        }
        if ( $_SESSION['sec']['appuser'] == 'root' ) $showcol=1;
        return $showcol;
        
    }
    
    function get_start_columns($sqlo) {
        $tablename =  $this->tablename;
        $this->colNames  = columns_get_pos($tablename) ;
        $col_rows=array();
        
        foreach( $this->colNames as $colName) {
            $colInfos   = colFeaturesGet($sqlo, $tablename, $colName, 0);
            $showcol = $this->x_col_allowed($sqlo, $colName);
            if ( !$showcol ) {
                continue;
            }
            
            $do_save=0;
            
            if ($colInfos['PRIMARY_KEY']) {
                $do_save=1;
            }
            if ($colInfos['MOST_IMP_COL']) {
                $do_save=1;
            }
            if ($do_save) {
                $code_col_name='x.'.$colName;
                $col_rows[$code_col_name]=1;
            }
        }
        return $col_rows;
    }
    
    /**
     *
     * @param object $sqlo
     * @return array objtab_column_STRUCT
     */
    function get_all_columns($sqlo, &$columns) {
        
        $tablename =  $this->tablename;
        $this->colNames = &$columns;
        
        $col_rows=array();
        $colNamesX=NULL; // not needed now
        
        foreach( $this->colNames as $colName) {
            
            
            $tmp_nice_found  = 0;
            
            $colcomment = '';
            $colInfos   = colFeaturesGet($sqlo, $tablename, $colName, 0);
            
            $showcol = $this->x_col_allowed($sqlo, $colName);
            
            if ( !$showcol ) {
                continue;
            }
            
            $colNiceName="";
            if ( $colName == "CCT_ACCESS_ID" ) {
                $col_rows[] = array( 'c'=>'a.CREA_DATE',   'na'=>'creation date', 'no'=>'creation date',     'ty'=>'acc');
                $col_rows[] = array( 'c'=>'a.MOD_DATE' ,   'na'=>'modification date', 'no'=>'creation date', 'ty'=>'acc');
                $col_rows[] = array( 'c'=>'a.user_name',   'na'=>'creator', 'no'=>'user of creation',        'ty'=>'acc');
                $col_rows[] = array( 'c'=>"a.H_ALOG_ACT_ID",'na'=>"audit status", 'no'=>"audit status of object", 'ty'=>'acc');
                continue;
            }
            
            if ( $colName == "EXTRA_OBJ_ID"  )  {
                $colNiceName="extra class";
                $tmp_nice_found=1;
                $col_rows[] = array( 'c'=>"c.NAME",'na'=>"class", 'no'=>"class of object", 'ty'=>'cls');
                continue;
            }
            
            if ( !$tmp_nice_found )  {
                $colNiceName = $colInfos['NICE_NAME'];
            }
            
            if ( is_array($colNamesX) ) {
                $colNiceName = $colNamesX[$colName]; // empty nice names stay EMPTY !!!
            }
            
            if ( $colcomment=="" ) $colcomment = column_remark2($tablename, $colName);
            
            $code_col_name = 'x.'.$colName;
            
            $ret_primary_name=NULL;
            $ret_imp_name    =NULL;
            $linked_tab = fk_check2( $colName, $tablename, $ret_primary_name, $ret_imp_name );
            if ($linked_tab!="" AND $colName!="EXTRA_OBJ_ID" AND $colName!="CCT_ACCESS_ID") {
                $colcomment .= " <a href=\"view.tmpl.php?t=".$linked_tab."\">=&gt; table</a>";
                $colNiceName = 'ID of '.$colNiceName;
            }
            if ($colName=="EXTRA_OBJ_ID") {
                $colcomment .= " <a href=\"view.tmpl.php?t=EXTRA_CLASS&condclean=1&searchCol=TABLE_NAME&searchtxt=".$tablename."\">=&gt; classes</a>";
            }
            
            $dt = appDataTypeNameGet2($colInfos['APP_DATA_TYPE_ID']);
            $type='';
            if ($dt=='id') $type='obj';
           

            $col_rows[] = array( 'c'=>$code_col_name, 'na'=>$colNiceName, 'no'=>$colcomment, 'ty'=>$type, 'dt'=>$dt, 'fk_tab'=>$linked_tab);
            
        }
        
        return $col_rows;
    }
    
    function get_all_user_columns($sqlo) {
        $tablename =  $this->tablename;
        $colNames  = columns_get_pos($tablename) ;
        return $this->get_all_columns($sqlo, $colNames);
    }
}
