<?php

##    Simple Database Abstraction Layer 1.5 [lib.sdba.php] 
##    by Gabe Bauman <gabeb@canada.com>
##    Wednesday, April 05, 2000
##    extended by Michael Howitz
##    Thursday, Jun 15, 2000 and
##    Extracted from db_access.inc by Steffen (2005-02-22)

/**************************************************************************************************
 **
 ** CDB_ODBC
 **
 */
class CDB_ODBC extends CDBAbstract {
    function __construct($host, $user, $pass, $autocommit = true) {

        $this->Open ($host, $user, $pass, "", $autocommit);
    }

    function Open($dsn, $user, $pass, $db = "", $autocommit = true) {

        ($this->_db_linkid = odbc_connect($dsn, $user, $pass)) or die("Error while in CDB_ODBC.Open()");
        $this->_auto_commit = $autocommit;
    }

    function Close() {
        if ($this->_db_qresult) {
            if (!$this->_auto_commit)
                $this->Commit(); // otherwise all pending transactions are rolled back.      
            odbc_free_result($this->_db_qresult);
        }
        /* odbc_close($this->_db_linkid) or die ("Error while in CDB_ODBC.Close()"); */
    }

    function SelectDB($dbname) {
        echo "CDB_ODBC does not support SelectDB";
        return 0;
    }

    function Query($querystr) {
        /* echo "<font color=#FF0000>DEBUG</FONT>SQL: ". $querystr ."<BR>"; 
         */
        $result = @odbc_exec($this->_db_linkid, $querystr);
        if (!$result) 
            /* die("Error in execution of query ... ODBC did not tell which one <P>"); */
            echo "<P>DBMS ERROR <P>";
        /* return 0; */
        if ($result == 0)
            return 0; 
        else {
            if ($this->_db_qresult && !$this->_auto_commit)
                odbc_free_result($this->_db_qresult);
            $this->RowData = array();   
            $this->_db_qresult = $result;
            $this->RowCount = odbc_num_rows($this->_db_qresult);
            if (!$this->RowCount) {
                // The query was probably an INSERT/REPLACE etc. 
                $this->RowCount = 0;
            }
            $this->NextRowNumber = 1;
            return 1;
    
        }
    }
  
    function queryInsert( $query_str, $pkey_name ) {
        /* echo "<font color=#FF0000>DEBUG</FONT> INS-SQL:".$query_str." pkey_id:$pkey_name<br>"; 
         */
        $exec_res = 0;
        $pkey_id  = 0;

        $result = @odbc_prepare($this->_db_linkid, $query_str);

        if (!$result) {  
            echo "Error in INSERT:query: ".$query_str;
            return false;
        }
   

        if ($this->_auto_commit)
            $exec_res = @odbc_exec($this->_db_linkid, $query_str);
        else
            $exec_res = @odbc_exec($this->_db_linkid, $query_str);

        if (!$exec_res) {
            echo "Error in EXEC query: ". $query_str; 
            return false;
        }
    
     	$this->row_data = array();
        $this->_db_qresult = $result;
        $this->row_count = 0;
        $this->next_row_number = 0;
 
        /* for SQL ANYWHERE ONLY */
    
        $this->Query( "select @@identity" );
        $this->ReadRow();
        $pkey_id=$this->RowData[0];
        return $pkey_id;
    }

    function SeekRow ($row = 1) {
        $this->NextRowNumber = $row;
    }
     
    function ReadRow() {
        if (odbc_fetch_into($this->_db_qresult, $this->NextRowNumber, $this->RowData)) {	
            $this->NextRowNumber++; 
            return 1;
        }
        else
            return 0;
    }

    function ReadArray() {
        echo "the method CDB_ODBC.ReadArray() is untested!"; // Please test it and remove this remainder.
        $tmp_res = array();
        if (odbc_fetch_into($this->_db_qresult, $this->NextRowNumber, $tmp_res)) {	
            $this->NextRowNumber++;
            foreach($tmp_res as $col_nr => $col_val)
                $this->RowData[strtoupper(odbc_field_name($this->_db_qresult, $col_nr + 1))] = $col_val;
            return 1;
        } else {
            return 0;
        }
    }

    function FetchArray(&$result_array) {
# descr: reads the whole result of a query into an array
# return: number of rows in array
#         writes into $result_array: name_of_col => array (row_number => row_value);
# error: none

        die ("CDB_ODBC.FetchArray() is not yet implemented.");
    }
  
    function ReadRowTest() {
        while ($th = each($this->RowData)) {
            $namer[] = strtoupper( odbc_field_name( $this->_db_qresult , $th[0]+1 ));
        }
        return $namer;
    }

    function GetColumnType() {
        reset ($this->RowData);
        while ($th = each($this->RowData)) {
            $column_type[]  = odbc_field_type($this->_db_qresult , $th[0]+1 );	
            $cnt++;
        }
        return $column_type;
    }

    function GetColumnTypeArray() {
# descr: returns an array ( <column_name> => <column_type>)
        die ("CDB_ODBC.GetColumnTypeArray() is not yet implemented.");
    }

    function Commit() {
        return odbc_commit($this->_db_linkid);
    }
    function Rollback() {
        return odbc_rollback($this->_db_linkid);
    }
    function addQuotes () {
        die("function addQuotes() not yet implemeted in CDB_ODBC");
    }
    function strToUpper($stringx) {
        return 'UPPER('.$stringx.')';
    }
    function BulkInsert () {
        die("function BulkInsert() not yet implemented for ODBC!");
    }
    function Timestamp2Sql () {
        die("function Timestamp2Sql() not yet implemented for ODBC!");
    }
    function Sql2DateString () {
        die("function Sql2DateString() not yet implemented for ODBC!");
    }
    function DateString2Sql () {
        die("function DateString2Sql() not yet implemented for ODBC!");
    }
    function _ident() {
        return "CDB_ODBC/1.5.4";
    }
}



