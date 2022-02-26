<?php
/**
 * ORACLE functions 
 * @package db_funcs.inc
 */
class DB_funcs {
    
    static function get_trigger_status($sqlo, $triggername) {
        $sqlsel    = "STATUS from USER_TRIGGERS where TRIGGER_NAME=".$sqlo->addQuotes($triggername);
        $sqlo->Quesel($sqlsel);
        if ( $sqlo->ReadRow() ) {
            $statusval = $sqlo->RowData[0];
        } else {
            $statusval='unknown';
        }
        
        return $statusval;
    }
}