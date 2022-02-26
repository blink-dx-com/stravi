<?php

class UT_o_CONCRETE_SUBST_proto_inc extends gUnitTestSub {


// return: 0 : not passed, 1: passed
function dotest( &$sqlo, $options ) {
    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
    
    $ABSTRACT_SUBST_ID = $this->_getExistObjID( "ABSTRACT_SUBST", "qcsteps" );
    
    $pra_id = $this->_getExistObjID( "ABSTRACT_PROTO", "QC" );
    
    $tablename='CONCRETE_SUBST';
    $args=array(
        'vals'=>array(
            "ABSTRACT_SUBST_ID"=>$ABSTRACT_SUBST_ID,
            'NAME'=>'UT:'.$FUNCNAME
        )
    );
    $suc_id = $this->_createObj($tablename, $args);
    
    $this->_infoNow( 'New SUC:',  $suc_id);
    
    $features = array(
        'vals'=>array('NAME'=>'UT:'.$FUNCNAME)
    );
    
    $x = new oCONCRETE_SUBST_proto($suc_id);
    $x->create_proto($sqlo, $features, $pra_id);
    
    // $prcs = $x->get_c_protos($sqlo);
    $prc_and_pra = $x->get_ca_protos($sqlo);
    $this->_infoNow( 'SUC:'.$suc_id.' PRC+PRA logs:',  print_r($prc_and_pra,1));
    
    
	
	
	return 1;
}

}
 