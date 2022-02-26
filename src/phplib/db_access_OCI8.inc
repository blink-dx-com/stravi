<?php
/*
 **
 ** CDB_OCI8 for oracle 8++ - databases
 **
 */
class CDB_OCI8 extends CDBAbstract {
    
    var $_CLASSNAME = 'CDB_OCI8';
    
    function __construct()
    { // use open-method to create connection to db
        //$this->error = & ErrorHandler::get();
        $this->_init();
    }
    
    function Open($host, $user, $pass, $db = "", $autocommit = true, $connection_type = 'std') {
        # return: void
        # input: $connection_type ... type of connection ('std' = normal connection, 'new' = new connection, 'persist' = persistant connection)
        # errors:
        #	db_access 1   .. connection failed
        
        $this->_auto_commit = $autocommit;
        //
        
        switch ($connection_type) {
            case 'new':     $this->_db_linkid = ociNlogon($user, $pass, $host); break;// new connection for parallel transactions
            case 'persist': $this->_db_linkid = ociPlogon($user, $pass, $host); break;// persistant connection for xml-rpc
            case 'std':     // no error for unknown connection type
            default:        $this->_db_linkid = ocilogon($user, $pass, $host);
        }
        
        if (!$this->_db_linkid) { // Check for errors
            $error_hash = OCIError();
            $this->_errorSet("db_access", 1, "Error on logon to Oracle-DBMS for $user@$host (".$error_hash['message'].")");
        }
    }##End function
    
    function Close() {
        # return: void
        # errors: none
        $log = &$this->_sql_log;
        $log->write_closeSqlHandle( $this );
        if ($this->_db_qresult)
            OCIFreeStatement($this->_db_qresult);
            OCILogOff($this->_db_linkid);
    }
    
    function SelectDB($dbname) {
        // CDB_OCI8 does not support SelectDB, so do nothing
        return true;
    }
    
    
    
    
    
    function _OCI_error_ana( $info=NULL ) {
        $log = &$this->_sql_log;
        $this->_errorstring();
        $log->write_errorline( $this, $info);
    }
    
    /**
     * free handle and set to 0
     */
    function _OCIFree_Result() {
        if ($this->_db_qresult) {
            OCIFreeStatement($this->_db_qresult);
            $this->_db_qresult = 0;
        }
    }
    
    
    
    
    
    
    
    
    
    /**
     * start a new sql query
     * @return int
     *   1 (on success)
     *   0 (on failure) ... but you should use the error_handler's got()-function.'
     *   2 error in query
     */
    function query (
        $querystr,   // query-string
        $quid=NULL	 // query-ID e.g. "fExpAna:get"
        ) {
            
            $log = &$this->_sql_log;
            
            // free previous query (open cursor):
            if ($this->_db_qresult)
                OCIFreeStatement($this->_db_qresult);
                
                // In order to start a NEW query, reset all query variables:
                $this->_db_qstring    = $querystr;
                $this->_db_qerror     = '';
                $this->RowData        = array();
                $this->NextRowNumber  = 0;
                $this->RowCount       = 0;
                
                $this->_sql_log->quid= $quid;
                $this->_sql_log->query_start_time = gettimeofday();
                $this->_db_qresult    = OCIParse($this->_db_linkid, $this->_db_qstring);
                
                if (!$this->_db_qresult){
                    $this->_errorSet("db_access", 2 , "OnParse: ('$this->_db_qstring'): " . $this->_db_qerror);
                    return 0;
                }
                
                $log->write_queryline($this);
                $retVal = $this->_auto_commit ? OCIExecute($this->_db_qresult, OCI_COMMIT_ON_SUCCESS) : OCIExecute($this->_db_qresult, OCI_DEFAULT);
                
                
                if ($retVal) {// Query executed successfully
                    $this->RowCount = OCIRowCount($this->_db_qresult);
                    $log->write_querylineEnd($this);
                    return 1;
                } else {
                    // an error occured:
                    $this->_OCI_error_ana();
                    $this->_errorSet("db_access", 2, "OnExecute ('$this->_db_qstring'): " . $this->_db_qerror);
                    return 0;
                }
    }
    
