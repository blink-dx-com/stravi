<?php
/**
 * POSTGRES functions 
 * @package db_funcs.inc
 */
class DB_funcs {
    
    static function get_trigger_status($sqlo, $triggername) {
        $sqlsel    = "tgenabled from pg_trigger where tgname=".$sqlo->addQuotes(strtolower($triggername));
        $sqlo->Quesel($sqlsel);
        if ( $sqlo->ReadRow() ) {
            $status_raw = $sqlo->RowData[0];
            $statusval='DISABLED';
            if ($status_raw=='O') $statusval='ENABLED';
        } else {
            $statusval='unknown';
        }
        
        return $statusval;
    }
}