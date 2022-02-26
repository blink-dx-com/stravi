<?php
require_once 'insert.inc';
require_once 'insertx.inc';
require_once 'get_cache.inc';

/**
 * use as CACHE_CUBE
 * manage a temporary SQLite database
 * @package g.db_tmp.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class g_db_tmp {
    
    static function &get_handle() {
        
        $_dbtype='CDBSqlite';
        $db='/tmp/goz_tmpsqlite.db';
        $user=NULL;
        $passwd=NULL;
        $sqlo_local = logon_to_db( $user, $passwd, $db, $_dbtype);
        return $sqlo_local;
    }

}

/**
 * database STRUCTURE modification methods
 *
 */
class g_db_tmp_MOD {
    
    const DB_TMP_CONFIG_KEY='goz_tmpsqlite';
    
    const APP_DATA_TYPE_2sqlite = array(
        'string'  => 'STRING',
        'notes'   => 'STRING',
        'name'    => 'STRING',
        'integer' => 'INT',
        'id'      => 'INT',
        'float'   => 'REAL',
        'email'   => 'STRING',
        'date'    => 'DATE',
        'boolean' => 'INT',
        
    );
    
    /**
     * 
     * @param object $sqlo_tmp  -- handle to this LITE database
     * @param object $sqlo_main -- handle to MAIN database (for CCT_TABLE)
     */
    function __construct(object &$sqlo_tmp, object &$sqlo_main) {
        $this->_sqlo = &$sqlo_tmp;
        $this->_sqlo_main = &$sqlo_main;
    }
    
    /**
     * init a fresh database
     */
    function init() {
        // The WAL journaling mode uses a write-ahead log instead of a rollback journal to implement transactions. 
        $this->_sqlo->Exec('PRAGMA journal_mode = wal;');
    }
    
    function table_exists($tablename) {
        $sqlsel ="count(*) FROM sqlite_master WHERE type='table' AND name = ".$this->_sqlo->addQuotes($tablename);
        $this->_sqlo->Quesel($sqlsel);
        $this->_sqlo->ReadRow();
        $cnt = $this->_sqlo->RowData[0];
        return $cnt;
    }
    
    function get_all_tables() {
        $sqlsel ="name FROM sqlite_master WHERE type='table'";
        $this->_sqlo->Quesel($sqlsel);
        $tables=array();
        while ($this->_sqlo->ReadRow() ) {
            $table = $this->_sqlo->RowData[0];
            $tables[]=$table;
        }
        return $tables;
    }

    function drop_table($tablename) {
        $this->_sqlo->Exec('DROP TABLE if exists '.$tablename);
        
        // check CCT_TABLE and dlete extry in CCT_TABLE and CCT_COLUMN
        if (!gElement_exists2($this->_sqlo_main, 'CCT_TABLE', array('TABLE_NAME'=>$tablename)) ) {
            return;
        }
        
        $this->_sqlo_main->deletex('CCT_COLUMN', 'TABLE_NAME='.$this->_sqlo_main->addQuotes($tablename));
        $this->_sqlo_main->deletex('CCT_TABLE' , 'TABLE_NAME='.$this->_sqlo_main->addQuotes($tablename));
    }
    

