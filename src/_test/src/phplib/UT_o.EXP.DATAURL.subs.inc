<?php
// example for testing: 

class UT_o_EXP_DATAURL_subs_inc extends gUnitTestSub {
    
    
    
    // return: 0 : not passed, 1: passed
    function dotest( &$sqlo, $options ) {
        
        //$winurl='Z:\Data\2020\blink.one\06\05\2020-06-05 18_52_BOP2-0003_T2_3C_Bead_V_Pipette_Ventilated';
        $winurl='Z:\Data\2020\blink.one\06\19\2020-06-19 09_00_AW-1_steffen 01';
        
        $testLib = new oEXP_DATAURL_subs($winurl);
        
        
        $this->_infoNow( 'Path', $winurl);
        
        $onefileExists = $testLib->oneElementExists();
        $this->_infoNow( 'answer', 'onefileExists: '.$onefileExists);
        
        
        $answer = $testLib->get_filelist_hashes();
        $this->_infoNow( 'answer', 'num_files: '.sizeof($answer));
        
        if (sizeof($answer)) {
            $i=0;
            foreach($answer as $row) {
                if ($i>5) {
                    $this->_infoNow( 'answer', 'Row: ...');
                    break;
                }
                $this->_infoNow( 'answer', 'Row: '.print_r($row,1));
                $i++;
            }
        }

        $result=0;
        if (!empty($answer)) $result=1;
        $this->_saveTestResult('TEST01', $result); // save test result ...
        
        $winurl2 = $winurl.'/';
        $testLib = new oEXP_DATAURL_subs($winurl2);
        $answer = $testLib->get_filelist_hashes();
        $result=0;
        if (!empty($answer)) $result=1;
        $this->_saveTestResult('TEST02_withslash', $result); // save test result ...

        
        return (1);
    }
    
}
