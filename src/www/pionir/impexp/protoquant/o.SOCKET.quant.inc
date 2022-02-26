<?php
require_once ('o.PROTO.quant.inc');

/**
 * [protoQuant] specials for experiment
   simple protocol compare (show only quantities)
 * @package o.SOCKET.quant.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class oSOCKET_Quant extends fQuant_obj_ABS  {
    
    function __construct() {
        $this->set_table('SOCKET');
    }

    function get_columns() {
        
        $cols = array(
            array(
                'name' => 'BCBATCH_ID',
                'col_nice'=> 'Related Cart-Batch',
                'nice' => 1,
                'ty'   => 'objtrc',        // object track
                'fk_col' => 'SOCKET_ID',   // on object tracking: look for this column in BCBATCH_H_SOC
                't'    => 'BCBATCH',       // the destination table
                'fkt'  => 'BCBATCH_H_SOC', // the table for the object tracking
                'notes'=> 'Used in Cart-Batch'
            ),
            
            array(
                'name'   => 'MISC_compo',
                'col_nice'=> 'One Component MISC',
                'nice'   => 1,
                'ty'     => 'ass',        
                'fk_col' => 'CONCRETE_SUBST_ID',   // on object tracking: look for this column in BCBATCH_H_SOC
                't'      => 'CONCRETE_SUBST',       // the destination table
                'fkt'    => 'CART_H_SUC',          // the table for the object tracking
                'pk2'    => 'KEY',
                'pk2_val'=> 'MISC',    // value of KEY
                'notes'  => 'Used MISC component'
            ),
            
            array(
                'name'   => 'Sensiflow_value in Batch',
                'col_nice'=> 'Sensiflow value of Cart',
                'ty'     => 'ass',
                'fk_col' => 'IVAL01',   // on 'ass' or 'objtrc': look for this column in BCBATCH_H_SOC
                'fkt'    => 'BCBATCH_H_SOC',          // the table for the object tracking
                'pk1'    => 'SOCKET_ID',  
                'pk2'    => 'none',       // no second primary key !
                'notes'  => 'Sensiflow value'
            ),
            
        );
        
        return $cols;
        
    }
}

