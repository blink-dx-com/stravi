<?php
/**
 * Simple Database Abstraction Layer 1.5 [lib.sdba.php] 
 * Easy way to read and write to any database.
 * supports:
 *  - CDB_OCI8
 *  - CDB_PG    [default]
 *  - CDBMySQL
 *  - CDBSqlite
 * <pre>
 * DEBUG: set $_SESSION['userGlob']["g.debugLevel"] : 0,1,2,3, ...
		$_SESSION['userGlob']["g.debugKey"]   : 
			"sql:bulk"   : bulk-action debug
			"sql:disp"   : display SQL-command
			"sql:syspid" : log the HTTP-process PID and memory usage
			"sql:functrc : log the php-function trace for SQL-commands
  </pre>
 * 
 * @since   April 05, 2000
 * @package db_access.inc
 * @swreq   SREQ:0000477: DB.management > create a database-handler class (db_access HOME) 
 * @author  Gabe Bauman <gabeb@canada.com>
 * @author  Michael Howitz <icemac@gmx.net>
 * @author  Steffen Kube <steffen@blink-dx.com>

 * @example <pre>
 * $sql  = logon2(  );
 *   // or
 * $sql = logon_to_db( $user, $passwd, $db, $_dbtype);
 * $sql->Query('SELECT lastname, firstname FROM people');
 * while ($sql->ReadArray()) { 
 *   echo $sql->RowData['LASTNAME']; 
 * }
 * $sql->close(); // not really necessary
 * </pre>
 * 
 * @throws exception  caller='db_access' <pre>
 *	 1 .. connection failed
 *   2 .. error in query
 * </pre>
 * @deprecated classes, see old code in db_access_other.inc <pre>
 * 	class CDB_ODBC
 * </pre>
 */


require_once('ErrorHandler.inc');
require_once('get_cache.inc');
require_once('ots/adodb-time.inc.php');

// until now these two following formats are equal beacuse tye do not consider time zone aspects!
define('ORACLE_PARTISAN_DATETIME_EXCHANGE_FORMAT',      "'YYYY-MM-DD HH24:MI:SS'");
define('ORACLE_PARTISAN_DATETIME_USER_OUTPUT_FORMAT',   "'YYYY-MM-DD HH24:MI:SS'");
define('ORACLE_DATETIME_MINUTE_FORMAT',      		    "'YYYY-MM-DD HH24:MI'");
define('ORACLE_PARTISAN_DATETIME_DATE',      			"'YYYY-MM-DD'");
define('PARTISAN_DATETIME_USER_OUTPUT_FORMAT_PHP_DATE', 'Y-m-d H:i:s');
define('ORACLE_PARTISAN_DATETIME_USER_INPUT_FORMAT',    "'DD-MON-YY'");
define('ORACLE_DATETIME_USER_GET_F2',                   "'DD-MM-YYYY'" );
define('PARTISAN_DATETIME_USER_INPUT_FORMAT_PHP_DATE',  'd-M-y');
define('MATH_1d1000000',  0.000001);    // 1/ 1e6
// the versions of the date format (*_FORMAT and *_FORMAT_PHP_DATE) must be equivalent
// (the first one for the db-to-date-function the second one for the php-date-function)


/**
 *
 * abstract layer CDBAbstract
 *
 */
class CDBAbstract {
    var $_db_linkid    = 0; 
    var $_db_qresult   = 0;
    var $_db_qstring   = '';	// default: the SQL string for logging, but also used for PARSE in function  query()
    var $_db_qerror    = '';
    var $_auto_commit  = true;
    var $RowData       = array(); 
    var $NextRowNumber = 0; 
    var $RowCount      = 0;
    var $error         = false; // handle of the error_handler
    var $_sql_log      = false; // handle of the sql_logger-class
   
   
    function __construct () {
        die ("CDBAbstract: Do not create instances of CDBAbstract! Use a subclass.");
    }
    function Open ($host, $user, $pass, $db = "", $autocommit = true) {}
    function Close () {}
    function SelectDB ($dbname) {}
    function Query  ($querystr, $quid=NULL ) { throw new Exception('Not implemented!');  }
    function Quesel ($qselstr, $quid=NULL ) {
        // pure SELECT-query
        return ( $this->query("select ".$qselstr) );
    }
       
	         
    function Updatex (
        $table,     	// the table
        $updatestr,     // string after "SET "
        $tabAlias=NULL, // can be alias for the table e.g. "x"
        $quid=NULL	    // query-ID e.g. "fExpAna::get"
        ) {
            // FUNCTION: special function to control/check a wild update
            $retval = $this->query ("update ".$table." SET ".$updatestr, "accux:".$quid);
            return ($retval);
    } 
	
