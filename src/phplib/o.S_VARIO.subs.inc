<?php

/**
 * S_VARIO read functions
 * @package o.S_VARIO.subs.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class oS_VARIO_sub {

    function __construct($tablename=NULL) {
    	$this->tablename = $tablename;
    }
    
    /**
     * set table for following analysis methods
     * @param string $tablename
     */
    function setTable( $tablename ) {
    	$this->tablename = $tablename;
    }
    
    /**
     * get VALUE by KEY of object
     * @param  $sql
     * @param  $id object id
     * @param  $key
     */
    function getValByKey( &$sql, $id, $key ) {
    	$tablename = $this->tablename;
    	$sql->query("SELECT VALUE FROM S_VARIO WHERE TABLE_NAME='".$tablename."' AND OBJ_ID=".$id.
    		 " AND KEY=". $sql->addQuotes($key) );
    	$sql->ReadRow();
    	$value = $sql->RowData[0];
    	return ($value);
    }
    
    /**
     * get code-name of defined keys
     * @param object $sql
     */
    function getDefKeys( &$sql ) {
    	$tablename = $this->tablename;
    	$keys = NULL;
    	$sql->query("SELECT KEY FROM S_VARIO_DESC WHERE TABLE_NAME='".$tablename."' order by KEY");
    	while ( $sql->ReadRow() ) {
    		$keys[] = $sql->RowData[0];
    	}
    	return ($keys);
    }
    
    /**
     * get array of all (KEY,VALUE) for one object
     * @param  $sql
     * @param  $id
     * @return array $varioarr [KEY]=VALUE
     */
    function getAllVals( &$sql, $id ) {
    
    	$tablename = $this->tablename;
    	$sql->query("SELECT KEY, VALUE FROM S_VARIO WHERE TABLE_NAME='".$tablename."' AND OBJ_ID=".$id.
    		 " order by KEY" );
    	$varioarr = array();
    	while ( $sql->ReadRow() ) {
    		$varioarr[$sql->RowData[0]] = $sql->RowData[1];
    	}
    	return ($varioarr);
    }
    
    /**
     * get the SQL-select statement to select all objects with this key of TABLE
     * @param  $sqlo
     * @param  $key VARIO-KEY
     * @param  $condition e.g. "VALUE is not NULL"
     * @return string
     */
    function getSqlSelOfKey( &$sqlo, $key, $valCondition=NULL) {
    	// $FUNCNAME= __CLASS__.':'.__FUNCTION__;
    	$tablename = $this->tablename;
    	if ($tablename==NULL) sys_error_my('class '.__CLASS__.': not initialized'  );

    	if ($valCondition!=NULL) {
    	    $valCondition = ' and '.$valCondition;
    	}
    	
    	$sqlSel = "SELECT OBJ_ID FROM S_VARIO WHERE TABLE_NAME='".$tablename."' AND ".
    		" KEY=". $sqlo->addQuotes($key) . $valCondition;
    	return $sqlSel;
    }
    
    /**
     * get all objects with key,val
     * @param object $sqlo
     * @param string $key
     * @param string $val
     */
    function getObjectsOfKeyVal(&$sqlo, $key, $val) {
        $obj_ids=array();
        $sql_full = $this->getSqlSelOfKey($sqlo, $key, 'VALUE='.$sqlo->addQuotes($val));
        $sqlo->Query($sql_full);
        while ( $sqlo->ReadRow() ) {
            $obj_ids[] = $sqlo->RowData[0];
        }
        return $obj_ids;
    }
    
    /**
     * get defined NICE-name of key
     * @param  $sqlo
     * @param string $key
     */
    function getDefKeyNice( &$sqlo, $key ) {
    	
    	$tablename = $this->tablename;
    	$sqlo->Quesel("NICE FROM S_VARIO_DESC ".
    			" WHERE TABLE_NAME='".$tablename."' and KEY='".$key."'");
    	$sqlo->ReadRow();
    	$nice = $sqlo->RowData[0];
    	
    	return ($nice);
    }
    
    /**
     * get full column features of S_VARIO_DESC for one KEY
     * @param $sqlo
     * @param $key
     * @return array $retarr [COLNAME]=VALUE
     */
    function getColFetaures( &$sqlo, $key ) {
    	
    	$tablename = $this->tablename;
    	$sqlo->Quesel("* FROM S_VARIO_DESC ".
    			" WHERE TABLE_NAME='".$tablename."' and KEY='".$key."'");
    	$sqlo->ReadArray();
    	$retarr = $sqlo->RowData;
    	if ($retarr['NICE']==NULL) {
    		$retarr['NICE']=$key;
    	}
    	return ($retarr);
    }
    
    /**
     * get all (KEY,NICE) entries
     * @param  $sql
     */
    function getAllKeysNice( &$sql ) {
    	$tablename = $this->tablename;
    	$keys = array();
    	$sql->query("SELECT KEY, NICE FROM S_VARIO_DESC WHERE TABLE_NAME='".$tablename."' order by NICE");
    	while ( $sql->ReadRow() ) {
    		$keys[$sql->RowData[0]] = $sql->RowData[1];
    	}
    	return ($keys);
    }

    /**
     * get VALUE by KEY of object
     * @param  $sql
     * @param  $id object id
     * @param  $key
     */
    static function getValByTabKey( &$sql, $tablename, $id, $key ) {
        if (!is_numeric($id)) {
            throw new Exception('ID must be numeric. ID "'.$id.'" was given.');
        }
        $sql->query("SELECT VALUE FROM S_VARIO WHERE TABLE_NAME='".$tablename."' AND OBJ_ID=".$id.
            " AND KEY=". $sql->addQuotes($key) );
        $sql->ReadRow();
        $value = $sql->RowData[0];
        return ($value);
    }
}
