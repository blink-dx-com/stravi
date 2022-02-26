<?php
require_once ("glob.obj.update.inc");

/**
 * DEVSIC help methods
 * TABLE: -- columns --
 *   INFOARR: JSON-string of NULL
 *     't'   => VAR-NAME
 *     'x'   => Title of VAR
 *     'type'=> type: 'checkbox', 'text'
 *     'n'=> NOTES
 * 
 * @package o.DEVSIC.subs.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class oDEVSIC_subs {
    
    const TYPE_Q= 'Q';
    const TYPE_V= 'V';
    const TYPE_W= 'W';
    
    const INFOARR_title_MAX_SIZE=100;
    const INFOARR_var_MAX_SIZE  =20;
    const INFOARR_notes_MAX_SIZE=70;
    
    /**
     * get DEVSIC_info_STRUCT
     * @param object $sqlo
     * @param int $devsic_id
     * @return array
     */
    static function get_info_struct($sqlo, $devsic_id) {
        $infoarr = array();
        if (!$devsic_id) return $infoarr;
        
        $infoarr_str = glob_elementDataGet( $sqlo, 'DEVSIC', 'DEVSIC_ID', $devsic_id, 'INFOARR');
        $infoarr     = json_decode($infoarr_str, TRUE);
        if (empty($infoarr)) $infoarr = array();
        return $infoarr;
    }
    
    /**
     * get DEVSIC_info_STRUCT
     * @param object $sqlo
     * @param int $devsic_id
     * @return void|mixed
     */
    static function features($sqlo, $devsic_id) {
        if (!$devsic_id) return;
        $features   = glob_elemDataGet3( $sqlo, 'DEVSIC', array('DEVSIC_ID'=> $devsic_id), array('*') );
        return $features;
    }
}


/**
 * class for one Device
 */
class oDEVSIC_obj_sub {

    function __construct($id) {
        $this->id = $id;
    }
    function get_info_struct($sqlo) {
        return oDEVSIC_subs::get_info_struct($sqlo, $this->id);
    }   
    function features($sqlo) {
        return oDEVSIC_subs::features($sqlo, $this->id);
    }
}

/**
 * modify an entry
 *
 */
class oDEVSIC_mod {
    
    function __construct($sqlo, $id) {
        $this->id = $id;
    }
    
    function update_INFOARR(&$sqlo, $infoarr) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $info_json=NULL;
        if (!empty($infoarr)) $info_json = json_encode($infoarr);
        
        if ( strlen($info_json)>=4000 ) {
            $error->set( $FUNCNAME, 1, 'too many characters. Max 4000 chars can be saved.' );
            return;
        }
 
        $UpdateLib = new globObjUpdate();
        $args = array( 'vals'=>array('INFOARR'=>$info_json) );
        $UpdateLib->update_meta( $sqlo, 'DEVSIC', $this->id, $args );
    }

}