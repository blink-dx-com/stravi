<?php
/**
 * global, common and basic functions
 * @package globalFuncs
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/phplib/globals.inc 59 2018-11-21 09:04:09Z $
 * @var VARSTRUCT updateSqlOpt_STRUCT 
 * 	"types" = array [COL_NAME] => type 
 *     "DATE1" :: YYYY-MM-DD HH24:MI:SS
 *     "TIME"  :: UnixStamp
 */
 
require_once('access_check.inc');
require_once('varcols.inc'); 

// get remark of table
function table_remark2($tablename) {
  global $_s_i_table;
  return $_s_i_table[$tablename]['__table_data__']['COMMENTS'];
}

// get comment of column
function column_remark2($tablename, $colname) {
  global $_s_i_table;
  return $_s_i_table[$tablename][$colname]['COMMENTS'];
}

/**
 * get (foreign table, primary key, important name column) of (tablename,$primary_key) 
 * @deprecated : use objInfoC::fk_info()
 * @param  $primary_key
 * @param  $tablename
 * @param string  $ftab_prim_name [RETURN] name of primary key column
 * @param string $ftab_imp_name  [RETURN] name of most important column
 * @return string foreign key table
 */
function fk_check2($primary_key, $tablename, &$ftab_prim_name, &$ftab_imp_name ) {
  global $_s_i_table;
  
  if ($outstr = $_s_i_table[$tablename][$primary_key]['CCT_TABLE_NAME']) {
    $ftab_prim_name = PrimNameGet2($outstr) ? PrimNameGet2($outstr) : $primary_key;
    $ftab_imp_name  = importantNameGet2($outstr) ? importantNameGet2($outstr) : $ftab_prim_name;
    return $outstr;
  }
  return '';
}

/**
 * get zero based array containing primary keys in descending order of their importance
 * @param $tablename
 * @return array (0 => 'most imp pk', 1 => 'pk2' ...)
 */
function primary_keys_get2($tablename) {     
  global $_s_i_table;

  if (isset($_s_i_table[$tablename]))
	return $_s_i_table[$tablename]['__table_data__']['PRIM_KEYS'];
  return array();
}

/**
 * get string for where SQL-condition (concat primary key vals)
 * @param  $primas
 * @param  $primasid
 * @param  $idname
 * @param  $id
 */
function primary_select_str(&$primas, &$primasid,  $idname, $id  ) {
	$select_str= $idname . "='" . $id ."'";	
	if (!empty($primas[1]) && isset($primasid[1])) {
		$select_str=$select_str . ' AND '.$primas[1]."='".$primasid[1]."'";
	}
	if (!empty($primas[2]) && isset($primasid[2])) {
		$select_str=$select_str . ' AND '.$primas[2]."='".$primasid[2]."'";
	}
	return ($select_str);
}

/**
 * concat primary key vals for sql-selection
 * @param  $sql
 * @param  $tablename
 * @param  array $pkey_values values of primary keys (zero based array (array (0 => 'first_pk_value', 1 => 'second_pk_value', ...)
 * @return string
 */
function primary_select_str2(&$sql, $tablename, $pkey_values) {
	global $_s_i_table;
	$pks        = array();

	foreach($pkey_values as $num => $id)
		$pks[] = $_s_i_table[$tablename]['__table_data__']['PRIM_KEYS'][$num].' = '.$sql->addQuotes($id);

	return implode (' AND ', $pks);
}

/**
 * get most NAME column of table
 * @param  $tablename
 * @return string
 */
function importantNameGet2($tablename) {
  global $_s_i_table;
  return $_s_i_table[$tablename]['__table_data__']['MOST_IMP_COL'];
}

// get KEY of tablename
function globTabMetaByKey($tablename, $key) {
  global $_s_i_table;
  return $_s_i_table[$tablename]['__table_data__'][$key];
}
// get SHORT of tablename
function globTabGetSHORT($tablename) {
    global $_s_i_table;
    return $_s_i_table[$tablename]['__table_data__']['SHORT'];
}
// get tablename by SHORT code
function globTablebyShort($short) {
    global $_s_i_misc;
    return $_s_i_misc['t']['SHORT'][$short];
}

// get primary key column
function PrimNameGet2($tablename) {
  global $_s_i_table;
  return $_s_i_table[$tablename]['__table_data__']['PRIM_KEYS'][0];
}

// get number of primary keys
function countPrimaryKeys($tablename) {
  global $_s_i_table;
  if (!is_array($_s_i_table[$tablename]['__table_data__']['PRIM_KEYS']))  return 0;
  return count($_s_i_table[$tablename]['__table_data__']['PRIM_KEYS']);
}

