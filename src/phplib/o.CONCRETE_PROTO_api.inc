<?php
/**
 * CONCRETE_PROTO interface keys
 * @package o.CONCRETE_PROTO_api.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class oCONCRETE_PROTO_toolbox  {
    
    // keys for the toolbox DB_toolbox_keys
    private static $abs_key_translator = array(
        'NAME'                 => 'nam',
        'ABSTRACT_SUBST_ID'    => 'suaid',
        'QUANTITY'             => 'qua',
        'H_UNIT_ID'            => 'uniid',
        'ABS_DEV_ID'           => 'deaid',
        'NOTES'                => 'noa',
        'STEP_NR'              => 'st',
        'ABSTRACT_SUBST_NAME'  => 'sua',
        'ABS_DEV_NAME'         => 'dea',
        'H_UNIT_NAME'          => 'uni',
    );
    
    private static $conc_key_translator = array(
        'STEP_NR'            => 'st',
        'CONCRETE_SUBST_ID'  => 'suc',
        'CHIP_READER_ID'     => 'dev',
        'QUANTITY'           => 'quc',
        'NOTES'              => 'noc'
    );
    
    static function get_abs_DB2key() {
        return self::$abs_key_translator;
    }
    static function get_conc_DB2key() {
        return self::$conc_key_translator;
    }
}
