<?php
/**
 * main functions for objects
 * - look also for:
 *   -  object.info.inc (ONE object action)
 *   -  table.info.inc  (ONE table actions)
 * @package object.subs.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */

/**
 * get some attributes of one CCT_ACCESS object
 * @param object $sql
 * @param int $cct_access_id
 * @param int $format [0], 1,2,3,... see db_access.inc
	default: user_output_format    : YYYY-MM-DD HH24:MI:SS
	1 for partisan_exchange_format : YYYY-MM-DD HH24:MI:SS
	2 for user_input_format        : DD-MON-YY
	3 easy date 			        : DD-MM-YYYY
	4 minutes						: YYYY-MM-DD HH24:MI
 * @return array $ret
		'owner_id'
		'owner'
		'crea_date'
		'mod_date'
		'modifier_id',
		'modifier'
		'h_log_name'
		'wflow_id'
 */
function access_data_getai( &$sql, $cct_access_id, $format=NULL ) {
    if ( !$cct_access_id ) {
        return (NULL);
    }
	
    $ret = NULL;
	$addSql = NULL;
	$addSql .= ", a.H_ALOG_ACT_ID";
	
	if (glob_column_exists("CCT_ACCESS", "WFLOW_ID")) {
		$addSql .= ", a.WFLOW_ID";
	}
	
    $sql->query('SELECT u.db_user_id, u.nick, '.$sql->Sql2DateString('a.crea_date', $format).
				' crea_date, '.$sql->Sql2DateString('a.mod_date',$format ).
				' mod_date, a.db__db_user_id '.$addSql.
                '  FROM cct_access a, db_user u '.
                '  WHERE a.cct_access_id = '.$cct_access_id.' AND a.db_user_id = u.db_user_id');

    $sql->ReadRow();
	$ret['owner_id']  = $sql->RowData[0];
    $ret['owner']     = $sql->RowData[1];
    $ret['crea_date'] = $sql->RowData[2];
    $ret['mod_date']  = $sql->RowData[3];
	$ret['modifier_id']=$sql->RowData[4];
	
    $tmpmodifier      = $sql->RowData[4];
	$H_ALOG_ACT_ID    = $sql->RowData[5];
	$ret['wflow_id']  = $sql->RowData[6];

    if ($tmpmodifier) {
		if ($tmpmodifier!=$ret['owner_id']) { // try to save a SQL-action
			$sql->query('SELECT u.nick '.
						' FROM db_user u '.
						' WHERE u.db_user_id='.$tmpmodifier);
	
			$sql->ReadRow();
			$ret['modifier'] = $sql->RowData[0];
		} else {
			$ret['modifier'] = $ret['owner'];
		}
    }
	if ( $H_ALOG_ACT_ID ) {
		$sql->query("SELECT NAME from H_ALOG_ACT where H_ALOG_ACT_ID=".$H_ALOG_ACT_ID, $FUNCNAME.":H_ALOG_ACT_ID");
		$sql->ReadRow();
		$ret['h_log_name'] = $sql->RowData[0];
	}
	
    return $ret;
	
}

// get params from access_data_getai() by ($tablename, $pkname, $id)
function access_data_get(&$sql, $tablename, $primname, $id) {
    $sql->query('SELECT cct_access_id FROM '.$tablename.' WHERE '.$primname.'='.$sql->addquotes($id));
    $sql->ReadRow();
    return ( access_data_getai($sql, $sql->RowData[0]) );
}

/**
 * get cct_access_id of object
 * @param  $sqlo
 * @param  $tablename
 * @param  $id
 * @return $cct_access_id
 */
function getAccessId(&$sqlo, $tablename, $id) {
	$primname = PrimNameGet2($tablename);
    $sqlo->Quesel('cct_access_id FROM '.$tablename.' WHERE '.$primname.'='.$sqlo->addquotes($id));
    $sqlo->ReadRow();
    return ( $sqlo->RowData[0] );
}

/**
 * get column names: WARNING: no order info!
 * @param $tablename
 * @return array $colarray ... array(COLNAMES)
 */