    /**
     * pure SELECT-query
     * @param $qselstr SQL-select without the SELECT keyword
     * @see query()
     * @example $qselstr="EXP_ID from EXP where name='kfdkdk'";
     */
    function Quesel ($qselstr, $quid=NULL ) {
        return ( $this->query("select ".$qselstr, $quid) );
    }
    
    /**
     * insert a data-row
     * @param string $table
     * @param string $querystr sql-text after: "insert into ".$table
     * @return int $pkey_name of generated row on success
     *    0 on failure
     */
    function QueryInsert($table, $querystr, $pkey_name) {
        $FUNCNAME='QueryInsert';
        $log = &$this->_sql_log;
        
        // Free previous query (open cursor):
        if ( $this->_db_qresult )
            OCIFreeStatement($this->_db_qresult);
            
            // In order to start a NEW query, reset all query variables:
            $this->_db_qstring    = "insert into ".$table." ". $querystr . " RETURNING $pkey_name INTO :retv";
            $this->_db_qerror     = '';
            $this->RowData        = array();
            $this->NextRowNumber  = 0;
            $this->RowCount       = 0;
            $pkey_id              = 0;
            
            $log->write_queryline($this);
            $this->_db_qresult = OCIParse($this->_db_linkid, $this->_db_qstring);
            
            if (!$this->_db_qresult) {
                $log->write_errorline($this);
                $this->_errorSet($FUNCNAME, 2, "OnParse ('$this->_db_qstring'): " . $this->_db_qerror);
            }
            
            OCIBindByName($this->_db_qresult, ":retv", $pkey_id, 38);
            $retVal = $this->_auto_commit ? OCIExecute($this->_db_qresult,OCI_COMMIT_ON_SUCCESS) : OCIExecute($this->_db_qresult,OCI_DEFAULT);
            
            if ($retVal) { // Query executed successsfully:
                $this->RowCount = OCIRowCount($this->_db_qresult);
                $log->write_querylineEnd($this);
                return $pkey_id;
            } else { // an error occured:
                $this->_OCI_error_ana();
                $this->_errorSet($FUNCNAME, 3, "Error in query ('$this->_db_qstring'): " . $this->_db_qerror);
            }
            return 0;
    }##End function
    
    
    function SeekRow ($row = 0) {
        die ("CDB_OCI8 does not support SeekRow");
    }
    
    /**
     * reads the next row of a query, writes the result to $this->RowData as
     * array which associates column-position in query-string to its value
     * - must call query() before
     * @return:
     *   1 .. if existing next row
     *   0 .. if end of data
     */
    function ReadRow() {
        
        if ($this->_db_qresult)   {
            if(OCIFetchInto($this->_db_qresult, $this->RowData, OCI_NUM + OCI_RETURN_NULLS)) {
                $this->NextRowNumber++;
                return 1;
            }
            return 0;
        }
    }
    
    /**
     *  reads the next row of a query, writes the result to $this->RowData as
     *   array which associates column-name in query-string to its value
     * @return:
     *   1 .. if existing next row
     *   0 .. if end of data
     */
    function ReadArray() {
        if ($this->_db_qresult) {
            if(OCIFetchInto($this->_db_qresult, $this->RowData, OCI_ASSOC + OCI_RETURN_NULLS)) {
                $this->NextRowNumber++;
                return 1;
            }
            return 0;
        }
    }
    
    
    function FetchArray(&$result_array) {
        # descr: reads the whole result of a query into an array
        # return: number of rows in array
        #         writes into $result_array: name_of_col => array (row_number => row_value);
        # error: none
        
        if ($this->_db_qresult) {
            $this->RowCount = OCIFetchStatement ($this->_db_qresult, $result_array);
            return $this->RowCount;
        }
    }
    
    function ReadRowTest() {
        $namer=array();
        foreach( $this->RowData as $th0=>$th1) {
            
            $namer[] = OCIColumnName( $this->_db_qresult , $th0+1 );
        }
        return $namer;
    }
    
    // returns an array ( <column_number> => <column_type>)
    function GetColumnType() {
        
        $column_type=array();
        foreach( $this->RowData as $th0=>$th1) {
            $column_type[]  = OCIColumnType($this->_db_qresult , $th0+1 );
        }
        return $column_type;
    }
    
