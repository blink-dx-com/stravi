<?php

/**
 * simple warning class
 * @package f.warn.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class _warn_cls {
    
    private static $warning_arr=array();
    
    public static function reset() {
        self::$warning_arr=array();
    }
    
    public static function set_warn($text) {
        self::$warning_arr[] = $text;
    }
    
    public static function get_warnings() {
        return self::$warning_arr;
    }
}

 