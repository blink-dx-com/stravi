<?php
require_once ("f.assocUpdate.inc");

/**
 * EXP_H_IMG MODIFY methods
 * @package o.EXP_H_IMG.mod.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class oEXP_H_IMG_mod {
    
    private $exp_id;
    
    function __construct($sqlo, $exp_id=0) {
        
        $this->assoclib = new  fAssocUpdate();
        if ($exp_id) 
            $this->set_exp($sqlo, $exp_id);
    }
    
    function set_exp($sqlo, int $exp_id) {
        if (!$exp_id) {
            throw new Exception('Input exp_id missing.');
        }
        $this->exp_id=$exp_id;
        $this->assoclib->setObj( $sqlo, 'EXP_H_IMG', $exp_id );
    }
    
    /**
     * add image
     * @param object $sqlo
     * @param array $argu must contain 'POS'
     * @throws Exception
     */
    function add_img($sqlo, array $argu) {
        if (!$this->exp_id) {
            throw new Exception('Class not initialized.');
        }
        if (!$argu['POS']) throw new Exception('Input "POS" missing');
       
        $this->assoclib->insert($sqlo, $argu);
    }
}