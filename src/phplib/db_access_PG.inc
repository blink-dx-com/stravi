<?php

/** 
 *
 * CDB_PG => Simple Database Abstraction Layer 1.5 [lib.sdba.php] 
 *
 *  PostgreSQL accession layer
 *   by Gabe Bauman <gabeb@canada.com>
 *  Wednesday, April 05, 2000
 *  extended by Michael Howitz
 *  extended by Steffen Kube 15.03.2019
 */
class CDB_PG extends CDBAbstract
{
    
    var $_CLASSNAME = 'CDB_PG';
    var $_ERR_FUNC_ID = 'db_access';
    
    function __construct()
    { 
        $this->_init();
    }


    function Open($dsn, $user, $pass, $db = "", $autocommit = true) {
        
       $this->_db_linkid   = pg_connect("host=localhost dbname=$dsn user=$user password=$pass");
       if ( !$this->_db_linkid ) {   
           $this->_errorSet(__CLASS__, 1, "Error on logon to CDB_PG for $user@$dsn (".pg_last_error().")");
           return;
       }
        
       $this->_auto_commit = $autocommit;
	   //    $this->user         = $user;
	   $this->_sql_log = & SQL_log::getHandle();
    }


    function Close()
    { if ($this->_db_qresult)
        { if ( !$this->_auto_commit ) $this->Commit(); // otherwise all pending transactions are rolled back.      
        pg_freeresult($this->_db_qresult);
        }

        if ( is_object($this->_db_linkid) ) {
            pg_close($this->_db_linkid) or die ("Error in method CDB_PG.Close().\nProgram exited.\n");
        }
    }


    function SelectDB($dbname) { 
        die("CDB_PG does not support SelectDB");
        
    }

    /**
     * overwrite standard method
     * @see CDBAbstract::SetAutoCommit()
     */
    function SetAutoCommit ($autocommit) {
        
        $old_autocommit     = $this->_auto_commit;
        $this->_auto_commit = $autocommit;
        if ($autocommit>0) {
            // nothing
        } else {
            $this->Query('BEGIN');
        }
        
        return $old_autocommit;
    }
    
    /**
     *  This function starts a new SQL query.
        '1' is returned on success.
     * @see CDBAbstract::Query()
     */
    function Query($querystr, $quid=NULL)
    {   
        //global $error;
        //$FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $log = &$this->_sql_log;

        ## Free previous query (open cursor):
        if ( $this->_db_qresult ) { pg_freeresult($this->_db_qresult); } 

        ## In order to start a NEW query, reset all query variables:
        $this->_db_qstring    = $querystr;
        $this->_db_qerror     = '';
        $this->RowData        = array();
        $this->NextRowNumber  = 0;
        $this->RowCount       = 0;
        //$stmtname='my_query';
        
        /*
        if ( !pg_prepare($this->_db_linkid, $stmtname, $this->_db_qstring) ) {
            $this->_db_qerror = pg_errormessage($this->_db_linkid);
            $this->_errorSet("db_access", 2 , "OnParse: ('$this->_db_qstring'): " . $this->_db_qerror);
            return 0;
        }
        */

        $this->_db_qresult = pg_query($this->_db_linkid, $this->_db_qstring);
        $log->write_queryline($this);

        if ( $this->_db_qresult )
            { ## Query executed successsfully:
                $this->RowCount = pg_numrows($this->_db_qresult);
                $log->write_querylineEnd($this);
                return 1;
            }##End else
        else
            { ## An error occured:
                $this->_db_qerror = pg_errormessage($this->_db_linkid);
                $log->write_errorline($this);
                
                $this->_errorSet($this->_ERR_FUNC_ID, 2, "OnExecute ('$this->_db_qstring'): " . $this->_db_qerror);
                return 0;

                
            }

    }
  

