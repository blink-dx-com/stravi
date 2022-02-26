<?php
/*
 * called by db_access.inc
 * class CDBSqlite => interface to sqlite
 * @package db_access_sqlite.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class CDBSqlite extends CDBAbstract {
	
	
	function __construct () {
		//$this->Open ($host, $user, $pass);
		$this->_init();
	}
	
	/**
	 * (non-PHPdoc)
	 * @param string $db: db-file-name
	 * @see CDBAbstract::Open()
	 */
	function Open ($db, $user, $pass, $dummy="", $autocommit = true) {
	    //global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    if ($db==NULL) {
	        throw new Exception('Input error: no db-file given.');
	    }
	    $this->_db_linkid = new SQLite3($db);

		if (!$this->_db_linkid) { // Check for errors	
		    $this->_errorSet($FUNCNAME, 1, "Error on init of  to DBMS for db:".$db);
		}
		
		$this->_db_linkid->busyTimeout(5000);
	}
	function Close () {
		//TBD:
		return ;
	}
	
	function SelectDB ($dbfile) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    $sqliteerror=NULL;
	    if ( $this->_db_linkid->open($dbfile, 0666, $sqliteerror) == true) {
			return 1;
		}
		else {
		    $this->_errorSet($FUNCNAME, 1, "Error on init of  to DBMS for $user@$host");
			return 0;
		}
	}
	
	/**
	 * _db_qresult
	 */
	function  Query($sql, $quid = NULL) {
	    
	    if ($this->debug>0) echo "Query:DEBUG_ON: ".$sql."<br>";
	    $this->row = NULL;
	    $this->db_resource = $this->_db_linkid->query($sql);
	    if(!$this->db_resource){
	        $errorCode = $this->_db_linkid->lastErrorCode();
	        throw new Exception('QUERY:DB-ERROR: ('.$errorCode.') '.$this->_db_linkid->lastErrorMsg() ) ;
	    }
	    return 1;
	    
	}
	
	/**
	 * EXECUTE command
	 */
	function  Exec($sql) {
	    
	    if ($this->debug>0) echo "Exec:DEBUG_ON: ".$sql."<br>";
	    $result = $this->_db_linkid->exec($sql);
	    if(!$result){
	        $errorCode = $this->_db_linkid->lastErrorCode();
	        throw new Exception('EXEC:DB-ERROR: ('.$errorCode.') '.$this->_db_linkid->lastErrorMsg() ) ;
	    }
	    return 1;
	    
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
	 * TBD: should return value of new PK
	 * @param string $table
	 * @param string $querystr
	 * @param string $pkey_name
	 */
	function QueryInsert($table, $querystr, $pkey_name) {
	    
	    $sql    = "insert into ".$table." ". $querystr;
	    $this->Exec($sql);
	    $newval=0;
	    
	    return $newval;
	    
	}
	
	function SeekRow ($row = 0) {	
		return 0;		
	}
	
	/**
	 * returns an array indexed by column name as returned in the corresponding result set 
	 */
	function ReadArray () {
	    
	    $answer = 0;
	    if ( $this->RowData = $this->db_resource->fetchArray(SQLITE3_ASSOC) ) {
	        $answer = 1;
	    } else {
	        $answer = 0;
	    }
	    return $answer;
	}
	
	/**
	 * returns an array indexed by column number as returned in the corresponding result set, starting at column 0 
	 */
	function ReadRow () {
	    
	    $answer = 0;
	    if ( $this->RowData = $this->db_resource->fetchArray(SQLITE3_NUM) ) {
	        $answer = 1;
	    } else {
	        $answer = 0;
	    }
	    return $answer;
	}
	
	function InsertRow($table, $argu) {
	    
	    $sql_cols  = '';
	    $sql_vals  = '';
	    
	    $komma=NULL;
	    
	    
	    foreach($argu as $xcol =>$xval) {
	        
	        $sql_vals .= $komma;
	        $sql_cols .= $komma;
	        
	        $sql_cols .= $xcol;
	        if ( $xval === "") $tmpvalSql = 'NULL';
	        else {
	            $tmpvalSql = $this->addQuotes($xval);
	        }
	        
	        $sql_vals .= $tmpvalSql;
	        $komma=', ';
	        
	    }
	    
	    
	    $sqls = 'insert into '.$table.' ('.$sql_cols.') VALUES ('.$sql_vals.')';
	    
	    $ret = $this->Exec($sqls);
	    if(!$ret) {
	        throw new Exception('DB-ERROR: '.$this->_db_linkid->lastErrorMsg() ) ;
	    }
	    
	    return 1;
	}
	
	function Commit () {
		return 1;
	}
	function Rollback () {
		echo "WARNING: Rollback is not supported by Sqlite.";
	}
	function addQuotes ($text) {
        return "'". str_replace("'", "''", $text) ."'";
    }
    function Sql2DateString ( $date_column,  $format=0) {
        return $date_column;
    }
    function strToUpper($stringx) {
        return 'UPPER('.$stringx.')';
    }
    
	function _ident () {
		return "CDBSqlite/1.0";
	}
}
