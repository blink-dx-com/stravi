<?php
/**
 * [protoQuant] MINIMUM Definitions
 * @package o.PROTO.def_min.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class fQuant_defs {
    
    const PROFILE_DIR_NAME='Protoquant';
    
    /**
     * get protoquant profile PROJ
     * @param object $sqlo
     * @param string $create_flag
     * @return number
     */
    static function get_profile_PROJ($sqlo, $create_flag='READONLY') {
        $proj_lib = new profile_funcs();
        $profile_proj_id  = $proj_lib->getProj($sqlo,  self::PROFILE_DIR_NAME, $create_flag );
        return $profile_proj_id;
    }
}