    /**
     * Insert and return 'primary key' of the new entry.
     * This is implemented in analogy to method $this->Query():
     * @param string $table_name
     * @param string $querystr
     * @param string $pkey_name
     * @return int
     */
    function queryInsert( $table_name, $querystr, $pkey_name )
    {
        global $error;
        //$FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $log = &$this->_sql_log;
        $this->_db_qstring    = "insert into ".$table_name." ". $querystr . " RETURNING $pkey_name"; 
       
        $this->Query( $this->_db_qstring );
        if ($error->Got(READONLY))  {
            return 0;
        }
        
        $row_data_hash = pg_fetch_array($this->_db_qresult);
        $pkey_id = $row_data_hash[0];
      
        if ( $pkey_id  ){ 
            return $pkey_id; 
        } else   {
            $this->_db_qerror = pg_errormessage($this->_db_linkid);
            $log->write_errorline($this);
            $this->_errorSet($this->_ERR_FUNC_ID, 2, "OnExecute ('$this->_db_qstring'): " . $this->_db_qerror);

            return 0;
         }
    }


    function SeekRow ($row = 0)
    { $this->NextRowNumber = $row; }
     

    function ReadRow() {
        if ( ! $this->_db_qresult )                    { return 0; }
        if ( $this->NextRowNumber >= $this->RowCount ) { return 0; }
    
        $row_data_list = pg_fetch_row($this->_db_qresult, $this->NextRowNumber);
        if ( $row_data_list != FALSE )
            { $this->RowData = $row_data_list;
            $this->NextRowNumber++;
            return 1;
            }##End if
    
        
        die("CDB_PG.ReadRow() failed");
    
        return 0;
    }##End function
     

    function ReadArray() {
        if ( ! $this->_db_qresult )                    { return 0; }
        if ( $this->NextRowNumber >= $this->RowCount ) { return 0; }
    
        $row_data_hash = pg_fetch_array($this->_db_qresult, $this->NextRowNumber, PGSQL_ASSOC);
    
        if ( $row_data_hash != FALSE ) {
            
            $this->RowData = NULL;
            if (is_array($row_data_hash)) {
                
                // make column names to UPPERCASE ...
                foreach($row_data_hash as $key => $val) {
                    $this->RowData[strtoupper($key)] = $val;
                }
            }
            
            
            $this->NextRowNumber++;
            return 1;
            
        } else { 
           
            die("CDB_PG.ReadArray() failed");
            return 0;
        }##End else
                
    }##End function

    function FetchArray(&$result_array)
    {
        # descr: reads the whole result of a query into an array
        # return: number of rows in array
        #         writes into $result_array: name_of_col => array (row_number => row_value);
        # error: none

        die ("CDB_PG.FetchArray() is not yet implemented.");
    }##End function
 
    function ReadRowTest()
    { $column_name_list = array();

    for( $i=0; $i<sizeof($this->RowData); $i++ )
        { $column_name_list[] = strtoupper( pg_fieldname($this->_db_qresult, $i) ); }
  
    return $column_name_list;
    }##End function


    function GetColumnType()
    { $column_type_list = array();

	for( $i=0; $i<sizeof($this->RowData); $i++ )
        { $column_type_list[] = pg_fieldtype($this->_db_qresult, $i); }

	return $column_type_list;
    }##End function

    function ColumnNames() {
        
        $name_list = array();
        
        if ( ! $this->_db_qresult )   { 
            return $name_list; 
        }
        
        $i = pg_num_fields($this->_db_qresult);
        for ($j = 0; $j < $i; $j++) {
            
            $fieldname = pg_field_name($this->_db_qresult, $j);
            $name_list[] = $fieldname;
        }
        
        return $name_list;
        
    }

    function GetColumnTypeArray() 
# descr: returns an array ( <column_name> => <column_type>)
    {
        die ("CDB_PG.GetColumnTypeArray() is not yet implemented.");
    }##End function

  
    function Commit()
    { ## This is just a wrapper for calling  method $this->Query():
        return $this->Query('commit');
    }##End function