    // returns an array ( <column_name> => <column_type>)
    function GetColumnTypeArray() {
        
        $ncols = OCINumCols($this->_db_qresult);
        $column_type=array();
        for ($i = 1; $i <= $ncols; $i++) {
            $column_type[OCIColumnName($this->_db_qresult, $i)] = OCIColumnType($this->_db_qresult, $i);
        }
        return $column_type;
    }
    
    function Commit() { // This is just a wrapper for calling method $this->Query(), necessary to get it to log-file
        return $this->Query('commit');
    }
    
    function Rollback() { // This is just a wrapper for calling  method $this->Query(), necessary to get it to log-file
        return $this->Query('rollback');
    }
    
    function ColumnNames() {
        
        $name_list = array();
        $ncols = OCINumCols($this->_db_qresult);
        for ( $i = 1; $i<=$ncols; $i++ ) {
            $name_list[] = OCIColumnName($this->_db_qresult,$i);
        }
        return $name_list;
        
    }
    
    
    function addQuotes ($text) {
        return "'". str_replace("'", "''", $text) ."'";
    }
    
    function strToUpper($stringx) {
        return 'UPPER('.$stringx.')';
    }
    
    # private function:
    function _errorstring () {
        # write OCI error message as single string to $this->_db_qerror:
        if (!$this->_db_qresult) return;
        $error_hash = OCIError($this->_db_qresult);
        $this->_db_qerror = $error_hash ? ('ORA-'.$error_hash['code'].': '.$error_hash['message']) : '';
    }
    
    /**
     * test BulkInsert results
     * @param string $table_name
     * @param string $objid
     * @param string $field_name
     * @param array  $field_fix - one dim array of fix data values, type: NUMBER
     array ( COL_NAME => value )
     e.g.	array ( "EXP_ID" => 278347, "CHANNEL_ID" => 3 )
     * @return array(int, string) : 0: o.k., 1: error
     */
    function _bulkCheckResults($table_name, $objid, &$field_name, $field_fix) {
        $FUNCNAME="_bulkCheckResults";
        $retval  = 0;
        $rettxt  = "";
        
        $pkname = key($field_fix);
        $fixobjid= current($field_fix);
        
        
        reset ($field_name);
        foreach( $field_name as $dummy=>$name) {
            $sqlsel = "$name from ".$table_name. " where ".$pkname."=".$fixobjid." AND $name is not null";
            $this->Quesel($sqlsel, $FUNCNAME);
            if ( !$this->ReadRow() ) {
                // error found
                $retval = 1;
                $rettxt = "col '".$name."'";
                break;
            }
        }
        reset ($field_name);
        return array($retval, $rettxt);
    }
    
    /**
     * save input array as LOG-file
     */
    function _saveInputAsLog( $FUNCNAME, $objid, $extraInfo, &$inargs ) {
        $tmpPrefix = $this->_sql_log->getGlobLogFilePre();
        $utimestamp= microtime();
        $timestamp = floor($utimestamp);
        $milliseconds = round(($utimestamp - $timestamp) * 1000);
        $timetag   = date('dHis').'_'.$milliseconds;
        $fileName  = $tmpPrefix . 'EXP.bulkinsert.'.$objid. '.'.$timetag.'.log';
        $fp = fopen( $fileName, 'w' );
        
        $allargs = array( 'funcname'=>$FUNCNAME, 'objid'=>$objid,
            'info'=>$extraInfo, 'inargs'=>&$inargs );
        
        if ($fp) {
            $retVal = fputs( $fp, serialize( $allargs ) );
            fclose( $fp );
        }
    }
    
