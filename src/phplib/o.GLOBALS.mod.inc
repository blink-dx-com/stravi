<?php
require_once ('f.update.inc');
require_once 'insert.inc';
/**
 * manage GLOBALS table
 * @package o.GLOBALS.mod.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class oGLOBALS_mod {
    
    static function getval(&$sqlo, $key) {
        return glob_elementDataGet( $sqlo, 'GLOBALS', 'NAME', $key, 'VALUE'); 
    }
    
    /**
     * UPDATE or INSERT KEY/VAL
     * NO access check here ...
     * @param object $sqlo
     * @param string $key
     * @param string $val
     * @param string $notes
     */
    static function update(&$sqlo, $key, $val, $notes=NULL) {
        
        $idarr=array(
            'NAME'=>$key,
        );
        $argu=array(
            'VALUE'=>$val
        );
        
        $sqlo->Quesel('VALUE from GLOBALS where NAME='.$sqlo->addQuotes($key));
        if ($sqlo->ReadRow() ) {
            gObjUpdate::update_row_s($sqlo, "GLOBALS", $argu, $idarr);
        } else {
            $argu_ins=array(
                'NAME'=>$key,
                'VALUE'=>$val
            );
            insert_row_s($sqlo, "GLOBALS", $argu_ins);
        }

    }
}