function columns_get2($tablename) {
  global $_s_i_table;

  $ret = array();
  if (isset($_s_i_table[$tablename])) {    
	
	foreach( $_s_i_table[$tablename] as $colName=>$dummy ) {
	  if (strcmp($colName, '__table_data__')) // strcmp returns 0 on equal!
		$ret[] = $colName;
	}
  }
  return $ret;
}

/**
 * get column names, sorted by POS (read from CACHE)
 * @param $tablename
 * @return array $colarray ... array(COLNAMES)
 */
function columns_get_pos($tablename) {
  global $_s_i_table;

  $ret = array();
  $ret2= array();
  
  if (isset($_s_i_table[$tablename])) {
	
      foreach( $_s_i_table[$tablename] as $colName=>$data ) {
	  if (strcmp($colName, '__table_data__')) { // strcmp returns 0 on equal!
		$pos = $data["POS"];
        if ( empty($ret2[$pos]) ) $ret2[$pos] = $colName;  // POS is not missing ???
        else  $ret2[] = $colName; // append
      }
	}  
    ksort($ret2); // key sort
    
    // recreate index (start with 0)
    foreach( $ret2 as $col ) {
        $ret[] = $col;
    }
  }
  return $ret;
}

/**
 * get database-RAW-types of columns
 * @param string $tablename
 * @return array [COL]=TYPE
 */    
function appRawTypeGet(&$sqlo,$tablename) {
  $sqlsel = "COLUMN_NAME, DATA_TYPE from CCT_COL_VIEW where ".
      " TABLE_NAME=".$sqlo->addQuotes($tablename). " order by COLUMN_NAME";
  $sqlo->Quesel($sqlsel);
  $outarr=array();
  while ( $sqlo->ReadRow() ) {
      $outarr[$sqlo->RowData[0]]=$sqlo->RowData[1];
  }
  return $outarr;
}

// get app-data-type-id for tablename,column
function appDataTypeGet2($tablename, $colName) {
  global $_s_i_table;
  return isset($_s_i_table[$tablename][$colName]['APP_DATA_TYPE_ID']) ? $_s_i_table[$tablename][$colName]['APP_DATA_TYPE_ID'] : 0;
}

// get app-data-type-name from id
function appDataTypeNameGet2($app_data_type_id) {
  global $_s_i_app_data_type;

  if ($app_data_type_id > 0) {
	return $_s_i_app_data_type[$app_data_type_id]['NAME'];
  }
  return '';
}


// get app-data-type NAME for (tablename,column)
function appDataTypeNGet($tablename, $colName) {
    global $_s_i_table, $_s_i_app_data_type;
    $type_id =  isset($_s_i_table[$tablename][$colName]['APP_DATA_TYPE_ID']) ? $_s_i_table[$tablename][$colName]['APP_DATA_TYPE_ID'] : 0;
    if (!$type_id) return '';
    return $_s_i_app_data_type[$type_id]['NAME'];
}

/**
 * get app-data-type ARRAY from id
 * @param int $app_data_type_id
 * @return mixed|array
 * 'NAME'      - name of app type
   'DATA_TYPE'
   'REG_EXPR'
   'MAX_LENGTH'
   'MIN'
   'MAX'

 */
function appDataType_arr( $app_data_type_id ) {
    global $_s_i_app_data_type;
    
    if ($app_data_type_id > 0) {
        return $_s_i_app_data_type[$app_data_type_id];
    }
    return array();
}


/**
 * computes the app_data_type.id from the app_data_type.name
 * @param $app_data_type_name
 * @return $adt_id
 */
function get_app_data_type_id (      $app_data_type_name) {
  global $_s_i_app_data_type;

 
  foreach( $_s_i_app_data_type as $adt_id=> $adt_data ) {
	if ($adt_data['NAME'] === $app_data_type_name)
	  return $adt_id;
  }
  return 0;
}

function column_query_get( $sql, $tablename) {
  $sel_str_tmp = implode(', ', columns_get2($tablename));

  return (empty($sel_str_tmp) ? '*' : $sel_str_tmp);
}

/**
 * get object-name of ABSTRACT object of this concrete object with $tablename, $id
 * @param $sql
 * @param $tablename
 * @param $id
 * @param $objname
 * @return string name of object
 */