	function Deletex (   // do a wild "DELETE"-query, $wherestr contains text after "where "
		    $table,     	// the table
		    $wherestr, 		// string after "SET "
		    $tabAlias=NULL, // can be alias for the table e.g. "x"
		    $quid=NULL	    // query-ID e.g. "fExpAna::get"
		    ) {
		        // FUNCTION: - special function to control/check a wild delete
		        //		     - TBD: 20080911: still 106 wild "delete" actions in Partisan-code
		        //				    detected, which do not use Deletex()
		        
		        $retval = $this->query ("delete from ".$table." ".$tabAlias." where ".$wherestr, "accdx:".$quid);
		        return ($retval);
		}
		
	function Insertx (   // do a wild "UPDATE"-query, $updatestr contains text after "update "
		    $table,     	// the table
		    $colstr, 		// string after "insert into TABLE "
		    $valstr,
		    $quid=NULL
		    ) {
		        $retval = $this->query ("insert into ".$table." (".$colstr.") values(".$valstr.")", "accix:".$quid);
		        return ($retval);
	}
	
	/**
	 * do easy SQL-insert from argument array
	 * - only for system-intern usage !
	 */
	function InsertSysArr( $tablename, &$argu ) {
	    $komma=NULL;
	    $sqls =NULL;
	    $sql_cols=NULL;
	    
	    foreach( $argu as $xcol=>$xval) {
	        
	        $sql_cols .= $komma . $xcol;
	        if ( $xval == "") $tmpvalSql = 'NULL';
	        else {
	            $tmpvalSql = $this->addQuotes($xval);
	        }
	        $sqls .= $komma. $tmpvalSql;
	        $komma=', ';
	    }

	    $sqls = 'insert into '.$tablename.' ('.$sql_cols.') VALUES ('.$sqls.')';
	    $retval = $this->query ($sqls);
	    return ($retval);
	}
		
	function strOrderNatural ($col_name) {throw new Exception('Not implemented!'); }
	function SeekRow ($row = 0) {throw new Exception('Not implemented!'); }
    function ReadRow ()         {throw new Exception('Not implemented!'); }   // reads one row of the result into an zero-based array
    function ReadArray()        {throw new Exception('Not implemented!'); }  // reads one row of the result into an associative array having the column names as keys
    function FetchArray(&$result_array) {throw new Exception('Not implemented!'); } // reads the whole result of a query into an array
    function GetColumnTypeArray() {throw new Exception('Not implemented!'); } // get array ('column_name' => 'column_type')
    function Commit ()          {throw new Exception('Not implemented!'); }
    function Rollback ()        {throw new Exception('Not implemented!'); }
    function addQuotes ($text)  {throw new Exception('Not implemented!'); }
    function strToUpper($stringx) {throw new Exception('Not implemented!');  }
    function BulkInsert ($table_name, $field_name, $field_type, $field_data, $field_fix, $objid=0) {} // Insert data using the Oracle Bulk Insert technology
    function Timestamp2Sql ()   {throw new Exception('Not implemented!');} // convert unix-timestamp to sql-date (for insert & update)
    function Sql2DateString ( $date_column,  $format=0) {throw new Exception('Not implemented!'); } // convert sql-date to date string of defined format (e.g. 'YYYY-MM-DD HH24:MI:SS') (for select)
    function DateString2Sql ($date,    $format=0)       {throw new Exception('Not implemented!'); } // convert date string of defined format (e.g. 'YYYY-MM-DD HH24:MI:SS') to sql-date (for insert/update)
    function SetAutoCommit ($autocommit) {
        $this->_sql_log->query_start_time = gettimeofday();
        $old_autocommit     = $this->_auto_commit;
        $this->_db_qstring  = $autocommit ? 'autocommit set to on' : 'autocommit set to off';
        $this->_auto_commit = $autocommit;

        $this->_sql_log->write_queryline($this, 1);
        return $old_autocommit;
    }
    function _ident () { 
        return "CDBAbstract/1.5.4";
    }
    