    /**
     * 
     
     * @param array $t_info -- structure of CCT_TABLE
     * @param array $col_info -- structure of CCT_COLUMN
     *   'COLUMN_NAME'
     *   'APP_DATA_TYPE' => NAME of  data type
     *   'PRIMARY_KEY'
     */
    function create_table(array $t_info, array $col_info) {
        
        $table = $t_info['TABLE_NAME'];
        if ($table==NULL) {
            throw new Exception('Input TABLE_NAME missing.');
        }
        
        $sql_cmd = 'CREATE TABLE IF NOT EXISTS '.$table.' (';
        $pk_arr = array();
        foreach($col_info as $row) {
            
            $col_name=$row['COLUMN_NAME'];
            if ($row['PRIMARY_KEY']>0) $pk_arr[] = $col_name;
            
            $sqlite_data_type=self::APP_DATA_TYPE_2sqlite[$row['APP_DATA_TYPE']];
            if ($sqlite_data_type==NULL) {
                throw new Exception('No DATA_TYPE for "'.$row['APP_DATA_TYPE'].'" known..');
            }
            $sql_cmd .= $col_name.'  '.$sqlite_data_type.',';
        }
             
        if(empty($pk_arr)) {
            throw new Exception('No PRIMARY_KEY given.');
        }
        $sql_cmd .= ' PRIMARY KEY ('.implode(', ',$pk_arr).') '."\n".
            ' ) ';
        
        $this->_sqlo->Exec($sql_cmd);
        
        $x_argu=array(
            'TABLE_NAME'=>$table 
        );
        $xopt=array(
           'prim_name' => 'TABLE_NAME'
        );
        insert_row_s( $this->_sqlo, 'X_TAB_META', $x_argu, $xopt);
        
       
    }
    
    /**
     * register this TABLE in CCT_TABLE, so the user can use the standard LIST-VIEW tool
     
     * @param array $t_info
     * @param array $col_info
     */
    public function create_CCT_struct(array $t_info, array $col_info) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $sqlo_main = &$this->_sqlo_main;
        $user_table = $t_info['TABLE_NAME'];
        if ($t_info['NICE_NAME']==NULL) $t_info['NICE_NAME'] = $t_info['TABLE_NAME'];
        if ($t_info['TABLE_TYPE']==NULL) $t_info['TABLE_TYPE'] = 'SYS';
        
        insertRawC::insertR($sqlo_main, 'CCT_TABLE', $t_info, 'TABLE_NAME');
        
        $dyn_args=array(
            'TABLE_NAME'=> $user_table,
            'KEY'  =>'FOREIGN_DB', 
            'VALUE'=>self::DB_TMP_CONFIG_KEY
        );
        insertRawC::insertR($sqlo_main, 'CCT_TABLE_DYN', $dyn_args, 'TABLE_NAME');
        
        $def_vals=array(
            'MOST_IMP_COL'=>0,
            'PRIMARY_KEY'=>0,
            'VISIBLE'=>1
        );
        
        // 
        $pos=1;
        foreach($col_info as $row) {
            
            $col_row=array();
            $col_row['TABLE_NAME'] =$user_table;
            $col_row['COLUMN_NAME']=$row['COLUMN_NAME'];
            $col_row['POS']=$pos;
            $col_row['NICE_NAME']  =$row['NICE_NAME'];
            
            debugOut('(185) COLUMN: '.$col_row['COLUMN_NAME'], $FUNCNAME, 2);
            
            foreach($def_vals as $col_key=>$def_val) {
                if(array_key_exists($col_key, $row)) {
                    $col_row[$col_key] = $row[$col_key];
                } else {
                    $col_row[$col_key] = $def_val;
                }
            }
            if ($col_row['NICE_NAME']==NULL) $col_row['NICE_NAME']=$row['COLUMN_NAME'];
            
            if (array_key_exists('APP_DATA_TYPE', $row)) {
                $app_id = glob_elementDataGet( $sqlo_main, 'APP_DATA_TYPE', 'NAME', $row['APP_DATA_TYPE'], 'APP_DATA_TYPE_ID');
                $col_row['APP_DATA_TYPE_ID'] = $app_id;
            }
            // echo "DDDX:create_CCT_struct: ".print_r($col_row,1)."<br>";
            
            insertRawC::insertR($sqlo_main, 'CCT_COLUMN', $col_row, 'TABLE_NAME');
            $pos++;
        }
        
        if ($error->Got(READONLY))  {
            $error->set( $FUNCNAME, 1, 'Error on meta-data creation for table '.$user_table );
            return;
        }
        
        // update app cache ...
        $display=0;
        get_cache(1, $display);
        if ($error->got(CCT_ERROR_READONLY)) {
            $error->set( $FUNCNAME, 10, 'Error on meta-data creation for table '.$user_table );
            return;
        } else {
            // reset warnings ...
            $error->reset();
        }
    }
}