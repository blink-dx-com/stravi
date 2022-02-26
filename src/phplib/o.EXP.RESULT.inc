<?php
/**
 * functions for handling of experiment results
 * - many help-functions for table H_EXP_RAW_DESC_COL
 * - handle tables:
 * 		- H_EXP_RAW_DESC_COL
 * 		- EXP_RAW_RESULT
 *		- SPOT_RESULT
 *		- EXP_CHAN_RES
 * 
 * @package o.EXP.RESULT.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @var res_type: VARSTRUCT: = values:
 *			  "raw"  : EXP_RAW_RESULT
 * 			  "normalized" : SPOT_RESULT
 *			  "chan" : EXP_CHAN_RES 
 * @version $Header: trunk/src/phplib/o.EXP.RESULT.inc 59 2018-11-21 09:04:09Z $
 */

require_once("db_access.inc");
require_once("globals.inc");

 
class gSpotResC {

function __construct() {
	$this->mapTab = array(
		"raw"        =>"EXP_RAW_RESULT",
		"normalized" =>"SPOT_RESULT",
		"chan"       =>"EXP_CHAN_RES"
		);
	$this->viewMap = array(
		"normalized"=> "EXP_SPOT_RESULT",
		"raw"       => "EXP_RAW_SPOT_RESULT"
		);
}

// MAP_COL to NAME mapping
function sqlsel_map2nice($h_raw_id, $table, $mapcol) {
	$sqlcmd = "NAME FROM H_EXP_RAW_DESC_COL WHERE H_EXP_RAW_DESC_ID = ".$h_raw_id. 
		" AND TABLE_NAME='".$table."' AND MAP_COL='".$mapcol."'";
	return ($sqlcmd);
}

/**
 * get all entries of (MAP_COL, NAME) for one H_EXP_RAW_DESC_ID and TABLE
 * @return $sqlcmd
 * @param object $h_raw_id
 * @param object $table
 */
function  sqlsel_allCols($h_raw_id, $table) {
	$sqlcmd = "MAP_COL, NAME FROM H_EXP_RAW_DESC_COL WHERE H_EXP_RAW_DESC_ID = ".
					$h_raw_id." and TABLE_NAME='".$table."' ORDER BY map_col";
	return ($sqlcmd);
}

function sqlsel_notes($h_desc_id, $table, $colname) {
	$sqlcmd = "NOTES FROM H_EXP_RAW_DESC_COL ".
			"WHERE h_exp_raw_desc_id=".$h_desc_id." and TABLE_NAME='".$table."' and map_col='".$colname."'";
	return ($sqlcmd);
}

function sqlsel_colByNotes($h_desc_id, $table, $notes) {
	$sqlcmd = "MAP_COL FROM H_EXP_RAW_DESC_COL ".
			"WHERE h_exp_raw_desc_id=".$h_desc_id." and TABLE_NAME='".$table."' and NOTES='".$notes."'";
	return ($sqlcmd);
}

// get number of columns for a table
function countCols(&$sql, $h_desc_id, $table) {
	$sql->query("SELECT COUNT(map_col) FROM H_EXP_RAW_DESC_COL ".
		" WHERE H_EXP_RAW_DESC_ID = ".$h_desc_id." and TABLE_NAME='".$table."' ");
    $sql->ReadRow();
    return ($sql->RowData[0]);
}

/**
 * get MAP_COL from NAME of TABLE
 * @return MAP_COL
 * @param object $sqlo
 * @param object $h_raw_id
 * @param object $table
 * @param object $nicecol
 */
function nice2map( &$sqlo, $h_raw_id, $table, $nicecol ) {
	$mapcol=NULL;
	$sqlo->query("SELECT MAP_COL FROM H_EXP_RAW_DESC_COL ".
		"WHERE H_EXP_RAW_DESC_ID = ".$h_raw_id. " AND TABLE_NAME=".
		$sqlo->addQuotes($table)." AND NAME='".$nicecol."'");
	if ($sqlo->ReadRow()) {
		if ($sqlo->RowData[0] !="") $mapcol = $sqlo->RowData[0];  
	}
	return ($mapcol);
}

// get map_col by name
function get_mapcol_bynice( &$sql, $h_raw_id, $nicecol ) {
 	$mapcol = $this->nice2map($sql, $h_raw_id, 'SPOT_RESULT', $nicecol);
	return ($mapcol);
}

/**
 * get NICE from MAP_COL,TABLENAME
 * @return MAP_COL
 * @param object $sqlo
 * @param object $h_raw_id
 * @param object $table : e.g SPOT_RESULT
 * @param object $col : e.g BIOVAL
 */
function map2nice( &$sqlo, $h_raw_id, $table, $mapcol ) {
	$nicecol=NULL;
	$sqlo->Quesel("NAME  FROM H_EXP_RAW_DESC_COL ".
			"WHERE H_EXP_RAW_DESC_ID = ".$h_raw_id. " AND TABLE_NAME=".
			$sqlo->addQuotes($table)." AND MAP_COL='".$mapcol."'");
	if ($sqlo->ReadRow()) {
		$nicecol = $sqlo->RowData[0];
	}
	return ($nicecol);
}

function get_norm_nice( &$sql, $exp_id, $column ) {
# descr: -get nice name of COLUMN (SPOT_RESULT)
#        -if H_EXP_RAW_DESCR is set, get extra nice name from there
#          QSV (col:VALUE2) 
    $nice_name = columnname_nice2("SPOT_RESULT", $column);
    
    $sql->query("SELECT H_EXP_RAW_DESC_ID FROM exp WHERE exp_id = ".$exp_id);
    $sql->ReadRow();
    $h_raw_id = $sql->RowData[0];   
    if ( $h_raw_id ) {
    	$sqlsel = $this->sqlsel_map2nice($h_raw_id, 'SPOT_RESULT', $column);    
        $sql->Quesel($sqlsel);
        if ($sql->ReadRow()) {
            if ($sql->RowData[0] !="") $nice_name = $sql->RowData[0]." (col:".$column.")";  
        }
    }
    return ($nice_name);
}

/**
 * - get nice name + RAW_colname of COLUMN (EXP_RAW_RESULT)
 - H_EXP_RAW_DESCR should be set
 * @param unknown $sql
 * @param unknown $exp_id
 * @param unknown $column
 * @return string
 */
function get_raw_nice( &$sql, $exp_id, $column ) {

    $nice_name = $column;
    
    $sql->query("SELECT H_EXP_RAW_DESC_ID FROM exp WHERE exp_id = ".$exp_id);
    $sql->ReadRow();
    $h_raw_id = $sql->RowData[0];   
    if ( $h_raw_id ) {    
        $sqlsel = $this->sqlsel_map2nice($h_raw_id, 'EXP_RAW_RESULT', $column);    
        $sql->Quesel($sqlsel);
		if ($sql->ReadRow()) {
            if ($sql->RowData[0] !="") $nice_name = $sql->RowData[0]." (col:".$column.")";  
        }
    }
    return ($nice_name);
}

/**
 * - get ONLY nice name of COLUMN (EXP_RAW_RESULT)
   - H_EXP_RAW_DESCR should be set
 * @param unknown $sql
 * @param unknown $exp_id
 * @param unknown $column
 * @return string
 */
function get_raw_nice_pure( &$sql, $exp_id, $column ) { 
	$nice_name = $column;

	$sql->query("SELECT H_EXP_RAW_DESC_ID FROM exp WHERE exp_id = ".$exp_id);
	$sql->ReadRow();
	$h_raw_id = $sql->RowData[0];
	if ( $h_raw_id ) {
		$sqlsel = $this->sqlsel_map2nice($h_raw_id, 'EXP_RAW_RESULT', $column);
		$sql->Quesel($sqlsel);
		if ($sql->ReadRow()) {
			if ($sql->RowData[0] !="") $nice_name = $sql->RowData[0];
		}
	}
	return ($nice_name);
}
				
function get_chan_nice( &$sql, $exp_id, $column ) {
# descr: - get nice name of COLUMN (EXP_CHAN_RES)
#        - H_EXP_RAW_DESCR should be set
    $nice_name = $column;
    
    $sql->query("SELECT H_EXP_RAW_DESC_ID FROM exp WHERE exp_id = ".$exp_id);
    $sql->ReadRow();
    $h_raw_id = $sql->RowData[0];   
    if ( $h_raw_id ) {    
		$sqlsel = $this->sqlsel_map2nice($h_raw_id, 'EXP_CHAN_RES', $column);    
        $sql->Quesel($sqlsel);
        if ($sql->ReadRow()) {
            if ($sql->RowData[0] !="") $nice_name = $sql->RowData[0]." (col:".$column.")";  
        }
    }
    return ($nice_name);
}


function set_table_link() {
# descr: sets the usage of table 'spot_result' resp. 'exp_raw_result' for an object
# sideeffect: writes text to screen
  
    

    $var   = "f.spot_result.table";
    $state = ($_SESSION['userGlob'][$var]!=NULL) ? $_SESSION['userGlob'][$var] : "normalized";
    $invert_state = ($state == "normalized") ? "raw" : "normalized";
    $but_img = "images/but.spot_result.${invert_state}.gif";
    echo "&nbsp;using $state data <a href='".$_SESSION['s_sessVars']["loginURL"]."/preferences.php?variable=$var&val=".urlencode($invert_state)."'>";
    echo "<b>==&gt;</b> switch to $invert_state data</a>\n";
}

function get_table(      $data_kind='') {
# descr: get which table ('spot_result' or 'exp_raw_result') is to be used for an object (translates contens of userPref)
# input: data_kind ... data_kind ('raw' or 'normalized') for which the table is to be returned instead of selected one
# return: selected table (according to preferences) or 'spot_result' if none selected or table for $data_kind if parameter not empty;

    
  
    $var = 'f.spot_result.table';

    if (empty($data_kind))
        $data_kind = $_SESSION['userGlob'][$var];
		
	if ($data_kind=="")  $data_kind="normalized";
	
	$table = $this->mapTab[$data_kind];
	
    return ($table);
}

function get_view() {
# descr: get which view ('exp_spot_result' or 'exp_raw_spot_result') is to be used for an object (translates contens of userPref)
# return: selected view (according to preferences) or 'exp_spot_result' if none selected;

    

    $var   = "f.spot_result.table";
    if (empty($data_kind))
        $data_kind = $_SESSION['userGlob'][$var];
	if ($data_kind=="")  $data_kind="normalized";	
    $table = $this->viewMap[$data_kind];
	return ($table);
}

function get_data_kind() {
# descr: get which data_kind ('raw' or 'normalized') is to be used for an object
# return: selected data_kind (from preferences) or 'normalized' if none selected;

    

    $var   = "f.spot_result.table";
    if ($_SESSION['userGlob'][$var]!=NULL)
        return $_SESSION['userGlob'][$var];
    else
        return "normalized";
}

function sql_channel_id(   $channel_id,       $table_name,       $data_kind='') {
# descr: coumputes if the channel_id is necessary in the sql-string
# input: channel_id ... identifier of the channel
#        table_name ... name of the table (alias name!) where channel_id belonges to. (WITHOUT the "."!)
#        data_kind ... data_kind ('raw' or 'normalized') for which the table is to be returned instead of selected one
# return: sql-code for where-clause or empty string

    if (empty($data_kind))
        $data_kind = $this->get_data_kind();

    return ($data_kind === 'normalized') ? '' : (
            empty($table_name) ? ' AND channel_id = '.$channel_id.' ' : ' AND '.
			$table_name.'.channel_id = '.$channel_id.' ');
}



function get_std_col(      $data_kind='') {
# descr: get the standard column which is to be shown if nothing selected
# input: data_kind ... data_kind ('raw' or 'normalized') for which the table is to be returned instead of selected one
# return: name of the standard column (of selected data_kind or $data_kind) as string.

    
    $var = 'f.spot_result.table';
	
    if (empty($data_kind))
        $data_kind = $_SESSION['userGlob'][$var];

    switch ($data_kind) {
		case 'raw': 
			$retval = 'V05';
			$vartmp = "f.spot_result.raw.colm";
			$tmpval =  $_SESSION['userGlob'][$vartmp];
			if ($tmpval!="") $retval=$tmpval;
			break;
		case 'chan': 
			$retval = 'V01';
			$vartmp = "f.spot_result.chan.colm";
			$tmpval =  $_SESSION['userGlob'][$vartmp];
			if ($tmpval!="") $retval=$tmpval;
			break;
		default:
			$retval = 'VALUE';
			$vartmp = "f.spot_result.norm.colm";
			$tmpval =  $_SESSION['userGlob'][$vartmp];
			if ($tmpval!="") $retval=$tmpval;   
			break;
    }
	
	return ($retval);
}

function get_multiStdCols( $data_kind ) {
# descr: get the MULTI standard columns which is to be shown if nothing selected
# input: data_kind ... 
# return:array of column-names

    
	$retval = NULL;
	
    switch ($data_kind) {
		
		case 'chan': 
			$retval = 'V01';
			$vartmp = "f.spot_result.chan.coar";
			$tmpval =  unserialize($_SESSION['userGlob'][$vartmp]);
			if ($tmpval!=NULL) $retval = $tmpval;
			break;
		default:
			$retval = NULL;
    }
	
	return ($retval);
}

function print_select_box(  &$sql,       $var_name,       $sel_row,    $exp_id, $res_type=NULL
  ) {
# descr: prints a combo-box to select the column those data should be uses
# input: sql ... instance of a subclass of CDBAbstract
#        var_name ... name of the combo-box --> variable_name after posting the form
#        sel_row ... row which is to be preselect
#        exp_id ... identifier of the experiment (necessary for using raw data)
#        res_type . "raw" or "normalized"
# sideeffect: html-code for combo-box
# errors:
#        spot_result 1 ... experiment does not exist
#        spot_result 2 ... no raw result type defined
#        db_access   2 ... error in query

    $error = & ErrorHandler::get();
    $table = $this->get_table($res_type);
    $cols  = $this->get_column_names($sql, $table, $exp_id);
    if ($error->got(READONLY)) return;

    echo "<select name='$var_name'>\n";

    foreach($cols as $col_name => $col_nice) {
        echo "<option value='$col_name'";
        if ($sel_row == $col_name)
            echo " selected";
        echo "> $col_nice </option>\n";
    }
    echo "</select>";
}

function get_column_names(  &$sql,       $table,    $exp_id) {
# descr: gets the nice_names of the columns of the selected table (for exp_raw_result it only gets the names of real columns (starting with 'v' or 's'))
#        - exp is given and has H_EXP_RAW_DESCR_ID than take nice_names from there ...
# input: sql ... instance of a subclass of CDBAbstract
#        table ... 'spot_result' or 'exp_raw_result' 'exp_chan_res'
#        exp_id ... identifier of the experiment (necessary for using raw data)
# return: array assinging column_name => nice_name
# error:
#        spot_result 1 ... experiment does not exist
#        spot_result 2 ... no raw result type defined
#        db_access   2 ... error in query

    global $_s_i_table;
	$FUNCNAME="get_column_names";
	
    $error = & ErrorHandler::get();
    
	$h_exp_raw_desc_id = NULL;
	
	if ($exp_id) {
		$sql->query("SELECT h_exp_raw_desc_id FROM exp WHERE exp_id = $exp_id");
		$sql->readRow();
		$h_exp_raw_desc_id = $sql->RowData[0];
    }
        
	return ($this->get_colNames_rawid( $sql, $table, $h_exp_raw_desc_id) );
}

function get_colNames_rawid( &$sql, $table, $h_exp_raw_desc_id ) {
	# descr: gets the nice_names of the columns from  H_EXP_RAW_DESCR_ID
	# return: columns[column_name => nice_name]

	global $_s_i_table, $error;
	$FUNCNAME = "get_colNames_rawid";
    $error    = & ErrorHandler::get();
	
	$ret   = array();
    $not_data_cols = array("EXP_ID", "SPOT_ID", "CHANNEL_ID");
	
	switch ($table) {
		case "SPOT_RESULT":
			
			if ($h_exp_raw_desc_id) {
				$sqlcmd = $this->sqlsel_allCols($h_exp_raw_desc_id, $table);
				$sql->Quesel($sqlcmd);
				while ($sql->readrow()) {
					if ( ($sql->RowData[0]!="") AND !in_array($sql->RowData[0], $not_data_cols)) {
						$ret[$sql->RowData[0]] = $sql->RowData[1];
					}
				}
			} else {
			    
				$sql->query("SELECT column_name FROM cct_column WHERE table_name = '$table'");
				if ($error->got(READONLY))
					return;
				while($sql->ReadRow()) {
					switch($sql->RowData[0]) {
					   case "EXP_ID": 
					       break;
					   case "SPOT_ID": 
					       break;
					   default:
						   $ret[$sql->RowData[0]] = columnname_nice2("SPOT_RESULT", $sql->RowData[0]);
					}
				}
			}
			break;
		case "EXP_RAW_RESULT":
			
			if (!$h_exp_raw_desc_id) {
				$error->set($FUNCNAME, 2, "Experiment (id=$exp_id) has no raw result type defined.", 1);
				return;
			}
	
			$sqlcmd = $this->sqlsel_allCols($h_exp_raw_desc_id, $table);
			$sql->Quesel($sqlcmd);
			if ($error->got(READONLY))
				return;
			while ($sql->readrow()) {
				if (array_key_exists($sql->RowData[0], $_s_i_table["EXP_RAW_RESULT"]) && !in_array($sql->RowData[0], $not_data_cols))
					$ret[$sql->RowData[0]] = $sql->RowData[1];
			}
			break;
		case "EXP_CHAN_RES":
					
			if (!$h_exp_raw_desc_id) {
				$error->set($FUNCNAME, 2, "Experiment (id=$exp_id) has no 'raw result' type defined.", 1);
				return;
			}
			$sqlcmd = $this->sqlsel_allCols($h_exp_raw_desc_id, $table);
			$sql->Quesel($sqlcmd);
			if ($error->got(READONLY))
				return;
			while ($sql->readrow()) {
				if (array_key_exists($sql->RowData[0], $_s_i_table["EXP_CHAN_RES"]) && !in_array($sql->RowData[0], $not_data_cols))
					$ret[$sql->RowData[0]] = $sql->RowData[1];
			}
			break;
		default:
			$error->set($FUNCNAME, 2, " table (='".$table."') unknown for this function.");	
    }
    return $ret;
}

function get_colNiceRaw( &$sql, $table, $h_exp_raw_desc_id ) {
	// return: array[NICE_name] = raw_name
	$colsRaw = $this->get_colNames_rawid( $sql, $table, $h_exp_raw_desc_id );
	if (sizeof($colsRaw)>0) {
		$colsNice=array_flip($colsRaw);
	} 
	return ($colsNice);
}


function count_spot_results (  &$sql,    $exp_id,       $column,    $channel_id,       $data_kind='') {
# descr: counts the results for the selected column
# input: sql        ... instance of a subclass of CDBAbstract
#        exp_id     ... identifier of the experiment
#        column     ... name of the column those values should be counted (db_name of the column!)
#        channel_id ... id of the channel for raw data
#        data_kind  ... use this datakind ('raw'|'normalized') instead of selected datakind
# return: number of values in specified column
# errors: 
# db_access 2 ... error in query

    $error    = & ErrorHandler::get();
    $tab      = $this->get_table($data_kind);
    $channel  = $this->sql_channel_id($channel_id, "s", $data_kind);
    $real_col = empty($data_kind) ? $column : $this->get_std_col($data_kind);

    $sql->query('SELECT COUNT('.$real_col.') FROM '.$tab.' s WHERE exp_id = '.$exp_id.$channel);
    if (!$error->got(READONLY))
        if ($sql->ReadRow())
            return $sql->RowData[0];
    return;
}


function get_spot_minmax (  &$sql,    $exp_id,       $column,    $channel_id) {
# descr: computes the minimum & maximum of the results in the selected column
# input: sql ... instance of a subclass of CDBAbstract
#        exp_id ... identifier of the experiment
#        column ... name of the column those values should be counted (db_name of the column!)
#        channel_id ... id of the channel for raw data
# return: array("min" => min_val, "max" => max_val)
# errors: 
#         db_access 2 ... error in query
#       spot_result 3 ... not supported raw result type to filter out invalid spots (warning level: 3)

    $error   = & ErrorHandler::get();
    $tab     = $this->get_table();
    $channel = $this->sql_channel_id($channel_id, '');
    $flags   = $this->sql_only_valid_flags ($sql, $exp_id);

    if ($error->got(CCT_ERROR_READONLY)) return;
    if ($error->got(CCT_WARNING_READONLY)) $flags =""; // set it from NULL to "";

    $sql->query("SELECT MIN($column), MAX($column) FROM $tab WHERE exp_id = ${exp_id}${channel}${flags}");
    if (!$error->got(CCT_ERROR_READONLY))
        if ($sql->ReadRow())
            return array("min" => $sql->RowData[0], "max" => $sql->RowData[1]);
    return;
}

function get_values (  &$sql,    $exp_id,       $column,    $channel_id,      $sort_col,    $arr_id) {
# descr: generates sql-string to select the values of one column & the substance_name from the database
# input: exp_id ... identifier of the experiment
#        column ... name of the column those values are needed
#        channel_id ... id of the channel for raw data
#        sort_col ... 'name', 'value' or 'name_and_spot_id'
#        arr_id ... identifier of the abstract array used in experiment
# return: sql-string
# errors:
#         db_access 2 ... error in query
#       spot_result 3 ... not supported raw result type to filter out invalid spots (warning level: 3)

    $error = & ErrorHandler::get();

    switch ($sort_col) {
    case 'name':             $orderBy = '1'; break;
    case 'name_and_spot_id': $orderBy = '1, 4'; break;
    default:                 $orderBy = "2 desc";
    }

    $tab = $this->get_table();
    $valid_flags = $this->sql_only_valid_flags($sql, $exp_id, "e", 1);
    if ($error->got(CCT_ERROR_READONLY)) return;
    if ($error->got(CCT_WARNING_READONLY)) $valid_flags = ""; // set it from NULL to ""
    $flags_and = " AND $valid_flags";
    $flags_not = " AND NOT $valid_flags";

    //   $valid_flags = false;

    if ($valid_flags)
        return "SELECT so.name, e.$column, 1 valid, so.spot_id ".
            "FROM $tab e, (SELECT a.spot_id, s.name FROM a_probe_on_array a, abstract_subst s WHERE a.abstract_array_id = $arr_id AND a.abstract_subst_id = s.abstract_subst_id AND s.name > ' ') so ".
            "WHERE e.exp_id  = $exp_id ".$this->sql_channel_id($channel_id, "e").
            " AND  e.spot_id = so.spot_id ".
            " AND  e.$column IS NOT NULL ".$flags_and.
            " UNION ".
            "SELECT so.name, e.$column, 0 valid, so.spot_id ".
            "FROM $tab e, (SELECT a.spot_id, s.name FROM a_probe_on_array a, abstract_subst s WHERE a.abstract_array_id = $arr_id AND a.abstract_subst_id = s.abstract_subst_id AND s.name > ' ') so ".
            "WHERE e.exp_id  = $exp_id ".$this->sql_channel_id($channel_id, "e").
            " AND  e.spot_id = so.spot_id ".
            " AND  e.$column IS NOT NULL ".$flags_not.
            " ORDER BY $orderBy";
    else
        return "SELECT so.name, e.$column, 1 valid, so.spot_id ".
            "FROM $tab e, (SELECT a.spot_id, s.name FROM a_probe_on_array a, abstract_subst s WHERE a.abstract_array_id = $arr_id AND a.abstract_subst_id = s.abstract_subst_id AND s.name > ' ') so ".
            "WHERE e.exp_id  = $exp_id ".$this->sql_channel_id($channel_id, "e").
            " AND  e.spot_id = so.spot_id ".
            " AND  e.$column IS NOT NULL ".
            " ORDER BY $orderBy";
}





function sql_only_valid_flags (  &$sql,    $exp_id,       $table_name="",     $wo_op=0) {
# descr: returnes an sql-string which will the invalid spots not get out of the database
#        till now only works for normalized data
#                            and raw data from iconoclust and aida
# input: exp_id     ... identifier of the experiment
#        table_name ... name of the table (alias name!) where flags-column belonges to. (WITHOUT the "."!)
#        wo_op      ... sql-string is returned without the boolean "AND" operator at the beginning
# return: void (+warning), if no flags-column or not supported
#         otherwise sql-string
# errors:
#         db_access 2 ... error in query
#       spot_result 3 ... not supported raw result type to filter out invalid spots (warning level: 3)

    $error   = & ErrorHandler::get();
    $bool_op = $wo_op ? " " : " AND ";
    if ($table_name != "") $table_name .= ".";
    
    if ($this->get_data_kind() == "normalized")
        return $bool_op.$table_name."flags != 1";
    else {
        $sql->query('SELECT h_exp_raw_desc_id FROM exp WHERE exp_id = '.$exp_id);
        if ($error->got(READONLY)) return;
        if (!$sql->ReadRow()) return '';
        $h_exp_raw_desc_id = $sql->RowData[0];
        if (empty($h_exp_raw_desc_id)) return '';
        
        // get name
        $sql->query('SELECT name FROM h_exp_raw_desc WHERE h_exp_raw_desc_id = '.$h_exp_raw_desc_id);
        if ($error->got(READONLY)) return;
        if (!$sql->ReadRow()) return '';
        $h_exp_raw_desc_name = $sql->RowData[0];
        $warning_text = 'Raw result type \''.$h_exp_raw_desc_name.'\' not yet supported to recognize invalid spots!';
        
        
        // notes have to be 'flags'
        $sql->query('SELECT map_col, notes FROM H_EXP_RAW_DESC_COL'.
                    ' WHERE h_exp_raw_desc_id = '.$h_exp_raw_desc_id.
					' AND TABLE_NAME='.$sql->addquotes('EXP_RAW_RESULT').
					' and notes = '.$sql->addquotes('flags'));
        if ($error->got(READONLY)) return;
        if (!$sql->ReadRow()) {
            $error->set('spot_result', 3, $warning_text, 3);
            return;
        }
        
        //   	$error->set('spot_result', 3, $warning_text, 3); //***
        
        switch (true) {
        case !strcasecmp(substr($h_exp_raw_desc_name, 0, strlen('iconoclust')), 'iconoclust'):
            return $bool_op.$table_name.$sql->RowData[0].' != 1';
        case !strcasecmp(substr($h_exp_raw_desc_name, 0, strlen('aida')), 'aida'):
            return $bool_op.'('.$table_name.$sql->RowData[0].' IS NULL OR '.$table_name.$sql->RowData[0].' = 0)';
        default:
            $error->set('spot_result', 3, $warning_text, 3);
            return;
        }
    }
}


function sql_results_and_valid_flags (  &$sql,    $exp_id,       $column,     $order=0,    $subst_arr=0) {
# descr: returnes an sql-string with which you get spot_id, $column, valid [0=no; 1=yes]
# input: exp_id   ... identifier of the experiment
#        column   ... name of the column in either spot_result or exp_raw_result
#        order    ... include the order by
#        subst_arr ... get only the results for the spots where the abstract_subst with this id is on
#			[subst_id1]=1, [subst_id2]=1, ...
# return: sql-string, if no flags-column or not supported valid is always 1
#         
# errors:
#         db_access 2 ... error in query
#       spot_result 3 ... not supported raw result type to filter out invalid spots (warning level: 3)

    $error = & ErrorHandler::get();

    if (sizeof($subst_arr)>0) {
        $subst_cond = ' AND ( abstract_subst_id = '.implode(' OR abstract_subst_id = ', array_keys($subst_arr)).')';
        $subst_exists=1;
    } else {
        $subst_exists=0;
        $subst_cond = '';
    }
    $table = ($subst_exists > 0) ? $this->get_view() : $this->get_table();
  
    $order_by = $order ? " ORDER BY spot_id" : "";
    $flags = $this->sql_only_valid_flags($sql, $exp_id, "", 1);
    if ($error->got(CCT_ERROR_READONLY)) return;
    if ($error->got(CCT_WARNING_READONLY)) $flags = ""; // set it from NULL to ""
    $flags_and = " AND $flags";
    $flags_not = " AND NOT $flags";

    if ($flags)
        return
            'SELECT '.$column.', spot_id, 1 valid '.
            'FROM '.$table.
            ' WHERE exp_id = '.$exp_id.$this->sql_channel_id(1,'').
            ' AND '.$column.' IS NOT NULL'.$flags_and.$subst_cond.
            ' UNION '.
            'SELECT '.$column.', spot_id, 0 valid '.
            'FROM '.$table.
            ' WHERE exp_id = '.$exp_id.$this->sql_channel_id(1,'').
            ' AND '.$column.' IS NOT NULL'.$flags_not.$subst_cond.$order_by;
    else
        return
            'SELECT '.$column.', spot_id, 1 valid '.
            'FROM '.$table.
            ' WHERE exp_id = '.$exp_id.$this->sql_channel_id(1,'').
            ' AND '.$column.' IS NOT NULL'.$subst_cond;
}

function get_channel_num (  &$sql, $exp_id ) {
# descr: get number of channels
	$sqls = "select max(channel_id) from EXP_RAW_RESULT where exp_id=".$exp_id;
	$sql->query($sqls);
	$sql->ReadRow();
	$retnum = $sql->RowData[0];
	return ($retnum);
}

}