/**
 * check, if table is BO (check CCT_ACCESS_ID)
 * @param $tablename
 * @return false or true
 */
function cct_access_has2($tablename) {
  global $_s_i_table;
  return empty($_s_i_table[$tablename]['__table_data__']['IS_BUSINESS_OBJECT']) ? false : true;
}

/**
 * get CCT_ACCESS_ID of object
 * @param  $sql
 * @param  $tablename
 * @param  $prim_id
 * @return int CCT_ACCESS_ID
 */
function cct_access_get( &$sql, $tablename, $prim_id ) {
  $sql->query("SELECT cct_access_id FROM $tablename WHERE " .PrimNameGet2($tablename)." = $prim_id");
  if ($sql->ReadRow())
	return ($sql->RowData[0]);
  else
	return 0;
}

// table is a VIEW ?
function table_is_view($tablename) {
  global $_s_i_table;
  return (empty($_s_i_table[$tablename]['__table_data__']['IS_VIEW']) ? '' : $_s_i_table[$tablename]['__table_data__']['IS_VIEW']);
}

// table exists in system ?
function glob_table_exists($tablename) {
  global $_s_i_table;
  return ( empty($_s_i_table[$tablename] ) ? 0 : 1);
}

// column exists in system ?
function glob_column_exists($tablename, $colname) {
  global $_s_i_table;
  return ( empty($_s_i_table[$tablename][$colname] ) ? 0 : 1);
}

// get nice name of table
function tablename_nice2($tablename) {
  global $_s_i_table;
  
  $nicename = empty($_s_i_table[$tablename]['__table_data__']['NICE_NAME']) ? '' : $_s_i_table[$tablename]['__table_data__']['NICE_NAME'];
  if (!$nicename && ($_SESSION['sec']['appuser'] == 'root'))
    $nicename = $tablename; /* no name for normal users, if not described */
  return $nicename;
}

// make first letter capital
function tablename_nice_fcap($tablename) {
    $nice = tablename_nice2($tablename);
    $nice = strtoupper(substr($nice,0,1)).substr($nice,1);
    return $nice;
}

// table exists in meta-structure ?
function table_exists2($tablename) {
  global $_s_i_table;
  $exists=0;
  if ( is_array($_s_i_table[$tablename]) ) $exists=1;
  return $exists;
}

// get nice name of table
function columnname_nice2($tablename, $columnname) {
  global $_s_i_table;

  return $_s_i_table[$tablename][$columnname]['NICE_NAME'];
}
   


// get mother table of this table
function mothertable_get2($tablename) {
  global $_s_i_table;
  return $_s_i_table[$tablename]['__table_data__']['CCT_TABLE_NAME'];
}

/**
 * current user is admin ?
 * in future, this can be defined by a role right
 * @return boolean 0: no, 1: yes
 */
function glob_isAdmin() {
	$isAdmin = 0;
	if ( $_SESSION['sec']['appuser']=="root" ) $isAdmin = 1;
	return ($isAdmin);
}

/**
 * user is logged in ???
 * @return boolean
 */
function glob_loggedin() {
	$isLoggedin = 0;
	if ( empty($_SESSION['sec']) ) return 0;
	if ( $_SESSION['sec']['appuser']!="" ) $isLoggedin = 1;
	return ($isLoggedin);
}

// print strong text
function info_out( $strong, $info ) {
	echo (empty($strong) ? '' : '<font color="#ff0000"><B>'.$strong.':</B></font> ').$info.'<br>'."\n";
}

/**
 * print an array as structured html for debugging, See also: glob_array2String()
 * @param array $vararr
 * @param string $info
 * @param int $format
 *   0 : <PRE>print_r</pre>
 *   1 : foreach() key: vals. <br>
 */
function glob_printr($vararr, $info, $format=0) {
	echo "glob_printr() <b>$info</b>";
	if ( !is_array($vararr) ) {
		echo ' : is not an array.'."<br>\n";
		var_dump($vararr);
		echo "<br>\n";
		return;
	}
	
	switch($format) {
	    case 1:
	        echo "<br>\n";
	        foreach($vararr as $key=>$val) {
	            echo ' - '.$key.': ';
	            if (is_array($vararr)) echo print_r($val,1);
	            else echo $val;
	            echo "<br>";
	        }
	        echo "\n";
	       break;
	       
	    default:
        	echo "<pre>";
        	print_r ($vararr);
        	echo "</pre>\n";
       
	}
}

 
/**
 * simple debug text
 * with $_SESSION["userGlob"]["g.debugKey"] contains 'saveToTmpFileOnly': save to /tmp/gozilla_debug.txt
 * @param string $text
 * @param string $funcname - name of the show FUNCTION
 * @param number $minLevel - just show the message if $_SESSION["userGlob"]["g.debugLevel"] >= $minLevel
 */