function gAbsObjName( &$sql, $tablename, $id, $objname ) {
			
	switch ($tablename) {
		case "CONCRETE_PROTO":
			$sqls="select a.name from CONCRETE_PROTO c, ABSTRACT_PROTO a ".
			" where c.CONCRETE_PROTO_ID=".$id. " AND c.ABSTRACT_PROTO_ID=a.ABSTRACT_PROTO_ID";
			break;
		case "CONCRETE_SUBST":
			$sqls="select a.name from CONCRETE_SUBST c, ABSTRACT_SUBST a ".
			" where c.CONCRETE_SUBST_ID=".$id. " AND c.ABSTRACT_SUBST_ID=a.ABSTRACT_SUBST_ID";
			break;
		default:
			return;
	}
	$sql->query($sqls);
	$sql->ReadRow();
	$name = $sql->RowData[0];
	return ($name);
}

/**
 * get most important name (usually coumn NAME) of an object (SYS or BO)
 * - if object not exists: returns: "[ID:$id]  NOT FOUND!"
 * @param  $sql
 * @param  $tablename
 * @param  $id
 * @param  $opt // "absName" : 0,1 -- with abstract name
				// "noID"    : do not return the ID, if name is empty
 */
function obj_nice_name(&$sql, $tablename, $id, $opt=NULL ) {

  $nice_colname   = importantNameGet2($tablename);
  $tmp_prim_name  = PrimNameGet2($tablename);
  $nicename   = "[ID:$id]";
  if ($id===NULL or $id==='') {
      return ('ID:None');
  }
  
  if ($nice_colname != '') {
    $sql->query('SELECT '.$nice_colname.' FROM '.$tablename.' WHERE '.$tmp_prim_name.' = '.$sql->addquotes($id) );
    if ($sql->ReadRow()) {
		$nicename = $sql->RowData[0];
		if ($nicename=="" AND !$opt["noID"] )  $nicename = "[ID:$id]";
		if ($opt["absName"]) {
			$absName = gAbsObjName( $sql, $tablename, $id, $nicename );
			if ($absName!="")   $nicename .= " (".$absName.")";
		}
	} else {
		$nicename .= " NOT FOUND!";
	}
  }
  return ($nicename);
}

/**
 * helper function for colFeaturesGet
 * @return $ret as reference : 'NICE_NAME', 'VISIBLE'
 */ 
function _colFeatExpDesc (&$sql, $table, $desc_id, $map_col, &$ret) {
	$sql->query('SELECT name FROM H_EXP_RAW_DESC_COL where H_EXP_RAW_DESC_ID = '.
		$desc_id.' and '.
		' TABLE_NAME='.$sql->addQuotes($table).
		' and map_col = '.$sql->addQuotes($map_col));
	if (!$sql->ReadRow() or !$sql->RowData[0]) $ret['VISIBLE'] = 0;
    else $ret['NICE_NAME'] = $sql->RowData[0];
}

// function tablePkNumGet( &$sql, $tablename ) {
// removed, duplicate, use countPrimaryKeys in globals.inc alternatively

/**
 * - get features from CCT_COLUMN
 * @return array $features column features
 * @param object $sql -- can be NULL; if this is not a spacial EXP_result table 
 * @param object $tablename e.g. "EXP"
 * @param object $colName column name e.g. "EXP_ID"
 * @param object $h_exp_raw_desc_id[optional] need for dynamic columns RAW_RESULT
 */