    function _init() {
    	$this->error = & ErrorHandler::get();
    	date_default_timezone_set('Europe/Berlin'); // import, if we use gettimeofday() anywhere in PARTISAN
    	$this->_sql_log     = &SQL_log::getHandle();
    }
    
    /**
     * set Error logging ON/OFF
     * @param int $boolVal 0,1
     */
    function setErrLogging($boolVal) {
    	$this->error->doLog($boolVal);
    }
    
    /**
     * - set error
     * - log error-stack in global log
     */
    function _errorSet($func, $errno, $text, $prio=NULL) {
    	 
    	if (!$prio) $prio = 1; // default: a severe error !
    	$err = &$this->error;
    	$err->set($func, $errno , $text);
    	 
    	if ( $err->isLog() )  {
    		$log = &$this->_sql_log;
    		$funcstack=$log->_getFuncStack(5); // 5 call levels deep
    		$err->logx('INTERROR', $prio, $this->_CLASSNAME.':'.$func.' '.$text.':'.$funcstack);
    	}
    }
}


/*
 * connect
 * @param $user
 * @param $passwd
 * @param $db -DB-server
 * @param $_dbtype
 * @param $connection_type ...
 type of connection (
 'std' = normal connection,
 'new' = new connection,
 'persist' = persistant connection )
 @param $use_cache
 @dbname string name of DB
 */
function logon_to_db( $user, $passwd, $db, $_dbtype=NULL, $connection_type = 'std', $use_cache=true, $dbname=NULL)
{
    global $error;
    $FUNCNAME='logon_to_db';
    
    $error = & ErrorHandler::get();
    $conn  = false;
    if ($use_cache) { // this should only be false when initializing cache for the first time
        get_cache(0,0);
        $error->resetWarnings(); // remove all warnings because they are not shown to user here
        if ($error->got(CCT_ERROR_READONLY)) {
            if ($error->got(CCT_ERROR, 'init_cache', 7)) { // got error because of incompatibitity of cache with partisan version
                $error->getLast('init_cache', 7); // remove error from stack
                get_cache(1,0); // handle error
                $error->resetWarnings(); // remove all warnings because they are not shown to user here
            }
        }
        if ($error->printAll()) {
            die($FUNCNAME.": cache error");
        }
    }
    $errmsg = NULL;
    
    if ( $db==NULL ) {
        die ( "Error: Either you did not allow cookies (necessary for running Partisan, so please turn them on and retry)<br>" .
            "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;or your session has expired because you did nothing for " .
            "a long time. (In this case you have to login again.)" );
    }
    // create a query object for the requested type of database:
    
    if ( $_dbtype==NULL ) $_dbtype='CDB_PG';
    switch ($_dbtype) {
        case "CDB_OCI8":
            require_once('db_access_OCI8.inc');
            $conn = new CDB_OCI8();
            $conn->open($db, $user, $passwd, '', true, $connection_type);
            if ($error->got(CCT_ERROR_READONLY)) $conn = false;
            return $conn;
        case "CDBSqlite":
            require_once('db_access_sqlite.inc');
            $conn = new CDBSqlite();
            $conn->open($db, $user, $passwd);
            if ($error->got(CCT_ERROR_READONLY)) $conn = false;
            return $conn;
            break;
        case "CDB_PG":
            require_once('db_access_PG.inc');
            $conn = new CDB_PG();
            $conn->open($db, $user, $passwd, $dbname, true, $connection_type);
            if ($error->got(CCT_ERROR_READONLY)) $conn = false;
            return $conn;
            break;
        case "CDBMySQL":
            require_once('db_access_mysql.inc');
            $conn = new CDBMySQL();
            $conn->open($db, $user, $passwd, $dbname, true, $connection_type);
            if ($error->got(CCT_ERROR_READONLY)) $conn = false;
            return $conn;
            
        default:
            $errmsg = 'unknown';
    }
    if ( $errmsg ) die("Error: $_dbtype ". $errmsg);
}

