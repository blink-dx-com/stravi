<?php

class UT_o_SOCKET_subs_inc extends gUnitTestSub {


    function dotest( &$sqlo, $options ) {
    
        $SOCKET_ID = 995;

        $testLib = new oSOCKET_subs();
        $testLib->init($sqlo, $SOCKET_ID);
        $components = $testLib->get_cart_content_arr($sqlo);

        $this->_infoTable('01 components', $components);
    	
    	return 1;
    }

}
