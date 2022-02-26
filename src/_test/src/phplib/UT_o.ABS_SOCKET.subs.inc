<?php

class UT_o_ABS_SOCKET_subs_inc extends gUnitTestSub {


    function dotest( &$sqlo, $options ) {
    
        $ABS_SOCKET_ID = 27;

        $testLib = new oABS_SOCKET_subs();
        $testLib->init($sqlo, $ABS_SOCKET_ID);
        $components = $testLib->get_cart_content_arr($sqlo);

        $this->_infoTable('01 components', $components);
    	
    	return 1;
    }

}