/**
 * common database-connection-login
 * - if not logged in, tries to login and gives $callURL as destination
 * @param string $callURL original Script-URL
 * @param string $toRoot  relative path to the web-root (e.g. from "lab" => "../pionir/" )
 */
function logon2($callURL=NULL,$toRoot =NULL) {
    
    if ( ($_SESSION['sec']['db']=="") AND ($callURL!="") ) { // not registered? , try login
        // no session active !
        // try login
        $forwardUrl = urlencode($callURL);
        $newurl     = $toRoot."index.php?forwardUrl=".$forwardUrl;
        if ($_REQUEST["dbid"]!="")  $newurl .= "&dbid=".$_REQUEST["dbid"];
        echo "<html>\n&nbsp;&nbsp;[<a href=\"".$newurl."\">forwarded page &gt;&gt;</a>]\n";
        
        echo '<script language="JavaScript">'."\n";
        echo '	location.replace("'.$newurl.'"); '."\n";
        echo '</script>'."\n";
        
        exit();
        
    }
    
    return ( logon_to_db($_SESSION['sec']['dbuser'], $_SESSION['sec']['passwd'], $_SESSION['sec']['db'], $_SESSION['sec']['_dbtype']) );
}


/*
 **
 ** Class SQL_log: manage SQL_USER_LOG and SQL_GLOBLOG
 **
 ** Opens a file for logging SQL query strings.
 ** SQL strings and other data are extracted from 
 ** a SQL query object.
 ** 
 ** There should be only one instance of this class (singleton).
 ** You get this instance calling: $log = & SQL_log::getHandle();
 ** for format description: see 89_1001_SDS_01.pdf or Ticket SREQ:0002552
 **
 ** (Piet,Mac 25 Jun 2002)
 * @swreq SREQ:0002552: DB.management > create a LOG-class (class SQL_log) 
 */
class SQL_log {
## Private variables:
    var $_fileHDL      = 0; 
    private $_file_exists      = 0;
    var $_fileRealname = "";
	var $_globLogFile  = "";
    // var $_spacer       = "\t";
    var $_spacer       = " ";
    var $_doLogging    = false; // manage log "SQL_USER_LOG": tells if the SQL_USER_LOG is to be written to disk
	var $_doGlobLog    = false; // do SQL_GLOBLOG ? 0: no, 1: yes
	var $globLogTimeDiff=0;		// minimum time to log an SQL_GLOBLOG-event
    var $_errorObj     = false; // SingleError_object, if error occured
## public variables:
    var $query_start_time = 0;  // start time of a query
	var $quid		   = NULL;	// Query-identificator: name = "Class:Func:section"

# FUNCTION: write log files for SQL-queries
# ERRORS:
#         1 .. log-directory is not writeable, warning=1
#         2 .. log-file not writeable, warning=1
  
# constructor for class of pattern-type 'singleton':
# users should never call the constructor of this class, use $log = & SQL_log::getHandle() to get an instance of this class

# private constructor
    function __construct() {
        // NEVER instanciate the class SQL_log by hand!
        // Only ONE instance of this class is allowed!
        // use $log = & SQL_log::getHandle() to get an instance of the class!
    }
	

# SQL_log   getHandle()
    static function &getHandle() {
# desr:   static method to get a handle of this singleton-class.
# usage:  $mySQL_log = & SQL_log::getHandle();
# return: object of type SQL_log or void on error
# errors: none, use method gotError to test for errors

        static $sql_log_instance;

        $error = &ErrorHandler::get();

        if (!isset($sql_log_instance)) {
		
            $sql_log_instance = new SQL_log(); // warning: leave this way, because references are not stored statically!
			
            $sql_log_instance->globLogTimeDiff = 700.0;
            if ($_SESSION["globals"]["globLogTimeDiff"]>0) {
            	$sql_log_instance->globLogTimeDiff = $_SESSION["globals"]["globLogTimeDiff"];
            }
            
			// test global flag
			if ($_SESSION["globals"]["globLogFlag"]>0) {
				$sql_log_instance->setGlobLogging(true);
			}
			
            // sql logging is only enabled if the
            // flag '$_SESSION["userGlob"]["g.sql_logging"]' is set to 1.
            if (isset($_SESSION["userGlob"]["g.sql_logging"])
                && $_SESSION["userGlob"]["g.sql_logging"] == 1)
                $sql_log_instance->setLogging(true);
            else
                $sql_log_instance->setLogging(false);
            if ($error->got()) {
                $sql_log_instance->_errorObj = $error->getLast();
            }
        }
        return $sql_log_instance;
    }

# public method
    function &gotError() {
# returns object of type SingleError or false (if no error)
        return $this->_errorObj;
    }


