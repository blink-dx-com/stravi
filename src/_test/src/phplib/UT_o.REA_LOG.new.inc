<?php
class UT_o_REA_LOG_new_inc extends gUnitTestSub {
	
    
    function dotest( &$sqlo, $options ) {
    
        $data1 = array( 'KEY'=>'test1', 'STATUS'=>1,
            'data_path'=>'Z:\hallo', 'qc_version'=>'qc1.0', 'date'=>'2021-04-08 14:34' );
        $data2 = array( 'KEY'=>'test2', 'STATUS'=>0,
            'data_path'=>'Z:\hallo3', 'qc_version'=>'qc1.2', 'date'=>'2021-04-08 17:33' );
        
        $data3 = array( 'KEY'=>'test1', 'STATUS'=>0,
            'data_path'=>'Z:\hallo4', 'qc_version'=>'qc1.5', 'date'=>'2021-04-09 17:33' );
        
        $xdata = array($data1);
        $this->_infoNow( 'START: xdata', print_r($xdata,1) );
        
        oREA_LOG_new::DATAX_update_entry($xdata, 'test2', $data2);
        $this->_infoNow( 'END: xdata', print_r($xdata,1) );
    	$result=0;
    	if (sizeof($xdata)==2) $result=1;
    	$this->_saveTestResult('TEST01', $result); // save test result ...
    	
    	oREA_LOG_new::DATAX_update_entry($xdata, 'test1', $data3);
    	$this->_infoNow( 'END: xdata', print_r($xdata,1) );
    	$result=0;
    	if (sizeof($xdata)==2) $result=1;
    	$this->_saveTestResult('TEST02', $result); // save test result ...
    
    	
    	return 1;
    }

}
