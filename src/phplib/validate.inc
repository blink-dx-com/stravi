<?php
require_once ('db_access.inc');
require_once ('globals.inc');
require_once ('object.subs.inc');

/**
 * validates user's input against app_data_type
 * @namespace core::
 * @author  Michael Howitz (mac@clondiag.com)  20021024
 * @author  Steffen Kube
 * @package validate.inc
 */
class fValidate {

/**
 * - validates the values + checks for not-null columns & unique indexes according to cct_column
 * - WARNING: this function is not intended to use for varcols! use validate_extra_values() instead
 * @return array $dtypes data type names
 * @param object $sql
 * @param string $table name of the table where the columns belong to
 * @param array $argu   array(<COLUMN_NAME> => <column_value>, ...)
 * @param bool $doing_update validate columns for update, so not-null-columns needt not be existing
 */
function check_columns (  &$sql,       $table,     &$argu,     $doing_update) {
    global $_s_i_table, $error;
    $error = & ErrorHandler::get();
    $FUNCNAME='validate';

    if (!isset($_s_i_table[$table])) { // check for wrong tablename
       $error->set($FUNCNAME, 22, 'Table &quot;'.$table.'&quot; does not exist!');
       return;     
    }

  
    $pkey = PrimNameGet2($table);
    $pkey_may_be_null = (countPrimaryKeys($table) == 1) && (appDataTypeNameGet2(appDataTypeGet2($table, $pkey)) === 'id') && ($table !== 'app_data_type');

  
    foreach( $_s_i_table[$table] as $col=>$col_data ) {
   
        // check for not_null
        if ($col === '__table_data__') continue;
        if ($pkey_may_be_null  and ($col === $pkey)) continue;
	
        if (!$doing_update && $col_data['NOT_NULL'] && !isset($argu[$col])) {
            if (cct_access_has2($table) && $col === 'CCT_ACCESS_ID') continue;
            $error->set($FUNCNAME, 20, 'Column &quot;'.columnname_nice2($table, $col).'&quot; in table &quot;'.tablename_nice2($table).'&quot; must not be empty!');	  
            continue;
        }
        if ($col_data['NOT_NULL'] and
            isset($argu[$col])    and
            (($argu[$col] === '') or is_null($argu[$col]))
            ){
            $error->set($FUNCNAME, 20, 'Column &quot;'.columnname_nice2($table, $col).'&quot; in table &quot;'.tablename_nice2($table).'&quot; must not be empty!');	  
            continue;
        }

        // check unique index
        if (!$doing_update && $col_data['UNIQUE_COL'] && !empty($argu[$col])) {
            $sql->query('SELECT 1 FROM '.$table.' WHERE '.$col.' = '.$sql->addQuotes($argu[$col]));
            if ($error->got(READONLY)) continue;
            if ($sql->ReadRow())
                $error->set($FUNCNAME, 21, 'Column &quot;'.columnname_nice2($table, $col).'&quot; in table &quot;'.tablename_nice2($table).'&quot; must be unique! &quot;'.$argu[$col].'&quot; already exists.');
        }
    }

	
	$dtypes = NULL;
    // validate existing columns
    foreach($argu as $col => $val) {
        $adt_id   = appDataTypeGet2($table, $col);
		$adt_name = NULL;
        $tn_nice = 'table &quot;'.tablename_nice2($table).'&quot;';
        $cn_nice = columnname_nice2($table, $col);
        if ($adt_id) {
            $adt_name = $this->_check_datatype($tn_nice, $cn_nice, $val, $adt_id);
            $this->_check_regexpr ($tn_nice, $cn_nice, $val, $adt_id);
            $this->_check_maxlen  ($tn_nice, $cn_nice, $val, $adt_id);
            $this->_check_minmax  ($tn_nice, $cn_nice, $val, $adt_id);
        } else {
            $error->set($FUNCNAME, 2, 'App data type of column &quot;'.$cn_nice.'&quot; in table &quot;'.tablename_nice2($table).'&quot; not defined. Please inform your Admin.');
        }
        
        $col_feats = &$_s_i_table[$table][$col]; 
        if ($val!=NULL) {
            // check if FOREIGN object exists
            if ($col_feats['CCT_TABLE_NAME']!='') {
                $ftab = $col_feats['CCT_TABLE_NAME'];
                if (is_numeric($val)) { // previous App-Data-Error did not break the loop ...
                    if ( !gObject_exists ($sql, $ftab, $val) ) {
                        $error->set($FUNCNAME, 23, 'Column "'.$cn_nice.'": Object not found in foreign-table "'.tablename_nice2($ftab).'".');
                    }
                }
            }
        }
        
		$dtypes[$col] = $adt_name;
    }
	return ($dtypes);
}

function val_getTypes($table, $colnames) {
	global $_s_i_app_data_type;
	$dtypes = NULL;
    // validate existing columns
	foreach( $colnames as $col ) {
        $adt_id = appDataTypeGet2($table, $col);
		$dtype  = $_s_i_app_data_type[$adt_id]['DATA_TYPE'];
		$dtypes[$col] = $dtype;
    }
	return ($dtypes);
}

/**
 * check ISO-Date
 * @param  $val
 * @param  int $opt
 * 	[0] : YYYY-MM-DD
 *  [2] : YYYY-MM-DD HH:MM
 *  [3] : YYYY-MM-DD HH:MM:SS
 */
static function _DATE_iso($val, $opt=0) {
	
	switch ($opt) {
		case 2:
			if (strlen($val)>19) return 0; // check length; but allow also seconds here
			if ( preg_match('/^([1-9][0-9][0-9][0-9])-(0[1-9]|10|11|12)-(0[1-9]|[12][0-9]|30|31) ([0-9][0-9]):([0-9][0-9])/', $val)) return 1;
			else return 0;
		case 3:
			if (strlen($val)>19) return 0;
			if ( preg_match('/^([1-9][0-9][0-9][0-9])-(0[1-9]|10|11|12)-(0[1-9]|[12][0-9]|30|31) ([0-9][0-9]):([0-9][0-9]):([0-9][0-9])/', $val)) return 1;
			else return 0;
		default:
			if ( preg_match('/^([1-9][0-9][0-9][0-9])-(0[1-9]|10|11|12)-(0[1-9]|[12][0-9]|30|31)/', $val)) return 1;
			else return 0;
	}
}

#   void extra_values (int extra_class_id, array values)
function check_extra_values (   $extra_class_id,     &$values) {
# descr: validates the values for an extra object
# input: extra_class_id ... id of the class of the values
#        values         ... array(<attrib_id> => <attrib_value>, ...)
# return: void, errors through error_handler
# errors:
#         validate 31 .. class with extra_class_id does not exist
#                  32 .. attribute does not belong to class
#                  33 .. app data type of extra attribute not defined
#     * @throws validate  1 unknown native datatype
#     * @throws validate 10 data in column not of needed type
#     * @throws validate 11 data in column not matching regular expression
#     * @throws validate 12 data in column longer than allowed
#     * @throws validate 13 data in column out of range

    global $_s_i_varcol;
	$FUNCNAME='validate';
    $error = & ErrorHandler::get();

    if (!isset($_s_i_varcol[$extra_class_id])) { // check for wrong extra_class_id
          $error->set($FUNCNAME, 31, 'Class for extra_class_id = &quot;'.$extra_class_id.'&quot; does not exist!');
          return;      
    }

    // validate existing attributes
    foreach( $values as $attrib_id => $value ) {
        if (!array_key_exists($attrib_id, $_s_i_varcol[$extra_class_id])) {
            $this->_error->set($FUNCNAME, 32, 'Attribute with id = &quot;'.$attrib_id.'&quot; does not belong to class &quot;'.$_s_i_varcol[$extra_class_id]['nice_name'].'&quot; (class_id = '.$extra_class_id.')');
        }

        $adt_id = $_s_i_varcol[$extra_class_id][$attrib_id]['app_data_type_id'];
        $class_nice = 'class &quot;'.$_s_i_varcol[$extra_class_id]['nice_name'].'&quot;';
        $attrib_nice = $_s_i_varcol[$extra_class_id][$attrib_id]['nice_name'];
        if ($adt_id) {
            $this->_check_datatype($class_nice, $attrib_nice, $value, $adt_id);
            $this->_check_regexpr ($class_nice, $attrib_nice, $value, $adt_id);
            $this->_check_maxlen  ($class_nice, $attrib_nice, $value, $adt_id);
            $this->_check_minmax  ($class_nice, $attrib_nice, $value, $adt_id);
        } else {
            $error->set($FUNCNAME, 33, 'App data type of extra attribute &quot;'.$attrib_nice.'&quot; in '.$class_nice.' not defined. Please inform your Admin.');
        }
    }
}

/**
 * check, if value is unique in context
 * @param int $extra_class_id
 * @param string $attrName
 * @param string $value
 * @return $isUnique
 *  1 : o.k.
 * 	< 0 error
 */
function check_xobj_col_unique ( &$sqlo, $table, $extra_class_id, $attrib_name, $value) {
	global $varcol;
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	$mapcol = $varcol->attrib_name_to_mapcol($attrib_name, $extra_class_id);
	if ($mapcol==NULL) {
		$error->set( $FUNCNAME, 1, 'attribute "'.$attrib_name.'" unknown' );
		return;
	}
	
	$sqlsel = 'distinct(*) from '.$table.' x join EXTRA_OBJ o on x.EXTRA_OBJ_ID=o.EXTRA_OBJ_ID where EXTRA_CLASS_ID='.$extra_class_id.
		' and o.'.$mapcol.'='. $sqlo->addQuotes(trim($value));
	$sqlo->Quesel($sqlsel);
	$sqlo->ReadRow();
	$numFound = $sqlo->RowData[0];
	
	if (!$numFound) return 1;
	else return -1;
}

/**
 * helper function of validate_values to check datatype
 * @return string $dtype
 * @param object $table
 * @param object $col
 * @param object $val
 * @param int $adt_id
 */
function _check_datatype($table, $col, $val, $adt_id) {
    global $_s_i_app_data_type;

    $error = & ErrorHandler::get();
    $okay  = 0;
    $dtype = $_s_i_app_data_type[$adt_id]['DATA_TYPE'];
    
    $DATE_expect='DD-MON-YYYY';
    if ( $_SESSION['globals']['db.dateFormat']=='YYYY-MM-DD' ) $DATE_expect='YYYY-MM-DD';
  
    if (($val === '') or is_null($val)) { // empty strings are used for NULL-values
        $okay = 1;
    } else {
        switch ($dtype) {
            case 'FLOAT':  if (is_numeric($val)) $okay = 1; break;
            case 'INT':    if (is_numeric($val) and (intval($val)) == doubleval($val)) $okay = 1; break;
            case 'DATE': 
            	if ($DATE_expect=='DD-MON-YYYY')  
            		if (preg_match('/^([1-9]|0[1-9]|[12][0-9]|30|31)-(JAN|FEB|MAR|APR|MAY|JUN|JUL|AUG|SEP|OCT|NOV|DEC)-([0-9]{2,4})/i', $val)) $okay = 1; 
            	if ($DATE_expect=='YYYY-MM-DD')  
            		if ($this->_DATE_iso($val)) $okay = 1;
            	break;
    		case 'DATE2':  if ($this->_DATE_iso($val)) $okay = 1; break;
            case 'DATE1':  if ($this->_DATE_iso($val)) $okay = 1; break;
    		case 'STRING': $okay = 1; break;
    		case 'JSON':
    		    $answer = json_decode($val, TRUE);
    		    if ($answer!==NULL)  $okay=1;
    		    break;
            case 'BLOB':   $okay = 1; break;
            default: $error->set('validate', 1, 'Unknown native Datatype ("'.$dtype.'") of app_data_type (id='.$adt_id.'). Please inform your Admin.');
        }
    }
  
    if (!$okay)
		switch ($dtype) {
        	case 'DATE':
            	$error->set('validate', 10, 'Value for column &quot;'.$col.'&quot; in '.$table.' is no date. Expected format is '.$DATE_expect.'.');
        		break;	
			case 'DATE2':
            	$error->set('validate', 10, 'Value for column &quot;'.$col.'&quot; in '.$table.' is no date2. Expected format is YYYY-MM-DD.');
        		break;	
			default:
            	$error->set('validate', 10, 'Value for column &quot;'.$col.'&quot; in '.$table.' is not of type '.$dtype.'.');
		}
		
	return ($dtype);
}

function _check_regexpr($table, $col, $val, $adt_id) {
#descr: helper function of validate_values to check regular expression
    global $_s_i_app_data_type;

    $error    = & ErrorHandler::get();
    $okay     = 0;
    $reg_expr = $_s_i_app_data_type[$adt_id]['REG_EXPR'];
  
    if ((strlen($reg_expr) > 0) and (strlen($val) > 0) and !preg_match('/'.$reg_expr.'/i', $val))
        $error->set('validate', 11, 'Value for column &quot;'.$col.'&quot; in '.$table.' does not match regular expression for '.appDataTypeNameGet2($adt_id).'.');
}

function _check_maxlen($table, $col, $val, $adt_id) {
#descr: helper function of validate_values to check maximum length
    global $_s_i_app_data_type;

    $error   = & ErrorHandler::get();
    $okay    = 0;
    $max_len = $_s_i_app_data_type[$adt_id]['MAX_LENGTH'];
  
    if ($max_len and (strlen($val) > $max_len))
        $error->set('validate', 12, 'Data-len ('.strlen($val).') in column &quot;'.$col.'&quot; in '.$table.' is longer than the allowed '.$max_len.' characters.');
}

function _check_minmax($table, $col, $val, $adt_id) {
#descr: helper function of validate_values to check minimum & maximum values
    global $_s_i_app_data_type;

    $error = & ErrorHandler::get();

    if (($val === '') or is_null($val)) return; // do nothing if empty
  
    $max = $_s_i_app_data_type[$adt_id]['MAX'];
    $min = $_s_i_app_data_type[$adt_id]['MIN'];

    if (is_numeric($max) and ($val > $max))
        $error->set('validate', 13, 'Value for column &quot;'.$col.'&quot; in '.$table.' is bigger than the maximum of '.$max.'.');

    if (is_numeric($min) and ($val < $min))
        $error->set('validate', 13, 'Value for column &quot;'.$col.'&quot; in '.$table.' is smaller than the minimum of '.$min.'.');
}

/**
 * validate values against data type
 * @param $val - the value
 * @param $typename - the type of value
 * @return array(
 * 	  'ok'  => 1 :o.k.
 * 			   <0 errors
 *    'info'=> info text
 */
static function check_type($val, $typename) {
    // $error = & ErrorHandler::get();
    $okay  = 0;
    $info=NULL;
   
  
    if (($val === '') or is_null($val)) { // empty strings are used for NULL-values
        $okay = 1;
        return array('ok'=>$okay);
    } 
    
    switch ($typename) {
        case 'FLOAT':  
        	if (is_numeric($val)) $okay = 1; 
        	break;
        case 'INT':    
        	if (is_numeric($val) and (intval($val)) == doubleval($val)) $okay = 1; 
        	break;
        case 'DATE':  // YYYY-MM-DD
        	if (self::_DATE_iso($val)) $okay = 1;
        	else $info='Format mismatch: YYYY-MM-DD';
        	break;
		case 'DATE2':  // "YYYY-MM-DD 12:35"
        	if (self::_DATE_iso($val,2)) $okay = 1;
        	else $info='Format mismatch: YYYY-MM-DD HH:MM';
        	break;
        case 'DATE3':  // "YYYY-MM-DD 12:35:23"
        	if (self::_DATE_iso($val,3)) $okay = 1;
        	else $info='Format mismatch: YYYY-MM-DD HH:MM:SS';
        	break;
		case 'STRING': 
			$okay = 1; 
			break;
		case 'JSON':
		    $answer = json_decode($val, TRUE);
		    if ($answer!==NULL)  $okay=1;
		    break;

    }
		
	return array('ok'=>$okay, 'info'=>$info);
}

}
