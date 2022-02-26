<?php
/**
 * get prefered values of table columns
 * $Header: trunk/src/phplib/f.prefvals.inc 59 2018-11-21 09:04:09Z $
 * @package f.prefvals.inc
 * @author  Steffen Kube
 */
class fPrefValsGet {
	
	/**
	 * - select prepared values from H_VAL_INIT 
	 * - NOTES field contains the nice-name of VALUE
	 * @param object $sql
	 * @param string $tablename
	 * @param string $colname
	 * @return array[] = array(VAL, NICE_VAL)
	 */
	static function getValInitList( &$sql, $tablename, $colname ) {
	  $values  = array();
	  $cnt     = 0;
	  $MAXSHOW = 200; // max elements 
	  
	  $sql->Quesel('value, notes FROM h_val_init WHERE table_name = '.$sql->addQuotes($tablename).
				  ' AND column_name = '.$sql->addQuotes($colname).' ORDER BY value');
	  while ( $sql->ReadRow()  ) {
	  	
	  	if  ($cnt > $MAXSHOW) {
	  		$values[$cnt] = array('...', ' ... more ...');
	  		break;
	  	}
	  	if ( $sql->RowData[1]==NULL ) $niceval =  $sql->RowData[0];
		else $niceval =  $sql->RowData[0].' -- '.$sql->RowData[1];
		$values[$cnt] = array($sql->RowData[0], $niceval);
		$cnt++;
	  }
	  return $values;
	}

	/**
	 * get all VALUES of H_VAL_INIT for one COLUMN, sorted by VAL
	 * @param object $sql
	 * @param string $tablename
	 * @param string $colname
	 * @return array[VAL] = NICE_VAL
	 *   - if no values: return array()
	 */
	static function getListSort( &$sqlo, $tablename, $colname ) {
		
	  $values  = array();
	  $cnt     = 0;
	  
	  $sqlo->Quesel('value, notes FROM H_VAL_INIT WHERE table_name = '.$sqlo->addQuotes($tablename).
				  ' AND column_name = '.$sqlo->addQuotes($colname).' ORDER BY value');
	  while ( $sqlo->ReadRow()  ) {	
	  	$val   = $sqlo->RowData[0];
		$notes = $sqlo->RowData[1];
		$values[$val] = $notes;
		$cnt++;
	  }
	  return $values;
	}
	
	/**
	 * get NOTES of DATA_VALUE for a specific column
	 * @param $sqlo
	 * @param $tablename
	 * @param $colName
	 * @param $data_val
	 */
	static function getNotesOfVal( &$sqlo, $tablename, $colName, $data_val ) {
		$notesVal =  NULL;
	
		$sqlo->Quesel('NOTES FROM H_VAL_INIT WHERE TABLE_NAME = '.$sqlo->addQuotes($tablename).
					  ' AND COLUMN_NAME = '.$sqlo->addQuotes($colName).' and VALUE='. $sqlo->addQuotes($data_val) .
					  ' and NOTES is not NULL');
 		if ($sqlo->ReadRow() ) {
  			$notesVal =  $sqlo->RowData[0];
 		}
		return $notesVal;
	}
}
