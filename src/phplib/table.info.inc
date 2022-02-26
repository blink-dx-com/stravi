<?php

/**
 * helper methods for a TABLE
 * @author skube
 * @package table.info.inc
 */
class tableInfoC {
    
    function __construct($table) {
        $this->table = $table;
    }
    
    function cnt_obj_by_feature(&$sqlo, $column, $value) {
        
        $sqlsel = 'count(1) from '.$this->table .' where '.$column.'='.$sqlo->addQuotes($value);
        $sqlo->Quesel($sqlsel);
        $sqlo->ReadRow();
        $cnt = $sqlo->RowData[0];
        return $cnt;
    }
    
    function first_obj_by_feature(&$sqlo, $column, $value) {
        
        $obj_id = 0;
        $sqlsel = 'count(1) from '.$this->table .' where '.$column.'='.$sqlo->addQuotes($value);
        $sqlo->Quesel($sqlsel);
        $sqlo->ReadRow();
        $cnt = $sqlo->RowData[0];
        
        if ($cnt) {
            $pk_name = PrimNameGet2($this->table);
            $sqlsel = $pk_name. ' from '.$this->table .' where '.$column.'='.$sqlo->addQuotes($value);
            $sqlo->Quesel($sqlsel);
            $sqlo->ReadRow();
            $obj_id = $sqlo->RowData[0];
            
        }
        
        return array('cnt'=>$cnt,'id'=>$obj_id);
    }
    
}