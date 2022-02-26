<?php
/**
 * 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com) 
 * @package glob.obj.create.ABS.inc
 */
class gObjCreateC_ABS {
    
    var $infox;
    protected function _add_log($text) {
        $this->creaInfo[]=$text;
    }
    
    function get_log() {
        return ($this->creaInfo);
    }
    
    /**
     * before CREATION of object check/prepare arguments
     * manipulates this array, format see also object.info.inc
     format: array ( "vals" => data, "xobj" => extra_obj_data, "access" => access_data)
     */
    function crea_prep(&$sqlo, &$obj_param ) {}
    
    function post_crea( &$sql,  &$sql2, $objid,  $blueObjId, $option  = array()	) {}
    
    /**
     * on copy of object check/prepare arguments
     * @param $sqlo
     * @param $metaargu (contains changed values)
     * @throws error on problem
     * @global $this->infox[]
     */
    function copy_prep( &$sqlo, &$obj_blueparam ) {}
    
}