function colFeaturesGet( &$sql, $tablename, $colName, $h_exp_raw_desc_id=0 ) { 
  global $_s_i_table;

  $ret = $_s_i_table[$tablename][$colName]; // make a copy!
  
  if (!$h_exp_raw_desc_id) return $ret;     // no special column names
   
  /**
   * - now handle specials for experiment result tables, it changes $ret['NICE_NAME'] and $ret['VISIBLE']
   * - the nice column names are defined by a mapping using table H_EXP_RAW_DESC_COL
   * - each experiment contains a link to H_EXP_RAW_DESC, which describes the nice names of the columns
   * - also used for VIEWS like "EXP_RAW_SPOT_RESULT"
   */
  if ($ret["CCT_TABLE_NAME"]!=NULL) return $ret;   // link to foreign object, leave "NICE"
  
  if ($tablename=="EXP_RAW_RESULT" or $tablename=="EXP_RAW_SPOT_RESULT" ) {  // TBD: ignores VIEWS !!!
    if ( (strlen($colName) == 3) ) { // the matching columns have exactly 3 letters!
        _colFeatExpDesc ($sql, "EXP_RAW_RESULT", $h_exp_raw_desc_id, $colName, $ret);    
    }
  }
  
  if ($tablename=="SPOT_RESULT" or $tablename=="EXP_SPOT_RESULT") {
     if ( $colName!="SPOT_ID" ) {
        _colFeatExpDesc ($sql, "SPOT_RESULT", $h_exp_raw_desc_id, $colName, $ret);
    }
    
  } 
  
  if ($tablename=="EXP_CHAN_RES") { 
    if ( strlen($colName) == 3 ) { // the matching columns have exactly 3 letters!
        _colFeatExpDesc ($sql, $tablename, $h_exp_raw_desc_id, $colName, $ret);
    }
    
  }
      
  return $ret;
} 

/**
 * get raw features from CCT_COLUMN, nice name not corrected for EXP_RAW_RESULT and Co.
 * @return array $features
 * @param object $tablename
 * @param object $colName
 */
function colFeaturesGet2($tablename, $colName ) { 
  global $_s_i_table;
  return $_s_i_table[$tablename][$colName];
}

/**
 * get all parameters from BO:EXTRA_OBJ 
 * @param  $sql
 * @param  $table tablename of object
 * @param  $id    id of object
 * @param  $secid ... second PK (but no BO is using this ...
 * @return $vals = array (
	//			 "class_name" =>
	//			 "extra_class_id" =>
	//			 "values"     => 
	//			 "extra_obj_id" =>
	//			)
 */
/* DEPRECATED: use objInfoC::class_params()
 * 
function objClassParamsGet( &$sql, $table, $id, $secid=NULL  ) {
    global $error, $varcol; 
    
    $error = & ErrorHandler::get();
    $varcol = & Varcols::get();  
    
    $vals = NULL; 
    $pkname =  $pk_name = $table."_ID";
    $sqls = "select extra_obj_id from ".$table." where ".$pkname."=".$id;
    $sql->query($sqls); 
    if ( $sql->ReadRow() ) { 
        $extra_obj_id = $sql->RowData[0];
        if ($extra_obj_id) {    
            $vals =  $varcol->select_by_name ($extra_obj_id);  
			$vals["extra_obj_id"] = $extra_obj_id;
            if ( $vals["extra_class_id"] ) { 
                $sqls= "select name from EXTRA_CLASS where TABLE_NAME='".$table."' AND EXTRA_CLASS_ID=".$vals["extra_class_id"];
                $sql->query("$sqls");
                $sql->ReadRow();
	            $tmpname = $sql->RowData[0];  
                $vals["class_name"] = $tmpname;
            }
        }
    }  
    
    return ($vals);
} 
*/  

/**
 * CALL WITH & IN FROM OF FUNCTION NAME!!!
 * get the tables wich have $tablename as mothertable (views are ordered last)
 * @param  $tablename
 * @return array  array( [table_name =>
            'nice_name'  => ...
            'is_view'    => ...
            'is_bo'      => ...
			'is_intern' => ...
       )
        or false on error;
 */
function &get_assoc_tables($tablename) {
	global $_s_i_table;
	
	$ret = NULL;
	$i   = 0;
	
	foreach( $_s_i_table as $a_tab=> $a_tab_data ) {
	
		if ($a_tab_data['__table_data__']['CCT_TABLE_NAME'] == $tablename) {
		
			$tmparray = &$a_tab_data['__table_data__'];
			$ret[$a_tab] = array( 
				'nice_name'  => $tmparray['NICE_NAME'],
				'is_view'    => $tmparray['IS_VIEW'],
				'is_bo'      => $tmparray['IS_BUSINESS_OBJECT'],
				'is_intern'  => $tmparray['INTERNAL']
				);
			$i++;
		}
	}
	
	if ($ret) { // not empty array
		ksort($ret);
		return ($ret);
	}
}

/**
 * get all ASSOC-tables; VIEWS are ignored
 * @param  $sql
 * @param  $tablename
 * @return array( array(CODE name of table, NICE name of table) )
 */