    function Rollback()
    { ## This is just a wrapper for calling  method $this->Query():
        return $this->Query('rollback');
    }##End function

    function addQuotes ($text) {
        return "'". str_replace("'", "''", $text) ."'";
    }
    function BulkInsert ($table_name, $field_name, $field_type, $field_data, $field_fix, $objid=0) {
        die("function BulkInsert() not yet implemented for Postgres!");
    }
    function Timestamp2Sql ($timestamp=0) {
        if ($timestamp == 0) $timestamp = time();
        
        return "TO_TIMESTAMP('".adodb_date(PARTISAN_DATETIME_USER_OUTPUT_FORMAT_PHP_DATE, $timestamp)."', ".ORACLE_PARTISAN_DATETIME_USER_OUTPUT_FORMAT.')';
    }
    
    function Sql2DateString (      $date_column,    $format=0) {
        
        
        switch($format) {
            case 1:
                return 'TO_CHAR('.$date_column.', '.ORACLE_PARTISAN_DATETIME_EXCHANGE_FORMAT.')';
            case 2:
                return 'TO_CHAR('.$date_column.', '.ORACLE_PARTISAN_DATETIME_USER_INPUT_FORMAT.')';
            case 3:
                return 'TO_CHAR('.$date_column.', '.ORACLE_DATETIME_USER_GET_F2.')';
            case 4:
                return 'TO_CHAR('.$date_column.', '.ORACLE_DATETIME_MINUTE_FORMAT.')';
            case 5:
                return 'TO_CHAR('.$date_column.', '.ORACLE_PARTISAN_DATETIME_DATE.')';
            default:
                return 'TO_CHAR('.$date_column.', '.ORACLE_PARTISAN_DATETIME_USER_OUTPUT_FORMAT.')';
        }
    
    }
    
    function strToUpper($stringx) {
        return 'UPPER('.$stringx.')';
    }
    
    /**
     * returns array( COLUMN_TERM, COLLATE_TERM)
     * {@inheritDoc}
     * @see CDBAbstract::strOrderNatural()
     */
    function strOrderNatural ($col_name) {
        return array(
            "NOT starts_with(".$col_name.", '.'), ".$col_name,
            ' COLLATE "en_US"'
        );
    }
    /**
     * returns string; can only be used if only ONE column to sort ...
     * {@inheritDoc}
     * @see CDBAbstract::strOrderNatural()
     */
    function strOrderNaturalCompact ($col_name) {
        return  "NOT starts_with(".$col_name.", '.'), ".$col_name.' COLLATE "en_US"';
    }
    
    /**
     * 
     *  convert date string of defined format (e.g. 'YYYY-MM-DD HH24:MI:SS') to sql-date (for insert/update)
        @param date   ... date (which fulfils format) to be written to db
                format ... default: for user_output_format
                           1 for partisan_exchange_format
                           2 for user_input_format
     */
    function DateString2Sql (      $date,    $format=0) {

        switch($format) {
            case 1:
                return "TO_TIMESTAMP('".$date."', ".ORACLE_PARTISAN_DATETIME_EXCHANGE_FORMAT.')';
            case 2:
                return "TO_TIMESTAMP('".$date."', ".ORACLE_PARTISAN_DATETIME_USER_INPUT_FORMAT.')';
            case 4:
                return "TO_TIMESTAMP('".$date."', ".ORACLE_DATETIME_MINUTE_FORMAT.')';
                
            default:
                return "TO_TIMESTAMP('".$date."', ".ORACLE_PARTISAN_DATETIME_USER_OUTPUT_FORMAT.')';
        }
    }
    
    function get_transaction_status() {
        return pg_transaction_status($this->_db_linkid);
    }
    
    function _ident() {
        return "CDB_PG/1.5.4"; 
    }
}##End Class