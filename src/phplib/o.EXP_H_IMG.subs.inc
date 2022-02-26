<?php

/**
 * EXP_H_IMG sub-methods
 * @package o.EXP_H_IMG.subs.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class oEXP_H_IMG_subs {
    
    private $exp_id;
    
    function __construct($exp_id=0) {
        if ($exp_id) $this->exp_id=$exp_id;
    }
    
    function get_img_data($sqlo) {
        if (!$this->exp_id) {
            throw new Exception('Class not initialized.');
        }
        $img_arr=array();
        $sqlsel = "POS, IMG_ID, URL from EXP_H_IMG where EXP_ID=". $this->exp_id.' order by POS';
        $sqlo->Quesel($sqlsel);
        while ( $sqlo->ReadArray() ) {
            $img_arr[] = $sqlo->RowData;
        }
        return $img_arr;
    }
}