function get_assoc_tables2( &$sql, $tablename ) {
    $ret = NULL;
    $sqls = "select TABLE_NAME, NICE_NAME from CCT_TABLE where CCT_TABLE_NAME='".$tablename."' AND IS_VIEW!=1";
    $sql->query($sqls);
    while ( $sql->ReadRow() ) {
        $tmp_table = $sql->RowData[0];
        $tmp_nice  = $sql->RowData[1];
        $ret[] = array($tmp_table,$tmp_nice);
    } 
    return ($ret); 
}

/**
 * check, if table is defined by CCT_TABLE; if the CACHE is active you can also use glob_table_exists($tablename)
 * @param $sql
 * @param $tablename
 * @return int 0,1
 */
function table_exists( &$sql, $tablename ) { 
	$ret = NULL;
    $sqls = "select TABLE_NAME, IS_VIEW from CCT_TABLE where TABLE_NAME='".$tablename."'";
    $sql->query($sqls);
    if ($sql->ReadRow() ) {
        $tmp_table = $sql->RowData[0];
        $tmp_view  = $sql->RowData[1];
        $ret = array($tmp_table,$tmp_view);
    } 
    return ($ret);

}

/**
 * checks if an entry in the database exists (works only for single-PK objects)
 * - throws error, if id is NUMERIC, if the primary key is of data-type='id'
 * @param object $sql
 * @param string $tablename
 * @param string|int $id ... ID of object
 */
function gObject_exists(&$sql, $tablename, $id) {
  $pkname = PrimNameGet2($tablename);
  $type_name=appDataTypeNGet($tablename,$pkname);
  if ($type_name=='id') {
      if (!is_numeric($id)) {
          throw new Exception('t:'.$tablename.', ID ('.$id.') is not numeric!');
      }
  }
  $sql->query('SELECT 1 FROM '.$tablename.' WHERE '.$pkname."='".$id."'");
  return $sql->ReadRow();
}

/**
 * checks if an entry in the database exists (works for bos and assoc-tables)
 * @param  $sql
 * @param  string $tablename
 * @param  array $pk_array ... ZERO-based array containing primary-key-values
 * @return int 0,1
 */
function gElement_exists(&$sql, $tablename, $pk_array) {
  $sql->query('SELECT 1 FROM '.$tablename.' WHERE '.primary_select_str2($sql, $tablename, $pk_array));
  return $sql->ReadRow();
}
/**
 * checks if an entry in the database exists
 * @param  $sql
 * @param  string $tablename
 * @param  array $pk_array ... KEY=>VALUE
 * @return int 0,1
 */
function gElement_exists2(&$sql, $tablename, $pk_array) {
    $pk_str='';
    $and='';
    foreach($pk_array as $key=>$val) {
        $pk_str .= $and . $key.'='.$sql->addQuotes($val);
        $and=' AND ';
    }
    $sql->query('SELECT 1 FROM '.$tablename.' WHERE '.$pk_str);
    return $sql->ReadRow();
}

/**
 * TBD: exchange thios by gElement_exists2
* checks if an entry in the database exists (works for bos and assoc-tables)
* @param array $pk_array KEY-based array containing primary-key-values
* @return true/false
*/
function gElement_exist2(&$sql, $tablename, $pk_array) {

    $pks=array();
    foreach( $pk_array as $key=> $val) {
		$pks[] = $key.' = '.$sql->addQuotes($val);
    }

	$where = implode (' AND ', $pks);
	
	$sql->query('SELECT 1 FROM '.$tablename.' WHERE '.$where);
	return $sql->ReadRow();
}

/**
 * get data of all columns $outColArr from table, where PK=$obj_id
 * @param  string $tablename
 * @param  string $obj_id
 * @param  array $outColArr -- column names; All columns: array('*')
 * @return $value value of the $outputCol
 */
function glob_ObjDataGet( &$sql, $tablename, $obj_id, $outColArr) {
    $pk_name   = PrimNameGet2($tablename);
    if ($pk_name==NULL) {
        throw new Exception('table:'.$tablename.' has no defined PK.');
    }
    $outputCol = implode(',', $outColArr);
    $sqls = "select ".$outputCol." from ".$tablename." where ".$pk_name."=".$sql->addQuotes($obj_id);
    $sql->query($sqls);
    $sql->ReadArray();
    return ( $sql->RowData );
}