	# public bool-method
	# retruns, if the log is written to file.
    function isLogging() {
        return $this->_doLogging;
    }

	# analyse php-function stack, 
    # get only ONE call outside of 'db_access.inc'
    # $level : level of func-calls: default:0
	static function _getFuncStack($level) {

		$retval=NULL;
		$tmparr = debug_backtrace();
	 	next($tmparr); // remove one level

		
		$cnt=0;
		foreach( $tmparr as $id=>$content) {  
			$filex = basename($content["file"]);
			if ($filex=='db_access.inc') continue; // get call from outside ... 

			$outarr=NULL;
			$outarr[1]=$filex;   
			$outarr[2]=$content["function"];
			$outarr[3]=basename($content["line"]);
			$retval .= '<qustack file:'.$outarr[1]. ' func:'.$outarr[2].
					  ' line:'.$outarr[3].'>';
			if ($cnt>=$level) break;
			$cnt++;
		}
		reset($tmparr);
		

		return $retval;
	}
	
	/**
	 * get prefix-name for a global log file
	 */
	function getGlobLogFilePre() {
		$logpath = $_SESSION["globals"]["app.log.dir"] ;
		$serial   = $_SESSION["globals"]["magasin_serial"];
		$filenamePre = $logpath . DIRECTORY_SEPARATOR ."parti.". $serial . ".";
		return ($filenamePre);
	}
	
	function getGlobLogFile() {
		// get globallog-file name
		$filename = $this->getGlobLogFilePre() . "sqllog.txt";
		return ($filename);
	}


	function _getLogFileName() {
		$filename    = 'sql_log.txt';
		$abs_dirname = $_SESSION['globals']['work_path']. "/pdir_". session_id(); // full qualified WORK_FILE for a USER session
		return array('file'=>$filename, 'path'=>$abs_dirname, 'rel'=>$filename );
	}
	
	function log_file_exists() {
	    return $this->_file_exists;
	}

	/**
	 * opens the file for sql-logging
	 * return: use the error-handler for checking the state
	 *  errors:
         1 .. Directory is NOT writeable, warning=1
         2 .. log-file not writeable, warning=1
	 * @param string $file_mode  mode for fopen ('a' or 'w')
	 */
    private function _openLogfile ($file_mode) {
        global $error;
        $error      = & ErrorHandler::get();
        $fileinf    = $this->_getLogFileName();
        $filename   = $fileinf['file'];
        $abs_dirname= $fileinf['path'];
        $this->_file_exists=0;
        $this->_fileHDL = NULL;
        
        if (!file_exists($abs_dirname)) {
            mkdir($abs_dirname);
        }
	
        if (!is_writable($abs_dirname)) {
            // set ONLY WARNING here
            $error->set("SQL_log", 1, "Log-Directory ('$abs_dirname') is NOT writeable. SQL-logging switched off.", 1);
            return;
        } 
        $abs_filename = realpath($abs_dirname) . '/' . $filename;
	  
        if (!($this->_fileHDL = @fopen($abs_filename, $file_mode))) { 
            // set ONLY WARNING here
            $error->set("SQL_log", 2, "Can't open log-file '$abs_filename' for writing. SQL-logging switched off.", 1);
            return;
        } 
        // Function realpath() only works on already existing files,
        // thus we call this function after opening the log file:
        $this->_fileRealname = realpath($abs_filename);
        $this->_write_header();
        $this->_file_exists=1;
    }

