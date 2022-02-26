<?php
/**
 * ABSTRACT CLASS, included by module o.ACCEPT_PROT.subs.inc
 * @package o.ACCEPT_PROT.calc_ABS.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class o_ACCEPT_PROT_calc_ABS  {
    
    protected $cpid;
    protected $acc_sub_lib;
    protected $relevantSteps;
    
    function set_cpid($sqlo, $cpid, oACCEPT_PROT_subs &$acc_sub_lib) {
        $this->cpid  = $cpid;
        $this->acc_sub_lib = &$acc_sub_lib;
        $this->relevantSteps = array();
        
        $this->init($sqlo);
    }
    function get_relevant_steps() {
        return $this->relevantSteps;
    }
    
    public function init($sqlo) {}
    
    /**
     *  OVERWRITE !
     *  @return array $limit_problems ('err'=>0,1, 'info'=>text)
     *  or NULL
     */
    function calc($sqlo) {}
}
