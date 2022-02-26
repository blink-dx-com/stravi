<?php

class UT_o_ABSTRACT_PROTO_formula_inc extends gUnitTestSub {
	

// return: 0 : not passed, 1: passed
function dotest( &$sqlo, $options ) {

    $Cal = new X_Field_calculate();
    
    $test_arr=array(
        array('in'=>'5+7', 'expect'=>12),
        array('in'=>'(5+9)*5', 'expect'=>70),
        array('in'=>'(10.2+0.5*(2-0.4))*2+(2.1*4)', 'expect'=>30.4),
    );
    
    $ind=0;
    foreach($test_arr as $row) {
        $calc_res = $Cal->calculate($row['in']); 
        $this->_infoNow( 'TEST:', print_r($row,1). ' output:'.$calc_res );
        $this->_compareTestResult($ind, $row['expect'], $calc_res);
        
    }

	
	
	
	return 1;
}

}