    /**
     * Insert bulk data into $tablename
     * @param $tablename - name of the table
     * @param $field_name - list containg the name of the fields
     * @param $field_type - list containing the oracle data type of the $field_data
     (e.g. [number], varchar(255) and so on)
     * @param $field_data - 2 dimensional array containing data to be inserted.
     - OLD: If one collumn contains only
     one value tha same value will be inserted for the whole set of data.
     Values to be expanded have to be at the end of the list. After the first value
     which has to be expanded you can not add columns which contain a complete
     list of values.
     - ex: $f[0] = array(1..n)
     - ex: $f[1] = array(1..n)
     * @param $field_fix - one dim array of fix data values, type: NUMBER
     array ( COL_NAME => value )
     e.g.	array ( "EXP_ID" => 278347, "CHANNEL_ID" => 3 )
     * @param $objid
     *    - optional an object-id: added to the filed name to ensure UNIQUE FIELD-Names in the user-space
     - TBD: - define global types NUMBER once for "EXP" raw-results and norm-results ???
     - must define the global type with maximum $nr_of_queries
     * @todo how to OCIFreeStatement ???
     * @global $_SESSION["userGlob"]["g.debugLevel"]
     * @global $_SESSION["userGlob"]["g.debugKey"] : "sql:bulk"
     */
    function BulkInsert ($table_name, $field_name, $field_type, $field_data, $field_fix, $objid=0) {
        global  $error;
        $FUNCNAME="BulkInsert";
        
        $log = &$this->_sql_log;
        $error = & ErrorHandler::get();
        
        if ($_SESSION['userGlob']["g.debugKey"]=="sql:bulk" ) {
            $doDebug=1;
        }
        
        //Determine the number of fields and queries
        $nr_of_fields  = sizeof($field_name);
        $nr_of_queries = sizeof($field_data);
        $dataIsArray   = is_array($field_data);
        //$dataIsType    = gettype($field_data);
        
        $this->_db_qstring = "Meta::BulkInsert() object-id:$objid of ".$nr_of_fields." fields, ".$nr_of_queries.
        " data sets for ".$table_name;
        //to global-log-file: $error->logx( "INFO", "BulkInsert", $this->_db_qstring );
        $this->_sql_log->query_start_time = gettimeofday();
        $log->write_queryline($this);
        
        if (!$nr_of_queries OR !$dataIsArray) {
            $log->write_querylineEnd($this);
            return 0;
        }
        
        // free old handle ...
        if ( $this->_db_qresult ) {
            OCIFreeStatement($this->_db_qresult);
        }
        // Create types and collections
        $field_base = "CCTBX"; // fallback, if no $objid is given (!!! dangerous for parallel usage of this function)
        if ($objid>0) $field_base = "CCTBU".$objid."_";
        
        // TBD: error detected: when number of results==1 =>
        //		PLS-00435: DML statement without BULK In-BIND cannot be used inside FORALL
        $mycoll=array();
        for ($i=0;$i<sizeof($field_type);$i++) {
            
            //Have to create a type
            $tque="create or replace type ".$field_base."$i as varray(".$nr_of_queries.") of ".$field_type[$i];
            $tpque=OCIParse($this->_db_linkid,$tque);
            if($tpque===FALSE)
                return array(1,"Cannot parse types queries!");
                
                if(OCIExecute($tpque)===FALSE)
                    return array(2, "Cannot execute type queries !");
                    
                    if(OCIFreeStatement($tpque)===FALSE)
                        return array(3, "Cannot free statements from type queries");
                        
                        $mycoll[$i] = OCINewCollection($this->_db_linkid, $field_base . $i);
                        
                        if($mycoll[$i]===FALSE) return array(4, "Cannot create collections !");
                        
        };
        
        //Populate collections
        for($i=0;$i<$nr_of_fields;$i++) {
            
            for($j=0;$j<$nr_of_queries;$j++){
                $mycoll[$i]->append($field_data[$j][$i]);
            };
            
        }
        
        //Create bulk query
        $insert_def="";
        $insert_val="";
        $tmpkomma = "";
        
        for($i=0;$i<$nr_of_fields;$i++) {
            $insert_def.= $tmpkomma . $field_name[$i];
            $insert_val.= $tmpkomma . ":fb".$i."(i)";
            $tmpkomma = ", ";
        };
        
        foreach( $field_fix as $key=>$val) {
            $insert_def.= $tmpkomma . $key;
            $insert_val.= $tmpkomma . $val;
            $tmpkomma = ", ";
            $i++;
        }
        reset ($field_fix);
        
        $bulk_str="begin forall i in 1..".$nr_of_queries." insert into ".$table_name." (".$insert_def.
        ") values(".$insert_val.");end;";
        
        $bulk_query = OCIParse($this->_db_linkid,$bulk_str);
        if (!$bulk_query) {
            $this->_errorSet($FUNCNAME , 4, 'OCI-Parse: '.$objid .": sql:" . $bulk_str);
        } else {
            
            $this->_db_qresult = $bulk_query;
            
            if ($doDebug) $log->write_textline( "\n1:bulk_str: ".$bulk_str);
            
            //Bind Collections
            
            // for switch from PHP4 to PHP5
            if ( is_numeric(OCI_B_SQLT_NTY)) $useOciBindVar = OCI_B_SQLT_NTY;
            else $useOciBindVar = SQLT_NTY;
            
            for($i=0;$i<$nr_of_fields;$i++) {
                OCIBindByName( $bulk_query, ":fb".$i, $mycoll[$i], -1, $useOciBindVar );
            }
            
            
            if ($doDebug) {
                
                if ( $_SESSION["userGlob"]["g.debugLevel"]>1 ) {
                    $log->write_textline( "6:debug-level>3: " );
                    $log->write_textline( "field_name:". glob_array2String( $field_name, 1) );
                    $log->write_textline( "field_type:". glob_array2String( $field_type, 1) );
                    $log->write_textline( "field_data:". glob_array2String( $field_data, 1) );
                    $log->write_textline( "field_fix:".  glob_array2String( $field_fix, 1) );
                }
                
                for ($i=0;$i<sizeof($field_type);$i++) {
                    //Have to create a type
                    $tque="create or replace type ".$field_base."$i as varray(".$nr_of_queries.") of ".$field_type[$i];
                    $log->write_textline( "5:OCIPA: ". $tque );
                }
                
                
            }
            
            
            $this->_OCI_error_ana( 'object-id: '.$objid );
            if ($this->_db_qerror!=NULL ) {
                $this->_errorSet($FUNCNAME , 1, 'Prepare for object-id: '.$objid .":" . $this->_db_qerror);
            } else {
                //Execute Query
                $result    = OCIExecute($bulk_query);
                $this->_OCI_error_ana( 'object-id: '.$objid );
                
                if (!$result or $this->_db_qerror!=NULL ) {
                    
                    $this->_errorSet($FUNCNAME , 2, 'OciError for object-id: '.$objid .
                        " error:" . $this->_db_qerror. ": sql:" . $bulk_str );
                    
                    if ( strstr($_SESSION['globals']["app.logkey"], "bulkinsert.onerr") != NULL  ) {
                        $inargs    = func_get_args(); // save input data as log-file
                        $extraInfo = "error:" . $this->_db_qerror. ": sql:" . $bulk_str;
                        $this->_saveInputAsLog( $FUNCNAME, $objid, $extraInfo, $inargs );
                    }
                    
                }
            }
            $this->_OCIFree_Result();
            
            if ( strstr($_SESSION['globals']["app.logkey"], "bulkinsert.check1") != NULL ) {
                $checkres = $this->_bulkCheckResults($table_name, $objid, $field_name, $field_fix);
                if ($checkres[0]>0) {
                    $error->logx( "ERROR", 1, "<".$FUNCNAME.">"."_bulkCheckResults: table:$table_name objid:$objid ".$checkres[1]."</".$FUNCNAME.">");
                }
            }
            
        }
        
        //Free Collections
        for($i=0;$i<$nr_of_fields;$i++) {
            OCIFreeCollection($mycoll[$i]);
        }
        
        //Drop types
        for($i=0;$i<$nr_of_fields;$i++) {
            $dtque=OCIParse($this->_db_linkid,"drop type ".$field_base.$i);
            OCIExecute($dtque);
            OCIFreeStatement($dtque);
        }
        
        $log->write_querylineEnd($this);
        return 0;
    }
    
