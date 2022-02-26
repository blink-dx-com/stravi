<?php
require_once 'o.S_OBJLINK.subs.inc';

/**
 * aliquot management
 * @package o.ABSTRACT_SUBST.aliquot.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class oABSTACT_SUBST_aliquot {
    
    private const OBJ_SLINK_KEY='Aliquot';
    
    function __construct($sua_id=NULL) {
        if ($sua_id) $this->set_sua($sua_id);
    }
    
    
    function set_sua($sua_id) {
        $this->sua_id=$sua_id;
    }
    
    /**
     * get Aliquot
     * @param object $sqlo
     */
    function get_aliquot_sua($sqlo) {
        
        if (!$this->sua_id) throw new Exception('Input SUA-ID missing');
        $children = oS_OBJLINK_subs::getChiObjByKey($sqlo, 'ABSTRACT_SUBST', $this->sua_id, self::OBJ_SLINK_KEY);
        if (empty($children)) return;
        
        $child = current($children);
        $child_id=$child[1];
        return $child_id; 
    }
    
    /**
     * get Mother of Aliquot
     * @param object $sqlo
     */
    function get_mother_sua($sqlo) {
        if (!$this->sua_id) throw new Exception('Input SUA-ID missing');
        $mothers = oS_OBJLINK_subs::getMoObjsbyTabKey($sqlo, 'ABSTRACT_SUBST', $this->sua_id,  'ABSTRACT_SUBST', self::OBJ_SLINK_KEY);
        if (empty($mothers)) return;
        $mother = current($mothers);
        return $mother;
    }
    
   
}