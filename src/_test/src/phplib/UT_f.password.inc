<?php
/**
 * @package f.password.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class UT_f_password_inc extends gUnitTestSub {

    static function show_pw($pw) {
        return '|'.$pw.'| len:'.strlen($pw);
    }
    // return: 0 : not passed, 1: passed
    function dotest( &$sqlo, $options ) {
        $retval=1;
        
        $pw0='qwoP$xc2üö'; // 8 chars
        $pw_db = fPasswordC::hash_pw($pw0);
        $this->_infoNow( 'PW_crypt:','USER:'.$pw0. ' CRYPT:'.self::show_pw($pw_db));
        
        
        $out = fPasswordC::verify_pw($pw_db, $pw0);
        $this->_infoNow( 'Verify:','Result: '.$out.' USER:'.$pw0. ' CRYPT:'.self::show_pw($pw_db));
        $this->_compareTestResult('TEST01', 1, $out);
        
        $pw1_bad='qwoP$xc3';
        $out = fPasswordC::verify_pw($pw_db, $pw1_bad);
        $this->_infoNow( 'Verify BAD-PW:','Result: '.$out.' USER:'.$pw1_bad. ' CRYPT:'.self::show_pw($pw_db));
        $this->_compareTestResult('TEST02', 0, $out);
        
        $pw_old_db = fPasswordC::password_OLD2crypt($pw0);
        $out = fPasswordC::verify_pw($pw_old_db, $pw0);
        $this->_infoNow( 'Verify OLD-PW:','Result: '.$out.' USER:'.$pw0. ' CRYPT:'.self::show_pw($pw_old_db));
        $this->_compareTestResult('TEST03', 1, $out);
        
        $pw_old_db = fPasswordC::password_OLD2crypt($pw0);
        $out = fPasswordC::verify_pw($pw_old_db, $pw1_bad);
        $this->_infoNow( 'Verify OLD-PW:','Result: '.$out.' USER:'.$pw1_bad. ' CRYPT:'.self::show_pw($pw_old_db));
        $this->_compareTestResult('TEST04', 0, $out);
        
        return ($retval);
    }
    
}