    /**
     * Normal Insert into $tablename
     * @param $tablename - name of the table
     * @param $field_name - list containg the name of the fields
     * @param $field_type - list containing the oracle data type of the fields
     *          (e.g. number, varchar(255) and so on)
     * @param $field_data - 2 dimensional array containing data to be inserted.
     *  - If one collumn contains only
     *  - one value tha same value will be inserted for the whole set of data.
     *  - Values to be expanded have to be at the end of the list. After the first value
     *     which has to be expanded you can not add columns which contain a complete
     *     list of values.
     * @param $field_fix - one dim array of fix data values, type: NUMBER
     - array ( COL_NAME => value )
     - e.g.	array ( "EXP_ID" => 278347, "CHANNEL_ID" => 3 )
     */
    function BulkInsertSlow ($table_name, $field_name, $field_type, $field_data, $field_fix ) {
        $FUNCNAME= 'BulkInsertSlow';
        
        
        //Determine the number of fields and queries
        $nr_of_fields =sizeof($field_name);
        $nr_of_queries=sizeof($field_data);
        
        //Run an insert query for each row of data from the array
        for ($my_nq=0;$my_nq<$nr_of_queries;$my_nq++){
            
            //Create bulk query
            $insert_def="";
            $insert_val="";
            $tmpkomma = "";
            
            for($i=0;$i<$nr_of_fields;$i++) {
                $insert_def.= $tmpkomma . $field_name[$i];
                $insert_val.= $tmpkomma . $field_data[$my_nq][$i];
                $tmpkomma = ", ";
            };
            
            foreach( $field_fix as $key=>$val) {
                $insert_def.= $tmpkomma . $key;
                $insert_val.= $tmpkomma . $val;
                $tmpkomma = ", ";
                $i++;
            }
            reset ($field_fix);
            
            //Generate query text
            $query_str="insert into ".$table_name." (".$insert_def.") values(".$insert_val.")";
            
            //Run the query
            $my_rv=$this->query($query_str);
            if ($this->error->got(READONLY)) {
                $this->error->set( $FUNCNAME, 1, 'insert failed.' );
                return;
            }
            
        };
        
        return $my_rv;
    }
    
    
    #
    /**
     * convert unix-timestamp to sql-date (for insert & update)
     * supports date-YEAR>2038
     * @see phplib/CDBAbstract::Timestamp2Sql()
     * @param timestamp ... unix timestamp (leave out to use the current timestamp)
     * @return string
     */
    function Timestamp2Sql (   $timestamp=0) {
        if ($timestamp == 0) $timestamp = time();
        
        return "TO_DATE('".adodb_date(PARTISAN_DATETIME_USER_OUTPUT_FORMAT_PHP_DATE, $timestamp)."', ".ORACLE_PARTISAN_DATETIME_USER_OUTPUT_FORMAT.')';
    }
    
    
    
