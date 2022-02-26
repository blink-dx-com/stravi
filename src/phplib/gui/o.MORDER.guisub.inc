<?php
/**
 * GUI definitions
 * @package o.MORDER.guisub.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class oMORDER_guisub {

    /**
     *  1=>'started',
            2=>'beauftragt',
            3=>'reviewed',
            4=>'prod_start',
            5=>'abgeschlossen',
     * @var array
     */
    static $f_ACCEPT_icons = array(
        'beauftragt'    => 'i13_morder_B.png',
    	'reviewed'      => 'i13_morder_R.png',
    	'prod_start'    => 'i13_morder_P.png',
    	'abgeschlossen' => 'i13_ok.gif',
    	
    );

     
    /**
     * get icon-name of status-flag
     */
    static function iconOfState($status_name) {
        return self::$f_ACCEPT_icons[$status_name];
    }
    
    static function get_states_info() {
        return self::$f_ACCEPT_icons;
    }
 
}
 
 