/************************************************************************************************** 
 **
 ** CDB_PG
 **
 ** PostgreSQL accession layer
 ** (Piet, 07 Mar 2002)
 **
 **
 */
class CDB_PG extends CDBAbstract
{ ## Constructor:
    function __construct($host, $user, $pass, $autocommit = true)
    { $this->Open ($host, $user, $pass, "", $autocommit); }


    function Open($dsn, $user, $pass, $db = "", $autocommit = true)
    { $this->_db_linkid   = pg_connect("host=localhost dbname=$dsn user=$user password=$pass") or die("Error in method 'CDB_PG.Open()'.\nProgram exited.\n");
    $this->_auto_commit = $autocommit;
	//    $this->user         = $user;
	$this->_sql_log = & SQL_log::getHandle();
    }##End function


    function Close()
    { if ($this->_db_qresult)
        { if ( !$this->_auto_commit ) $this->Commit(); // otherwise all pending transactions are rolled back.      
        pg_freeresult($this->_db_qresult);
        }##End if

    pg_close($this->_db_linkid) or die ("Error in method CDB_PG.Close().\nProgram exited.\n");
    }##End function


    function SelectDB($dbname)
    { die("CDB_PG does not support SelectDB");
    return 0;
    }##End function


    function Query($querystr)
    { ## This function starts a new SQL query.
## '1' is returned on success.

        $log = &$this->_sql_log;

## Free previous query (open cursor):
        if ( $this->_db_qresult ) { pg_freeresult($this->_db_qresult); } 

## In order to start a NEW query, reset all query variables:
        $this->_db_qstring    = $querystr;
        $this->_db_qerror     = '';
        $this->RowData        = array();
        $this->NextRowNumber  = 0;
        $this->RowCount       = 0;

        $this->_db_qresult = pg_exec($this->_db_linkid, $this->_db_qstring);
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
                $log-write_errorline($this);

                print "<pre>\n\n\n";
                print("Error in method CDB_PG.Query().\n");
                print_r($this);
#printf("Filename:  %s\n",zend_get_executed_filename(TSRMLS_C));
#printf("Line No:   %s\n",zend_get_executed_lineno(TSRMLS_C));
                printf("Function:  %s\n",get_active_function_name(TSRMLS_C));
                print("Program exited.\n");

## Always die if SQL query failes:
                die();

                return 0;
            }##End if 

    }##End function
  

    function queryInsert( $querystr, $pkey_name )
    { ## Insert and return 'primary key' of the new entry.
## This is implemented in analogy to method $this->Query():
        $this->Query( $querystr );

        $new_oid = pg_getlastoid($this->_db_qresult);
        if ( $new_oid < 1 )
            { ## Always die if no new primary key:
                print "<pre>\n\n\n";
                print("Error in method CDB_PG.queryInsert().\n");
                print_r($this);
                print("No OID returned.\n");
                print("Program exited.\n");
                exit;

                return 0;
            }##End if


        $table_name = preg_replace( '/_ID$/', '', strtoupper($pkey_name) );
        $this->Query("select $pkey_name from $table_name where oid=$new_oid");
        $this->ReadRow();
        $pkey_id = $this->RowData[0];

        if ( $pkey_id  )
            { return $pkey_id; }
        else
            { ## Always die if no new primary key:
                print "<pre>\n\n\n";
                print("Error in method CDB_PG.queryInsert().\n");
                print_r($this);
                print("No primary key generated.\n");
                print("Program exited.\n");
                exit;

                return 0;
            }##End if
    }##End function


    function SeekRow ($row = 0)
    { $this->NextRowNumber = $row; }
     

    function ReadRow()
    { if ( ! $this->_db_qresult )                    { return 0; }
    if ( $this->NextRowNumber >= $this->RowCount ) { return 0; }

    $row_data_list = pg_fetch_row($this->_db_qresult, $this->NextRowNumber);
    if ( $row_data_list != FALSE )
        { $this->RowData = $row_data_list;
        $this->NextRowNumber++;
        return 1;
        }##End if

    print "<pre>";
    print_r($this);
    print "</pre>\n";
    die("CDB_PG.ReadRow() failed");

    return 0;
    }##End function
     

    function ReadArray()
    { if ( ! $this->_db_qresult )                    { return 0; }
    if ( $this->NextRowNumber >= $this->RowCount ) { return 0; }

    $row_data_hash = pg_fetch_array($this->_db_qresult, $this->NextRowNumber, PGSQL_ASSOC);

    if ( $row_data_hash != FALSE )
        { $this->RowData = $row_data_hash;
        $this->NextRowNumber++;
        return 1;
        }##End if
    else
        { print "<pre>";
        print_r($this);
        print "</pre>\n";
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

    function addQuotes () {
        die("function addQuotes() not yet implemeted for Postgres");
    }
    function BulkInsert () {
        die("function BulkInsert() not yet implemented for Postgres!");
    }
    function Timestamp2Sql () {
        die("function Timestamp2Sql() not yet implemented for Postgres!");
    }
    function Sql2DateString () {
        die("function Sql2DateString() not yet implemented for Postgres!");
    }
    function DateString2Sql () {
        die("function DateString2Sql() not yet implemented for Postgres!");
    }
    function _ident() {
        return "CDB_PG/1.5.4"; 
    }
}##End Class