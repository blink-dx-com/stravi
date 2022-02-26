<?php
require_once 'o.ABSTRACT_PROTO.formula.inc';
require_once 'o.PROTO.upsteps.inc';

/**
 * CONCRETE_PROTO : calc values by FORMULA
 * @package o.CONCRETE_PROTO.formula.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @var 
 */
class oCONCRETE_PROTO_formula_mod {
    
    private $_error_lines=array(); // array by line_id
    private $data_out; // data out
    
    function __construct($sqlo, $prc_id) {
        $this->objid=$prc_id;
        $this->pra_id  = glob_elementDataGet( $sqlo, 'CONCRETE_PROTO', 'CONCRETE_PROTO_ID', $this->objid, 'ABSTRACT_PROTO_ID');
        $this->formula_lib     = new oABSTRACT_PROTO_formula($sqlo, $this->pra_id, 1 );
    }
    
    /**
     *
     * @param array $data_in
     */
    public function calculate_all(&$data_in){
        $this->data_out = $this->formula_lib->calculate_all($data_in);
    }
    
    function get_data_out() {
        return $this->data_out;
    }
    
    /**
     * save in database
     * @param object $sqlo
     */
    function save($sqlo) {
        
        $step_params=array();
        foreach($this->data_out as $step_nr => $quantity) {
            $row=array('QUANTITY'=>$quantity);
            $step_params[$step_nr]=$row;
        }
        
        $proto_mod_lib = new oConcProtoUpSteps();
        $proto_mod_lib->setProto($this->objid);
        $proto_mod_lib->protoUpdate($sqlo, $this->pra_id, $step_params);
        
    }
}