/**
 * get data of ONE column $outputCol from table, where PK=$obj_id
 * @param  string $tablename
 * @param  string $obj_id
 * @param  array $outColArr -- column names; All columns: array('*')
 * @return $value value of the $outputCol
 */
function glob_ObjDataColGet( &$sql, $tablename, $obj_id, $outputCol) {
    $pk_name   = PrimNameGet2($tablename);
    if ($pk_name==NULL)    throw new Exception('table:'.$tablename.' has no defined PK.');
    if ($outputCol==NULL)  throw new Exception('Input missing: outputCol.');
    $sqls = $outputCol." from ".$tablename." where ".$pk_name."=".$sql->addQuotes($obj_id);
    $sql->Quesel($sqls);
    $sql->ReadRow();
    return ( $sql->RowData[0] );
}


/**
 * get data of ONE column from table, where $key = $val
 * @param  string $tablename
 * @param  string $outputCol
 * @return $value value of the $outputCol
 */
function glob_elementDataGet( &$sql, $tablename, $key, $val, $outputCol) {
	$sqls = "select ".$outputCol." from ".$tablename." where ".$key."=".$sql->addQuotes($val);
	$sql->query($sqls);
	$sql->ReadRow();
	return ( $sql->RowData[0] );
}

/**
 * - get object-ID by object-name
 * - just works fine, if  NAME is unique in table (e.g. for SYSTEM tables)
 * @param string $name name of object
 * @return $objectid
 */
function glob_getObjIdByName( &$sqlo, $tablename, $name) {
	global $error;
	$FUNCNAME= 'glob_getObjIdByName';
	
	$pkname  = PrimNameGet2($tablename);
	$nameCol = importantNameGet2($tablename);
	if ($pkname=='' or $nameCol=='') {
		$error->set( $FUNCNAME, 1, 'Missing sys-info for table '.$tablename );
		return;
	}
	$sqlsel = $pkname.' from '.$tablename.' where '.$nameCol.'='.$sqlo->addQuotes($name);
	$sqlo->Quesel($sqlsel);
	$sqlo->ReadRow();
	return ( $sqlo->RowData[0] );
}

/**
 * 
 * get data of ONE column from table, where condition in $whereArr, result column-name in $outputCol
 * @param  array  $whereArr  array(COLNAME=>value)
 * @param  string $outputCol
 * @return $value value of the $outputCol
 */
function glob_elementDataGet2( &$sql, $tablename, $whereArr, $outputCol ) {
	$sqls  = "select ".$outputCol." from ".$tablename." where ";
	$tmpand="";
	
	foreach( $whereArr as $key=>$val ) {
		$sqls .= $tmpand . $key."=".$sql->addQuotes($val);
		$tmpand=" AND ";
	}
	
	$sql->query($sqls);
	$sql->ReadRow();
	return ( $sql->RowData[0] );
}


/**
 * get one data-row from table, where condition in $whereArr, result columns in $outColArr
 * @param  array $whereArr  array(COLNAME=>value)
 * @param  array $outColArr array(OUT_COLNAMES)
 * @return array $valuearr  array(OUT_COLNAME=>value)
 */
function glob_elemDataGet3( &$sql, $tablename, $whereArr, $outColArr ) {
	
	$outputCol = implode(',', $outColArr);
	$sqls  = "select ".$outputCol." from ".$tablename." where ";
	$tmpand= NULL;
	foreach( $whereArr as $key=>$val ) {
		$sqls .= $tmpand . $key."=".$sql->addQuotes($val);
		$tmpand=" AND ";
	}
	
	
	$sql->query($sqls);
	$sql->ReadArray();
	return ( $sql->RowData );
}


/**
 * general object methods
 *
 */
class gObj {
    
    // check if text is an ID
    // 0: string
    // 1: number
    static function is_ID(string $text) {
        $output=0;
        if (is_numeric($text)) {
            if (strval(intval($text))==$text) {
                $output=1;
            }
        }
        return $output;
    }
}

