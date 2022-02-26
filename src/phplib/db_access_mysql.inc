<?php
/*
 * called by db_access.inc
 * class CDBMySQL => interface to mysql
 * @package db_access_mysql.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/phplib/db_access_mysql.inc 59 2018-11-21 09:04:09Z $
 */
class CDBMySQL extends CDBAbstract {
	
	var $_CLASSNAME = 'CDBMySQL';
	
	function __construct () {
		//$this->Open ($host, $user, $pass);
		$this->_init();
	}
	
	/**
	 * (non-PHPdoc)
	 * @see CDBAbstract::Open()
	 */
	function Open ($host, $user, $pass, $db = "", $autocommit = true) {
		$this->_db_linkid = mysql_connect ($host, $user, $pass);
		if ($db != "")
			$this->SelectDB($db);

		if (!$this->_db_linkid) { // Check for errors
			$error_hash = mysql_error ();
			$this->_errorSet("db_access", 1, "Error on logon to MySql-DBMS for $user@$host (".$error_hash.")");
		}
	}
	function Close () {
		@mysql_free_result($this->_db_qresult);
		return mysql_close ($this->_db_linkid);
	}
	function SelectDB ($dbname) {
		if (@mysql_select_db ($dbname, $this->_db_linkid) == true) {
			return 1;
		}
		else {
			return 0;
		}
	}
	function  Query ($querystr, $quid=NULL) {

		$result = mysql_query ($querystr, $this->_db_linkid);
		if ($result == 0) {
			$sqlerr = mysql_error ();
			echo "DDD:Query:$querystr ERR:".$sqlerr."<br>";
			$this->_errorSet("db_access", 1 , "OnQuery: (".$querystr."): " . $sqlerr );
            return 0;
		}
		else {
			if ($this->_db_qresult)
				@mysql_free_result($this->_db_qresult);
			$this->RowData = array();
			$this->_db_qresult = $result;
			$this->RowCount = @mysql_num_rows ($this->_db_qresult);
			if (!$this->RowCount) {
				// The query was probably an INSERT/REPLACE etc.
				$this->RowCount = 0;
			}
			return 1;
		}
	}
	function SeekRow ($row = 0) {
		if ((!mysql_data_seek ($this->_db_qresult, $row)) or ($row > $this->RowCount-1)) {
			printf ("SeekRow: Cannot seek to row %d\n", $row);
			return 0;
		}
		else {
			return 1;
		}
	}
	function ReadRow () {
		if($this->RowData = mysql_fetch_array ($this->_db_qresult)) {
			$this->NextRowNumber++;
			return 1;
		}
		else {
			return 0;
		}
	}
	function Commit () {
		return 1;
	}
	function Rollback () {
		echo "WARNING: Rollback is not supported by MySQL";
	}
	function addQuotes ($text) {
        return "'". str_replace("'", "''", $text) ."'";
    }
	function _ident () {
		return "CDBMySQL/1.4.2";
	}
}
