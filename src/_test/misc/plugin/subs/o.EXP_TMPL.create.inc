<?php
require_once( $_SESSION['s_sessVars']['loginPATH'].'/api/lib/metacall.inc' );
require_once 'o.EXP_TMPL.proto.inc';
/**
 * @package o.EXP_TMPL.create.inc
 * @author skube
 *
 */
class oEXP_TMPL_create_UT extends create_UT_lib {
    
    
    private function cr_toolbox(&$sqlo) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
          
        if ( $_SESSION['s_product']['type']!="blinkLIMS") {
            return;
        }
   
        $args = array(
            'vals'=> array(
                'NOTES'=> 'UnitTest '.$FUNCNAME,
            )
        );
        $obj_id = $this->_create_UT_obj_RAW($sqlo, $args, $this->synonym, 'EXP_TMPL');
        if (!$obj_id) {
            $error->set( $FUNCNAME, 1, 'EXP_TMPL not created.' );
            return;
        }
       
        
        // add protos
        $one_obj_lib = new create_UT_oneObj($sqlo);
        
        $one_obj_lib->set('ABSTRACT_PROTO', 'Assaying' );
        $pra_id_1 = $one_obj_lib->ana_crea($sqlo);
        
        //$one_obj_lib->set('ABSTRACT_PROTO', 'Experiment protocol' );
        //$pra_id_2 = $one_obj_lib->ana_crea($sqlo);
        $use_PRA_ID = 45; // use this predefined PRA
        if (!gObject_exists ($sqlo, 'ABSTRACT_PROTO', $use_PRA_ID) ) {
            $error->set( $FUNCNAME, 2, 'Expected ABSTRACT_PROTO:'.$use_PRA_ID.' not found.' );
            return;
        }
        $pra_id_2 = $use_PRA_ID;
        
        $exp_tmpl_lib  = new oEXP_TMPL_protoC($obj_id);
        

        $exp_tmpl_lib->add_proto($sqlo, $pra_id_1, 1, 10);
        $exp_tmpl_lib->add_proto($sqlo, $pra_id_2, 2, 20);
        
        return $obj_id;
    }
    
    private function cr_toolbox_customer(&$sqlo) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        if ( $_SESSION['s_product']['type']!="blinkLIMS") {
            return;
        }
        
        $args = array(
            'vals'=> array(
                'NOTES'=> 'UnitTest '.$FUNCNAME,
            )
        );
        $obj_id = $this->_create_UT_obj_RAW($sqlo, $args, $this->synonym, 'EXP_TMPL');
        if (!$obj_id) {
            $error->set( $FUNCNAME, 1, 'EXP_TMPL not created.' );
            return;
        }
        
        
        // add protos
        $one_obj_lib = new create_UT_oneObj($sqlo);
        
        $one_obj_lib->set('ABSTRACT_PROTO', 'Assaying' );
        $pra_id_1 = $one_obj_lib->ana_crea($sqlo);
        
        $one_obj_lib->set('ABSTRACT_PROTO', 'Exp_with_samples' );
        $pra_id_2 = $one_obj_lib->ana_crea($sqlo);
        
        $exp_tmpl_lib  = new oEXP_TMPL_protoC($obj_id);
        
        
        
        $exp_tmpl_lib->add_proto($sqlo, $pra_id_1, 1, 10);
        $exp_tmpl_lib->add_proto($sqlo, $pra_id_2, 2, 20);
        
        return $obj_id;
    }
    
    function create(&$sqlo) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        switch ($this->synonym) {
            case 'default':
                $args = array(
                    'vals'=> array(
                    'NOTES'=> 'UnitTest '.$FUNCNAME,
                    )
                );
                $obj_id = $this->_create_UT_obj_RAW($sqlo, $args, $this->synonym, 'EXP_TMPL');
                if (!$obj_id) {
                    $error->set( $FUNCNAME, 1, 'EXP_TMPL not created.' );
                    return;
                }
                break;
                
            case 'toolbox':
                $this->infoout('Tool!');
                $obj_id = $this->cr_toolbox($sqlo);
                break;
            case 'toolbox_customer':
                $this->infoout('Tool!');
                $obj_id = $this->cr_toolbox_customer($sqlo);
                break;
        }
        return $obj_id;
    }
}