<?php
/**
 * info sub methods for CCT_TABLE
 * $Header: trunk/src/phplib/o.CCT_TABLE.info.inc 59 2018-11-21 09:04:09Z $
 * @package o.CCT_TABLE.info.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
 
class oCCT_TABLE_info {
    
    /**
     * get ALL tables
     * @param $sqlo
     * @param $type 'BO', 'BO_ASSOC'
     * @return array(TABLES)
     */
    static function getTables(&$sqlo) {
        $tables=NULL;
        $sqlsel = 'TABLE_NAME from CCT_TABLE order by TABLE_NAME';
        $sqlo->Quesel($sqlsel);
        while ( $sqlo->ReadRow() ) {
            $table = $sqlo->RowData[0];
            $tables[]=$table;
        }
        return ($tables);
    }
	
    /**
     * get tables by TYPE
     * @param $sqlo
     * @param $type 'BO', 'BO_ASSOC'
     * @return array(TABLES)
     */
    static function getTablesByType(&$sqlo, $type) {
    	$tables=NULL;
    	$sqlsel = "TABLE_NAME from CCT_TABLE where TABLE_TYPE=".$sqlo->addQuotes($type).' order by TABLE_NAME';
    	$sqlo->Quesel($sqlsel);
    	while ( $sqlo->ReadRow() ) {
    	    $table = $sqlo->RowData[0];
    	    $tables[]=$table;
    	}
    	return ($tables);
    }
    
    /**
     * get TABLE_NAME + NICE by TYPE, order by NICE, only ACTIVE tables
     * @param $sqlo
     * @param $type 'BO', 'BO_ASSOC'
     * @return array(TABLES=>NICE)
     */
    static function getTablesNiceByType(&$sqlo, $type) {
    	$tables=NULL;
    	$sqlsel = "TABLE_NAME, NICE_NAME from CCT_TABLE where TABLE_TYPE=".$sqlo->addQuotes($type).' and IS_ACTIVE>0 order by NICE_NAME';
    	$sqlo->Quesel($sqlsel);
    	while ( $sqlo->ReadRow() ) {
    	    $tables[$sqlo->RowData[0]]=$sqlo->RowData[1];
    	}
    	return ($tables);
    }
    
    /**
     * column of table exists ?, analyse CCT_COL_VIEW
     * make all strings UPPER !
     * @param $sqlo
     * @param $tablename
     * @param $colname
     * @return int 0,1
     */
    static function column_exists(&$sqlo, $tablename, $colname) {
    	$sqlsel = "1 from CCT_COL_VIEW where ".
    	    " upper(TABLE_NAME)= upper(".$sqlo->addQuotes($tablename). ") and upper(COLUMN_NAME)= upper(".
    		$sqlo->addQuotes($colname).")";
    	$sqlo->Quesel($sqlsel);
    	if ( $sqlo->ReadRow() ) return 1;
    	else return 0;
    }

}