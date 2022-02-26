<?php
########################################################################
# $Header: trunk/src/www/_tests/phplibSimu/db_access.inc 59 2018-11-21 09:04:09Z $
########################################################################
/*
##    Simple Database Abstraction Layer 1.5 [lib.sdba.php] 
##    by Gabe Bauman <gabeb@canada.com>
##    Wednesday, April 05, 2000
##    extended by Michael Howitz <icemac@gmx.net>
##    Thursday, Jun 15, 2000 and
##    Tuesday,  Jun 20, 2000
## 
##    Easy way to read and write to any database.
## 
##
##    Usage: 
##    $sql = new CDB_OCI8 ($DB_HOST, $DB_USER, $DB_PASS);
##    $sql->Query('SELECT lastname, firstname FROM people');
##    while ($sql->ReadArray()) { 
##      echo $sql->RowData['LASTNAME'].','.$sql->RowData['FIRSTNAME']."<br>\n"; 
##    }
##    $sql->close(); // not really necessary
## 
##    DEBUG: set $_SESSION['userGlob']["g.debugLevel"] : 0,1,2,3, ...
				 $_SESSION['userGlob']["g.debugKey"]   : 
						"sql:bulk"   : bulk-action debug
						"sql:disp"   : display SQL-command
						"sql:syspid" : log the HTTP-process PID and memory usage
 
# thrown errors: caller='db_access'
#	1 .. connection failed
#   2 .. error in query
*/

require_once('ErrorHandler.inc');
require_once('get_cache.inc');

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




// DO NOT CLOSE THE PHP-TAG, due to prevent output of WHITE-SPACES or other characters ...
/*************************************************************************************************/