	/**
	 * switches logging on or off
	 * errors: see _openLogfile
	 * @param int $boolVal 0,1
	 */
    function setLogging( $boolVal) {
        global $error;
        $error = & ErrorHandler::get();

        if (!$this->_fileHDL && $boolVal) {
            // no logfile open, but logging switched on
            $this->_openLogfile("a");
            if ($error->got(CCT_WARNING)) {
                $this->_doLogging = false;
                return;
            }
        } elseif ($this->_fileHDL && !$boolVal) {
            // logfile open, but logging switched off
            $this->close();
        }
        $this->_doLogging = $boolVal;
        
    }
	
	function setGlobLogging( $boolVal ) {
		// set global logging
		$this->_doGlobLog = $boolVal;
		$this->_globLogFile = $this->getGlobLogFile();
	}


	function _dateFormat($time_unx) {
		return date('Y-m-d H:i:s', $time_unx);
	}
	
	# private method:
	// write general script info
    function _write_header() {
		
		
        if ( $this->_spacer == "\t") { $s = $this->_spacer; }
        else                         { $s = ''; }
	
        fwrite($this->_fileHDL,"\n\n");
        fwrite($this->_fileHDL,"{$s}{$s}SQL-Log:  ". $_SERVER["PHP_SELF"]." Modul:".$_REQUEST["mod"]."\n");
        fwrite($this->_fileHDL,"{$s}{$s}Date:         ". $this->_dateFormat(time()) ."\n");
		if (strstr($_SESSION['userGlob']["g.debugKey"], "sql:syspid")!="") {
			// log the HTTP-process PID and memory usage
			$pid = getmypid();
			$memhas = trim(`ps --pid $pid --no-headers -orss`);  // TBD: only for Linux
			fwrite($this->_fileHDL,"{$s}{$s}System:       PID:".$pid." MEM:".$memhas." kB\n");
		}
		
        fwrite($this->_fileHDL,"\n");

		## Write header of output table:
        $s = $this->_spacer;
        fwrite($this->_fileHDL,"DB-ID{$s}Qry-ID{$s}Query{$s}Time[ms]\n");
        fwrite($this->_fileHDL,"\n");

        return 0;
    } ##End function


# public method:
    function write_queryline( &$queryOBJ, $linebreak=0 ) {
# write out the actual query string and some other attributes contained in $queryOBJ.
	  	
		
        if (!$this->_doLogging) return;

		
        $s    = $this->_spacer;
        $col1 = preg_replace('/^Resource id #/','',$queryOBJ->_db_linkid);
        $col2 = preg_replace('/^Resource id #/','',$queryOBJ->_db_qresult);
		$col2_ext = NULL;
		if ($this->quid != NULL) {
			$col2_ext = " <quid=".$this->quid.">";
		} 
		
		$keyx = 'sql:functrc';
		if ( strstr( $_SESSION['userGlob']["g.debugKey"], $keyx)!=NULL ) {
			$deblev = 0;
			if ( $_SESSION['userGlob']["g.debugLevel"]>0 ) $deblev = $_SESSION['userGlob']["g.debugLevel"];
			$stacktmp = $this->_getFuncStack($deblev);
			$col2_ext .= " ".$stacktmp;
		} 
		if ( strstr( $_SESSION['userGlob']["g.debugKey"],"sql:disp")!=NULL ) 
			echo "SQL-debug: ".htmlspecialchars($col2_ext.$queryOBJ->_db_qstring)."<br>\n";
		
        $col3 = preg_replace('/\s+/',' ',trim($queryOBJ->_db_qstring));
		if ($linebreak) $newline = "\n";
		else  $newline = NULL;
		
		if (!$this->_fileHDL) return;
        fwrite($this->_fileHDL,sprintf("%4d {$s}",$col1).sprintf("%4d%s  {$s}",$col2, $col2_ext) ."$col3 ".$newline);
		 
        return 0;
    }##End function 

