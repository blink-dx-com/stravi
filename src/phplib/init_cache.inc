<?php
require_once('db_access.inc');

/**
 * - initializes the cache for the application metadata (varcol data, cct_table & cct_colum);
 * - only init tables which are in CCT_TAB_VIEW and CCT_TABLE !
 * $Header: trunk/src/phplib/init_cache.inc 59 2018-11-21 09:04:09Z $
 * @package  init_cache.inc
 * @author  mac, Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @global array $_s_i_table 
 *    extra cached keys:
 *    - IS_BUSINESS_OBJECT
 *    - PRIM_KEYS
 *    - MOST_IMP_COL
 *    ... from CCT_TABLE_DYN ...
 *    - FOREIGN_DB 
 *    - SHORT
 * @global array $_s_i_app_data_type
 * @global array $_s_i_misc misc global data
 *    't' : table data  
 *      'SHORT': short code of table
 */
class gInitCache {
	
/** @var array $warnarr : warning array = array( 's'=>$initStruc, 't'=>$table, 'c'=>$col, 'i'=>$info, 'w'=>$warnnum )
 *		$initStruc: CCT_TABLE, CCT_COLUMN, EXTRA_ATTRIB
 */
var $warnarr; 
	
function __construct() {
	$this->warnarr=NULL;
}

/**
 * set a warning array
 * @param string $initStruc 'CCT_TABLE', 
 * @param $table
 * @param $col
 * @param $info
 */
function _setwarn($warnnum, $initStruc, $table, $col, $info ) {
	$this->warnarr[] = array( 's'=>$initStruc, 't'=>$table, 'c'=>$col, 'i'=>$info, 'w'=>$warnnum );
}

function getWarnArr() {
	return ($this->warnarr);
}

// got warnings ?
function warnGot() {
	return ( sizeof($this->warnarr));
}

/**
 * set meta key values
 * @param $metaArray
 * @param string $featKey (e.g. 'COLOR' )
 * @return 
 */
function _setMetainfo(&$metaArray, $featKey ) {
	global $_s_i_table;
	
	reset ($metaArray);
	foreach( $metaArray as $tab_name=>$val) {
		 $_s_i_table[$tab_name]['__table_data__'][$featKey] = $val;
	}
	reset ($metaArray); 
}
	
/**
 * initalizes the cache of the table meta data (cct_table & cct_column)
 * create $_s_i_table, $_s_i_app_data_type, $_s_i_misc
 * GLOBAL:
 *   $_s_i_table[TABLE][COLUMN]
 *       'DATAX_arr' json-decoded array
 *        
 * ERRORs
 * db_access  1 .. no connecton to db
 *            2 .. error in query
 *  init_cache 1 .. column not defined in cct_column; warning class 1
 *     2 .. app_data_type 'string' missing. Please contact support.
 *     3 .. table not defined in cct_table; warning class 1
 *     4 .. table defined in cct_table but not existing in database; warning class 1
 *    10 .. database says column is not nullable but cct_column tells opposite; warning class 1
 *    11 .. column does not exist in database but in cct_column; warning class 1
 *	  12 .. table not defined in CCT_TABLE
 * @param string $LogName
 * @param string $passwd
 * @param string $db
 * @param string $_dbtype
 */
function init_table_cache($LogName, $passwd, $db, $_dbtype) {     
    global $_s_i_table, $_s_i_app_data_type, $_s_i_misc;

    $error = & ErrorHandler::get();
    $sql   = logon_to_db($LogName, $passwd, $db, $_dbtype, 'std', false);
    if ($error->got(CCT_ERROR_READONLY)) return;
    $_s_i_table = array();

    // temporary code to have both varcol-versions accessible
    //$sql->query('SELECT value FROM globals WHERE name = '.$sql->addQuotes('DBVersion'));
    //$sql->ReadRow();
    //$dbversion = substr($sql->RowData[0], 0, 7);
    // end temp-code

    $string_adt_id  = null;
    $boolean_adt_id = null;

    foreach($_s_i_app_data_type as $adt_id => $adt_data) {
        if ($adt_data['NAME'] === 'string')
            $string_adt_id = $adt_id;
        if ($adt_data['NAME'] === 'boolean')
            $boolean_adt_id = $adt_id;
    }

    if (is_null($string_adt_id))
        return $error->set('init_cache', 2, 'app_data_type \'string\' missing. Please contact support.');
    if (is_null($boolean_adt_id))
        return $error->set('init_cache', 2, 'app_data_type \'boolean\' missing. Please contact support.');

    // just create table-entries, which are in CCT_TABLE
    // use the UPPER to support alos POSTGRES ...
	$sql->query('SELECT v.NAME, t.TABLE_NAME as TABLE_META FROM CCT_TAB_VIEW v left join CCT_TABLE t on UPPER(v.NAME)=t.TABLE_NAME '.
		' order by v.NAME '); // get all tables, which are also in CCT_TABLE
    if ($error->got(CCT_ERROR_READONLY)) return;
    
    while ($sql->ReadArray()) {
 
        $tab_name = strtoupper($sql->RowData['NAME']);
        $tab_meta = strtoupper($sql->RowData['TABLE_META']);
        if ($tab_meta==NULL) {
        	$this->_setwarn(12, 'CCT_TABLE', $tab_name, '', 'table not defined in CCT_TABLE. ignored.');
        	continue;
        }
        $_s_i_table[$tab_name] = array();
        $_s_i_table[$tab_name]['__table_data__'] = array();
        $_s_i_table[$tab_name]['__table_data__']['CCT_TABLE_NAME'] = ''; // set mother table later
        $_s_i_table[$tab_name]['__table_data__']['NICE_NAME']  = $tab_name; // do not beautify this name to have a criterion to distiguish between table which are & which are not on cct_table
        $_s_i_table[$tab_name]['__table_data__']['TABLE_TYPE'] = '';
        $_s_i_table[$tab_name]['__table_data__']['IS_VIEW']    = 0;
        $_s_i_table[$tab_name]['__table_data__']['INTERNAL']   = 1; // not in cct_table defined tables are always internal
        $_s_i_table[$tab_name]['__table_data__']['EXIM']       = 1; // deny export
        $_s_i_table[$tab_name]['__table_data__']['COMMENTS']   = $sql->RowData['COMMENTS'];
        $_s_i_table[$tab_name]['__table_data__']['PRIM_KEYS']  = array(); // set later
        $_s_i_table[$tab_name]['__table_data__']['IS_BUSINESS_OBJECT'] = false; // set later
        $_s_i_table[$tab_name]['__table_data__']['MOST_IMP_COL'] = ''; // set later
    }
    
    // precollect foreign-database-tables
    $foreign_tables = array();
    $sql->query("SELECT TABLE_NAME, VALUE FROM CCT_TABLE_DYN where KEY='FOREIGN_DB' and VALUE is not NULL order by TABLE_NAME");
    if ($error->got(CCT_ERROR_READONLY)) return;
    while($sql->ReadRow()) {
        $tab_name = $sql->RowData[0];
        $val_tmp  = $sql->RowData[1];
        $foreign_tables[]=$tab_name;
    }
        
    $sql->query('SELECT * FROM CCT_TABLE order by TABLE_NAME'); // get tables
    if ($error->got(CCT_ERROR_READONLY)) return;
    while($sql->ReadArray()) {
        $tab_name = $sql->RowData['TABLE_NAME'];
        $_s_i_table[$tab_name]['__table_data__']['CCT_TABLE_NAME'] = $sql->RowData['CCT_TABLE_NAME']; // mother table
        $_s_i_table[$tab_name]['__table_data__']['NICE_NAME'] 	= $sql->RowData['NICE_NAME'];
        $_s_i_table[$tab_name]['__table_data__']['TABLE_TYPE'] 	= $sql->RowData['TABLE_TYPE'];
        $_s_i_table[$tab_name]['__table_data__']['IS_VIEW'] 	= $sql->RowData['IS_VIEW'];
        $_s_i_table[$tab_name]['__table_data__']['INTERNAL'] 	= $sql->RowData['INTERNAL'];
        $_s_i_table[$tab_name]['__table_data__']['EXIM'] 		= $sql->RowData['EXIM'];
        if (!array_key_exists('COMMENTS', $_s_i_table[$tab_name]['__table_data__'])) {
            
            if (in_array($tab_name, $foreign_tables)) {
                $this->_setwarn( 5, 'CCT_TABLE', $tab_name, '',
                    ' is located in a FOREIGN table!');
            } else {
    			$this->_setwarn( 4, 'CCT_TABLE', $tab_name, '',
    					    ' is defined in CCT_TABLE but does not exist in database.'.
    						'(Removed from cache)');
                // $_s_i_table[$tab_name]['__table_data__']['COMMENTS'] = 'no comment';
    			unset ( $_s_i_table[$tab_name] );
            }
        } elseif (!$_s_i_table[$tab_name]['__table_data__']['COMMENTS'])
              $_s_i_table[$tab_name]['__table_data__']['COMMENTS'] = 'no comment';
        
        if ($sql->RowData['NOTES']!=NULL) {
            $_s_i_table[$tab_name]['__table_data__']['COMMENTS']   = $sql->RowData['NOTES'];
        }
        
        if ($sql->RowData['IS_ACTIVE']<=0) {
            unset($_s_i_table[$tab_name]);
            $this->_setwarn(14, 'CCT_TABLE', $tab_name, '', 'table is INACTIVE due to CCT_TABLE setting.');
        }
    }
	
    // get vario table values from CCT_TABLE_DYN
    $sql->query('SELECT * FROM CCT_TABLE_DYN order by TABLE_NAME'); 
    if ($error->got(CCT_ERROR_READONLY)) return;
    while($sql->ReadArray()) {
    	$tab_name = $sql->RowData['TABLE_NAME'];
    	$key      = $sql->RowData['KEY'];
    	$value    = $sql->RowData['VALUE'];
    	
    	
    	$_s_i_table[$tab_name]['__table_data__'][$key] = $value;
    	if ($key=='SHORT') {
    	    $_s_i_misc['t'][$key][$value]=$tab_name;
    	}
    }

    
    // get column comments (done first to get also columns not described in cct_column)
    $sql->query('SELECT * FROM CCT_COL_VIEW'); 
    if ($error->got(CCT_ERROR_READONLY)) return;
    
    while ($sql->ReadArray()) {
        $tab_name = strtoupper($sql->RowData['TABLE_NAME']);
        $col_name = strtoupper($sql->RowData['COLUMN_NAME']);
        
        if ( !isset($_s_i_table[$tab_name]) ) {
        	continue; // not implemented !!!
        }
        
        $_s_i_table[$tab_name][$col_name] = array();
        
         // last char is a * (used by mac in db-design for special fk-comments)
        if (substr($sql->RowData['COMMENTS'], -1) === '*')
            $_s_i_table[$tab_name][$col_name]['COMMENTS'] = substr($sql->RowData['COMMENTS'], 0, -1);
        else
            $_s_i_table[$tab_name][$col_name]['COMMENTS'] = $sql->RowData['COMMENTS'];
        $_s_i_table[$tab_name][$col_name]['NICE_NAME']    = str_replace('_', ' ', strtolower($sql->RowData['COLUMN_NAME'])); // can't set later because then columns which are not in cct_column get no nicename
        $_s_i_table[$tab_name][$col_name]['PRIMARY_KEY']  = 0;
        $_s_i_table[$tab_name][$col_name]['MOST_IMP_COL'] = 0;
        $_s_i_table[$tab_name][$col_name]['UNIQUE_COL']   = 0;
        $_s_i_table[$tab_name][$col_name]['NOT_NULL'] = $sql->RowData['NULLABLE']; // the value is 'Y' or 'N'
        

        $_s_i_table[$tab_name][$col_name]['VISIBLE'] = 1;
        $_s_i_table[$tab_name][$col_name]['CCT_TABLE_NAME'] = ''; // needed here because searchAdvance.php relies on this key
        $_s_i_table[$tab_name][$col_name]['APP_DATA_TYPE_ID'] = NULL; // for columns which have no cct_column-entry, below it gets set to 'string'
    }
    
    $sql->query('SELECT * FROM CCT_COLUMN order by TABLE_NAME'); // get columns
    if ($error->got(CCT_ERROR_READONLY)) return;
    
    while ($sql->ReadArray()) {
      
        $tab_name = $sql->RowData['TABLE_NAME'];
        $col_name = $sql->RowData['COLUMN_NAME'];
        
     	if ( !isset($_s_i_table[$tab_name]) ) {
        	continue; // not implemented !!!
        }
    	if ( !isset($_s_i_table[$tab_name][$col_name]) ) {
    	    if (in_array($tab_name, $foreign_tables)) {
    	        // column is valid ...
    	        // init the columns fileds
    	        $_s_i_table[$tab_name][$col_name] = array();
    	    } else {
    		    $this->_setwarn(13, 'CCT_COLUMN', $tab_name, $col_name,'column not defined in CCT_COLUMN. ignored.');
        	    continue; // not implemented !!!
    	    }
        }
        
        if ($sql->RowData['NICE_NAME'])
            $_s_i_table[$tab_name][$col_name]['NICE_NAME'] = $sql->RowData['NICE_NAME'];
        elseif ($sql->RowData['CCT_TABLE_NAME']) // is foreign key? then use nicename of referred table
            $_s_i_table[$tab_name][$col_name]['NICE_NAME'] = $_s_i_table[$sql->RowData['CCT_TABLE_NAME']]['__table_data__']['NICE_NAME'];
        $_s_i_table[$tab_name][$col_name]['PRIMARY_KEY'] = $sql->RowData['PRIMARY_KEY'];
        $_s_i_table[$tab_name][$col_name]['POS'] = $sql->RowData['POS'];
        $_s_i_table[$tab_name][$col_name]['APP_DATA_TYPE_ID'] = $sql->RowData['APP_DATA_TYPE_ID'];
        $_s_i_table[$tab_name][$col_name]['CCT_TABLE_NAME'] = $sql->RowData['CCT_TABLE_NAME'];
        $_s_i_table[$tab_name][$col_name]['MOST_IMP_COL'] = $sql->RowData['MOST_IMP_COL'];
        $_s_i_table[$tab_name][$col_name]['UNIQUE_COL'] = $sql->RowData['UNIQUE_COL'];
        
        if ($sql->RowData['NOTES'])
            $_s_i_table[$tab_name][$col_name]['COMMENTS'] = $sql->RowData['NOTES'];

        if ($_s_i_table[$tab_name][$col_name]['NOT_NULL'] == 'N' and 
	        $sql->RowData['NOT_NULL'] == '0' and 
	        $sql->RowData['APP_DATA_TYPE_ID'] != $boolean_adt_id) {
            // 'N' means nullable = no; '0' means nullable = yes; booleans have no need for the not-null flag because of application logic
            $this->_setwarn(10, 'CCT_COLUMN', $tab_name, $col_name,  ' database says it is not nullable but CCT_COLUMN tells opposite. Database value overrules cct_column value.', 1);
            $_s_i_table[$tab_name][$col_name]['NOT_NULL'] = 1;
        } else {
            $_s_i_table[$tab_name][$col_name]['NOT_NULL'] = $sql->RowData['NOT_NULL'];
        }

        $_s_i_table[$tab_name][$col_name]['VISIBLE'] = $sql->RowData['VISIBLE'];
        $_s_i_table[$tab_name][$col_name]['EDITABLE'] = $sql->RowData['EDITABLE'];
        $_s_i_table[$tab_name][$col_name]['EXIM'] = $sql->RowData['EXIM'];
        
        if ($sql->RowData['DATAX']) {
            $_s_i_table[$tab_name][$col_name]['DATAX_arr'] = json_decode($sql->RowData['DATAX'],TRUE);
        }
        
        if (($sql->RowData['COLUMN_NAME'] == 'CCT_ACCESS_ID') && ( substr($tab_name,0,4) != 'CCT_') )
            $_s_i_table[$tab_name]['__table_data__']['IS_BUSINESS_OBJECT'] = true;
        if ($sql->RowData['MOST_IMP_COL'] == 1)
            $_s_i_table[$tab_name]['__table_data__']['MOST_IMP_COL'] = $col_name;
        if ($sql->RowData['PRIMARY_KEY'] > 0)
            $_s_i_table[$tab_name]['__table_data__']['PRIM_KEYS'][($sql->RowData['PRIMARY_KEY'] - 1)] = $col_name; // we have a zero based array!
        
    }

    foreach ($_s_i_table as $tabname => $tabdata) {
        foreach ($tabdata as $colname => $coldata) {
            if ($colname === '__table_data__') {
                if ($coldata['NICE_NAME'] === $tabname) {
                	// TBD: do NOT guess data, leave empty ???
                	$this->_setwarn( 3, 'CCT_TABLE', $tabname, '', 'is not defined in CCT_TABLE. All metadata-values of this table are guessed.');
                }
                if (isset($_s_i_table[$tabname]['__table_data__']['PRIM_KEYS']) and 
                    is_array($_s_i_table[$tabname]['__table_data__']['PRIM_KEYS']))
                    ksort($_s_i_table[$tabname]['__table_data__']['PRIM_KEYS']); // order prim_keys in ascending order (1,2,3)
            } elseif (!isset($coldata['APP_DATA_TYPE_ID'])) { // column not defined in cct_column
                $this->_setwarn(1, 'CCT_COLUMN', $tabname, $colname, 'APP_DATA_TYPE_ID is not defined in CCT_COLUMN. Assumed AppDataType of \'string\'', 1);
                $_s_i_table[$tabname][$colname]['APP_DATA_TYPE_ID'] = $string_adt_id;
            }
        }
    }
}

function _attribWarn($num, $table, $class_id, $attrib_id, $info ) {
	$this->_setwarn($num, 'EXTRA_ATTRIB', $table.':CLASS_ID:'.$class_id, 'ATTRIB:'.$attrib_id, $info);
}

function init_varcol_cache($LogName, $passwd, $db, $_dbtype)
{
# descr: initalizes the cache of the varcol meta data
# WARNING: relies on filled app_data_type_cache!
# return: void
# error:
#        db_access  1 .. no connecton to db
#                   2 .. error in query
#        init_cache 5 .. datatype-mismatch in extra_attrib <x> used for numeric datatype; warning class 1
#                   6 .. datatype-mismatch in extra_attrib <x> used for date datatype; warning class 1
#                   7 .. datatype-mismatch in extra_attrib <x> used for string datatype; warning class 1
#                   8 .. value in column datatype in extra_attrib invalid; warning class 1

    global $_s_i_varcol, $_s_i_table, $_s_i_app_data_type;

    $error = & ErrorHandler::get();
    
    $sql  = logon_to_db($LogName, $passwd, $db, $_dbtype, 'std', false);
    if ($error->got(CCT_ERROR_READONLY)) return;
    $_s_i_varcol = array();
    
    // temporary code to have both varcol-versions accessible
    $sql->query('SELECT value FROM globals WHERE name = '.$sql->addQuotes('DBVersion'));
    $sql->ReadRow();
    $dbversion = substr($sql->RowData[0], 0, 7);

    $sql->query('SELECT * FROM extra_class'); // get classes
    if ($error->got(CCT_ERROR_READONLY)) return;
    while($sql->ReadArray()) {
        $class_id = $sql->RowData['EXTRA_CLASS_ID'];
        $_s_i_varcol[$class_id] = array();
        $_s_i_varcol[$class_id]['table_name'] = $sql->RowData['TABLE_NAME'];
        $_s_i_varcol[$class_id]['name']       = $sql->RowData['NAME'];
        $_s_i_varcol[$class_id]['comment']    = $sql->RowData['NOTES'] ? $sql->RowData['NOTES'] : 'no comment';
        $_s_i_varcol[$class_id]['nice_name']  = $sql->RowData['NICE_NAME'] ? $sql->RowData['NICE_NAME'] : $sql->RowData['NAME'];
    }
    
    $sql->query('SELECT * FROM extra_attrib ORDER BY extra_class_id, pos'); // get class attributes
    if ($error->got(CCT_ERROR_READONLY)) return;
    while ($sql->ReadArray()) {
        $class_id  = $sql->RowData['EXTRA_CLASS_ID'];
        $attrib_id = $sql->RowData['EXTRA_ATTRIB_ID'];
        $tmpTabName= $_s_i_varcol[$class_id]['table_name'];
        
        $_s_i_varcol[$class_id][$attrib_id] = array();
        $_s_i_varcol[$class_id][$attrib_id]['name'] = $sql->RowData['NAME'];
        $_s_i_varcol[$class_id][$attrib_id]['nice_name'] = $sql->RowData['NICE_NAME'] ? $sql->RowData['NICE_NAME'] : $sql->RowData['NAME'];
        
        $_s_i_varcol[$class_id][$attrib_id]['map_col'] = strtoupper($sql->RowData['MAP_COL']);
               
        $_s_i_varcol[$class_id][$attrib_id]['pos'] = $sql->RowData['POS'];
        $_s_i_varcol[$class_id][$attrib_id]['app_data_type_id'] = $sql->RowData['APP_DATA_TYPE_ID'];
        $_s_i_varcol[$class_id][$attrib_id]['id_col'] = $sql->RowData['ID_COL'];
        $_s_i_varcol[$class_id][$attrib_id]['exim'] = $sql->RowData['EXIM'];
        $_s_i_varcol[$class_id][$attrib_id]['comment'] = $sql->RowData['NOTES'] ? $sql->RowData['NOTES'] : 'no comment';
        if ($sql->RowData['NOT_NULL']>0)     $_s_i_varcol[$class_id][$attrib_id]['NOT_NULL'] = 1;
        if ($sql->RowData['EDITABLE']!=NULL) $_s_i_varcol[$class_id][$attrib_id]['EDITABLE'] = $sql->RowData['EDITABLE'];
        
        $tmptype = $_s_i_app_data_type[$sql->RowData['APP_DATA_TYPE_ID']]['DATA_TYPE'];
        
        
    	if (!isset($_s_i_table['EXTRA_OBJ'][strtoupper($sql->RowData['MAP_COL'])])) {
			$this->_attribWarn(5, $tmpTabName, $class_id, $attrib_id,  'Value (='.$sql->RowData['MAP_COL'].') in column map_col invalid! This column does not exist in table EXTRA_OBJ.');
		} else {
			switch (strtolower(substr($sql->RowData['MAP_COL'], 0, 1))) {
			case 's':
			    if (!($tmptype === 'STRING' or $tmptype === 'DATE' or $tmptype === 'JSON') )
					$this->_attribWarn(6, $tmpTabName, $class_id, $attrib_id, 'Datatype-mismatch! App_data_type with simple datatype of '.$tmptype.' used for data stored as string.');
				break;
			case 'n':
				if ( !(($tmptype === 'FLOAT') or ($tmptype === 'INT')) )
					$this->_attribWarn(7, $tmpTabName, $class_id, $attrib_id, 'Datatype-mismatch! App_data_type with simple datatype of '.$tmptype.' used for data stored as number.');
				break;
			default:
				$this->_attribWarn(8, $tmpTabName, $class_id, $attrib_id, 'Value in column map_col invalid! Allowed values are &quot;s&quot; and &quot;n&quot; + two-digit number');
			}
		}
        
    }
}

/**
 * initalizes the cache of the app data types (table app_data_type)
 * @return: void
 * @throws Exception
 *        db_access 1 no connecton to db
 *        db_access 2 error in query
 * @global $this->warnarr
 */
function init_app_data_type_cache ($LogName, $passwd, $db, $_dbtype)
{
    global $_s_i_app_data_type;

    $error = & ErrorHandler::get();    
    $sql   = logon_to_db($LogName, $passwd, $db, $_dbtype, 'std', false);
    if ($error->got(CCT_ERROR_READONLY)) return;
    $_s_i_app_data_type = array();
    
    $sql->query('SELECT * FROM app_data_type'); // get all app_data_types
    if ($error->got(CCT_ERROR_READONLY)) return;
    
    while ($sql->ReadArray()) {

        $id = $sql->RowData['APP_DATA_TYPE_ID'];
        $_s_i_app_data_type[$id] = array();
        $_s_i_app_data_type[$id]['NAME'] = $sql->RowData['NAME'];
        $_s_i_app_data_type[$id]['DATA_TYPE'] = $sql->RowData['DATA_TYPE'];
        $_s_i_app_data_type[$id]['REG_EXPR'] = $sql->RowData['REG_EXPR'];
        $_s_i_app_data_type[$id]['MAX_LENGTH'] = $sql->RowData['MAX_LENGTH'];
        $_s_i_app_data_type[$id]['MIN'] = $sql->RowData['MIN'];
        $_s_i_app_data_type[$id]['MAX'] = $sql->RowData['MAX'];
        
        
        //question: where is 'PHP_DATA_TYPE' used ?
        switch($sql->RowData['DATA_TYPE']) {
            case 'STRING': $_s_i_app_data_type[$id]['PHP_DATA_TYPE'] = 'string';
                break;
            case 'JSON':   $_s_i_app_data_type[$id]['PHP_DATA_TYPE'] = 'json';
                break;
            case 'INT'   : $_s_i_app_data_type[$id]['PHP_DATA_TYPE'] = 'integer';
                break;
            case 'FLOAT' : $_s_i_app_data_type[$id]['PHP_DATA_TYPE'] = 'double';
                break;
            case 'DATE'  : $_s_i_app_data_type[$id]['PHP_DATA_TYPE'] = 'string';
                break;
            default:       
                $_s_i_app_data_type[$id]['PHP_DATA_TYPE'] = 'string';
                
                $this->_setwarn(9, 'APP_DATA_TYPE', 'DATA_TYPE', '', 'Value in column data_type (id = '.$id
                    .', name = '.$sql->RowData['NAME'].') invalid! Allowed values are STRING, '.
                    ' INT, FLOAT, JSON and DATE');
        }
    }
    
}




function init_cache($LogName, $passwd, $db, $_dbtype)
{
# descr: calls all cache initializing functions.
# return: void
# error:
#        db_access  1 .. no connecton to db
#                   2 .. error in query
#        init_cache 1 .. column not defined in cct_column; warning class 1
#                   2 .. app_data_type 'string' missing. Please contact support.
#                   3 .. table not defined in cct_table; warning class 1
#                   4 .. table defined in cct_table but not existing in database; warning class 1
#                   5 .. datatype-mismatch in extra_attrib <x> used for numeric datatype; warning class 1
#                   6 .. datatype-mismatch in extra_attrib <x> used for date datatype; warning class 1
#                   7 .. datatype-mismatch in extra_attrib <x> used for string datatype; warning class 1
#                   8 .. value in column datatype in extra_attrib invalid; warning class 1
#                   9 .. value in column data_type in att_data_type invalid; warning class 1
#                  10 .. database says column is not nullable but cct_column tells opposite; warning class 1
#                  11 .. column does not exist in database but in cct_column; warning class 1
	global $error;
	global $_s_i_misc;
	$FUNCNAME='init_cache';
    $error = & ErrorHandler::get();
    
    $_s_i_misc=array();

    // echo "\n"; -- do not put NEWLINEs here => XML_RPC does not like it !
    // flush(); - removed, caused double sent headers
    
    // $CacheLib = new gInitCache();
    
    $this->init_app_data_type_cache($LogName, $passwd, $db, $_dbtype); // has to be done first because init_varcol_cache relies on filled data-type-cache
    if ($error->got(CCT_ERROR_READONLY)) return;
    $error->got(); // reset error-flag so that $error->got(READONLY) of db_access.logon_to_db won't get stuck

    $this->init_table_cache($LogName, $passwd, $db, $_dbtype); // has to be done second because init_varcol_cache relies on filled table-cache
    if ($error->got(CCT_ERROR_READONLY)) return;
    $error->got(); // reset error-flag so that $error->got(READONLY) of db_access.logon_to_db won't get stuck

    $this->init_varcol_cache($LogName, $passwd, $db, $_dbtype);
    if ($error->got(CCT_ERROR_READONLY)) return; // do not care about warnings
    $error->reset(); // reset error-flag so that $error->got(READONLY) of db_access.logon_to_db won't get stuck

    $numwarns = $this->warnGot();
    
	if ( $numwarns>0 ) {
		$error->set( $FUNCNAME, 1, $numwarns.' warnings occured.',1  ); // only warning
	}
}

}