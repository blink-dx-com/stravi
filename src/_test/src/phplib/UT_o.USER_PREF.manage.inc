<?php
// test for /group/it/cvs/partisan/phplib/o.USER_GROUP.subs.inc

class UT_o_USER_PREF_manage_inc extends gUnitTestSub {


// return: 0 : not passed, 1: passed
function dotest( &$sqlo, $options ) {

    
    
    $key='UT_tesk_key';
    $value='';
    oUSER_PREF_manage::entry_update($sqlo, $key, $value);
    $val_current = glob_elementDataGet( $sqlo, 'USER_PREF', 'VAR_NAME', $key, 'VALUE');
    $this->_compareTestResult('TEST01', NULL, $val_current);
    
    
    $key='UT_tesk_key';
    $value='newval';
    oUSER_PREF_manage::entry_update($sqlo, $key, $value);
    $val_current = glob_elementDataGet( $sqlo, 'USER_PREF', 'VAR_NAME', $key, 'VALUE');
    $this->_compareTestResult('TEST02', $value, $val_current);
    
    $key='UT_tesk_key';
    $value='halloUpdate';
    oUSER_PREF_manage::entry_update($sqlo, $key, $value);
    $val_current = glob_elementDataGet( $sqlo, 'USER_PREF', 'VAR_NAME', $key, 'VALUE');
    $this->_compareTestResult('TEST03', $value, $val_current);
	
	
	return 1;
}

}