    /**
     * convert sql-date to date string of defined format (e.g. 'YYYY-MM-DD HH24:MI:SS') (for select)
     * @param string date_column : database column wich contains the date
     * @param int format=0
     *  format      ... default: user_output_format    : YYYY-MM-DD HH24:MI:SS
     1 for partisan_exchange_format : YYYY-MM-DD HH24:MI:SS
     2 for user_input_format        : DD-MON-YY
     3 easy date 			        : DD-MM-YYYY
     4 minutes						: YYYY-MM-DD HH24:MI
     5 date						    : YYYY-MM-DD
     * @see CDBAbstract::Sql2DateString()
     */
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
    
    #   string DateString2Sql (string date, int format=0)
    function DateString2Sql (      $date,    $format=0) {
        # descr: convert date string of defined format (e.g. 'YYYY-MM-DD HH24:MI:SS') to sql-date (for insert/update)
        # input: date   ... date (which fulfils format) to be written to db
        #        format ... default: for user_output_format
        #                   1 for partisan_exchange_format
        #                   2 for user_input_format
        
        
        switch($format) {
            case 1:
                return "TO_DATE('".$date."', ".ORACLE_PARTISAN_DATETIME_EXCHANGE_FORMAT.')';
            case 2:
                return "TO_DATE('".$date."', ".ORACLE_PARTISAN_DATETIME_USER_INPUT_FORMAT.')';
            case 4:
                return "TO_DATE('".$date."', ".ORACLE_DATETIME_MINUTE_FORMAT.')';
                
            default:
                return "TO_DATE('".$date."', ".ORACLE_PARTISAN_DATETIME_USER_OUTPUT_FORMAT.')';
        }
    }
    
    function _ident () {
        return 'CDB_OCI8/1.5.4';
    }
}