function debugOut($text, $funcname=NULL, $minLevel=0, $saveToFile=0) {
	if ($minLevel>0) {
		if ( $_SESSION["userGlob"]["g.debugLevel"]<$minLevel ) return;
	}
	
	if ( strstr($_SESSION["userGlob"]["g.debugKey"], 'saveToTmpFileOnly')!=NULL or $saveToFile) {
	    
	    $logdir = $_SESSION["globals"]['app.log.dir'];
	    if($logdir==NULL) $logdir='/tmp';  // fallback
	    $datafile = $logdir.'/gozilla_debug.txt';
	    $fp = fopen($datafile, 'a+');
	    fwrite( $fp, date("Y-m-d H:i:s").':DEBUG: '.$funcname."(): ".$text."\n" ); 
	    fclose( $fp );

	    return;
	}
	
	echo "<b>DEBUGX:</b> ".$funcname."(): ".$text."<br>\n";
}

/**
 * serialize an array to a human readable string, See also: glob_printr()
 * @param  $vararr
 * @param  $slim  : 0-ALL, 1-slim (only pairs with VAL)
 * @param  $separator e.g. ', '
 * @return string
 */
function glob_array2String( 
	$vararr, 
	$slim=0,  
	$separator = ", "
	) { 
	$outstr = "";
	if ( !is_array($vararr)) return;
	
	$komma= "";
	foreach( $vararr as $key=>$val) {
		if (!$slim OR $val!=NULL) {
			$outstr .= $komma. $key."=".$val;
			$komma   = $separator;
			if ( is_array($val) ) {
				$outstr .= " ( ".glob_array2String($val, $slim) .") \n";
			}
		}
	}
	reset ($vararr);
	return ($outstr);
}


/**
 * serialize an key-value-pairs to a human readable string
 * @return string
 */ 
function glob_arrVals2Str( 
	$vararr, 
	$separator = ", "
	) {
	$outstr = "";
	if ( !is_array($vararr)) return;
	$komma= "";
	foreach( $vararr as $key=>$val) {
		$outstr .= $komma. $val;
		$komma   = $separator;
	}
	reset ($vararr);
	return ($outstr);
}

/**  
 * get the function stack as structured string
 * - this method must be highly reliable, to support the use of trigger_error()
 * @param int $level level of stack
 * @return string
 */
function glob_getFuncStack($level) {

		$retval = NULL;
		$tmparr = debug_backtrace();
		
		if (!is_array($tmparr)) return "<qustack UNKNOWN error, no call stack>\n";
		
	 	next($tmparr); // remove one level
		
		$cnt=0;
		foreach( $tmparr as $id=>$content) {  
			$filex = basename($content["file"]);

			$outarr=NULL;
			$outarr[1]=$filex;   
			$outarr[2]=$content["function"];
			$outarr[3]=basename($content["line"]);
			$retval .= '<qustack file:'.$outarr[1]. ' func:'.$outarr[2].
					  ' line:'.$outarr[3].'>'."\n";

			if ($cnt>=$level) break;
			$cnt++;
		}
		

		return $retval;
	}

	
# string starts with
function str_startswith ($haystack, $needle) {
	$len = strlen($needle);
	return (substr($haystack, 0, $len) === $needle);
} 
# string ends with needle
function str_endswith($haystack, $needle) {
    $length = strlen($needle);
    if ($length == 0) {
        return true;
    }
    return (substr($haystack, -$length) === $needle);
}

/**
 * - trigger parisan system error
 * - OLD: write complete call stack to PHP-LOG; done by trigger_error()
 */ 
function  sys_error_my( $message ) {
	throw new Exception($message);
	// OLD: 
	//$answer = glob_getFuncStack(5);
	//$answer = str_replace("\n", "[LF]", $answer);
	// trigger_error( $answer, E_USER_ERROR );
	// exit(-1);
}

/**
 * super global methods
 * @author steffen
 */
class gGlobal {
	
	/**
	 * this DB-version is minimum $minVersion ?
	 * @return boolean 0,1
	 */
	static function dbVersionIs( $minVersion ) {
		if ( $_SESSION["globals"]["DBVersionShort"] >= $minVersion  ) return 1;
		else return 0;	
	}
	
	// this edition is an internal one ?
	static function productIsIntern() {
		$edition = $_SESSION['s_product']['type'];
		if ($edition=='arrayLIMS') return 1;
		else return 0;
	}

}