	/**
	 * write end of the queryLine
	 * - support SQL_GLOBLOG
	 */
    function write_querylineEnd( &$queryOBJ ) {
	  	global $error;
		
        if (!$this->_doLogging AND !$this->_doGlobLog) return;
        
        $end_time = gettimeofday(); 
        $exectime = ($end_time["sec"] + $end_time["usec"]*MATH_1d1000000 - ($this->query_start_time["sec"] + $this->query_start_time["usec"]*MATH_1d1000000)) * 1000; 
        
		if ($this->_doGlobLog) {
			if ($exectime > $this->globLogTimeDiff) {
				$globfilename = $this->_globLogFile;
				$col1 = preg_replace('/^Resource id #/','',$queryOBJ->_db_linkid);
				$col2 = preg_replace('/^Resource id #/','',$queryOBJ->_db_qresult);
				$col3 = preg_replace('/\s+/',' ',trim($queryOBJ->_db_qstring));
				
				$phpfile = $_SERVER["PHP_SELF"];
				if (substr($phpfile, -5)=='p.php') {
					// get module-name
					$phpfile .= '?mod='.$_REQUEST['mod']; // append module-name
				}
				
				if (($globFileHndl = @fopen($globfilename, 'a' ))) {  // append, fast open, fast close
				
					fwrite( $globFileHndl, date('Y-m-d H:i:s')."\tUser: ".$_SESSION['sec']['db_user_id']."\tScript: ".$phpfile ."\t" );
					fwrite( $globFileHndl, sprintf("%4d\t",$col1).sprintf("%4d\t",$col2)."\t".$col3);
					fwrite( $globFileHndl, sprintf("\tTime: %4d",$exectime)." ms\n");
					fclose($globFileHndl);
				} else {
					// TBD: do not set error, may be parallel access to this file
					// $error->set("write_querylineEnd", 1, "Can't open global-log-file '".$globfilename."' for writing.", 1);
					// return;
				}
			}
		}
		if ($this->_doLogging) {
			fwrite($this->_fileHDL, sprintf("<end>Date:%s Exec:%.2f</end>", date('H:i:s'), $exectime)."\n");
		}
        return 0;
}
    
# public method:
    function write_closeSqlHandle( &$queryOBJ ) {
# write out time info about close of DB-handle
        if (!$this->_doLogging) return; 
        $s    = $this->_spacer;        
        fwrite($this->_fileHDL,"{$s}CLOSE handle: Date:         ".date('D, j M Y G:i:s T')."\n");
        fwrite($this->_fileHDL,"\n");
        return 0;
    }##End function


# public method:
    function write_errorline( &$queryOBJ, $info=NULL ) {
# write out the latest query error string contained in $queryOBJ.

        if (!$this->_doLogging) return;
        $s = $this->_spacer;

        if ( $queryOBJ->_db_qerror )
            fwrite($this->_fileHDL,"ERR  {$s}      {$s}" . $queryOBJ->_db_qerror . "\n");
        return 0;
    }##End function


# public method:
    function write_textline( $text='' ) {
# write an arbitrary text into SQL log file. Usage of this function is NOT recommended.

        if (!$this->_doLogging) return;

        fwrite($this->_fileHDL,"$text\n");
        return 0;
    }##End function


# public method:
    function blank_file() {
# descr:  remove (clear) all content of the log file.
# return: void
# errors:
#         see _openLogfile

        if (!$this->_doLogging) return;

        fclose($this->_fileHDL);
        $this->_openLogfile("w");
    }##End function


# public method:
    function close() {
# stop SQL logging.
##
## SQL logging is stopped temporarily, but will be
## resumed as soon as a new SQL query object is
## created by calling 'logon_to_db()'. This typically happens
## when the next PHP script is started within the current session.
##
## You must toggle $_SESSION["userGlob"]["g.sql_logging"]
## to disable SQL logging permanently.
# return: void
# errors:
#         see setLogging

        if ($this->_doLogging) {
            $s = $this->_spacer;
            fwrite($this->_fileHDL,"WARN {$s}      {$s}" . "SQL logging switched off.\n");
            fclose($this->_fileHDL);
            $this->_fileHDL = false;
        }

        $this->_doLogging = false;

    }##End function

}##End Class

// DO NOT CLOSE THE PHP-TAG, due to prevent output of WHITE-SPACES or other characters ...
/*************************************************************************************************/
