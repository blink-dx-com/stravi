<?php
/**
 * easy SQL queries
 * $Header: trunk/src/phplib/f.sqleasy.inc 59 2018-11-21 09:04:09Z $
 * @package fSqleasy
 * @author  qbi
 */
 
class fSqlEasyC {

// FUNCTION: get data of ONE column from a ONE-PK-table
function get_ColByID( &$sqlo, $tablename, $id, $outputCol) {			
	$key    = PrimNameGet2($tablename);
	$sqlsel = $outputCol." from ".$tablename." where ".$key."='".$id."'";
	$sqlo->Quesel($sqlsel);
	$sqlo->ReadRow();
	return ( $sqlo->RowData[0] );
}

}