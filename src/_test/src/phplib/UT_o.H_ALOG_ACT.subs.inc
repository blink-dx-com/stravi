<?php
class UT_o_H_ALOG_ACT_subs_inc extends gUnitTestSub {
	

    function dotest( &$sqlo, $options ) {
    
        $tablename='MORDER';
        
        $sqlsel = "count(1) from H_ALOG_ACT where TABLE_NAME is NULL or TABLE_NAME like " .$sqlo->addQuotes('%'.$tablename.'%');
        $sqlo->Quesel($sqlsel);
        $sqlo->ReadRow();
        $ori_cnt = $sqlo->RowData[0];
        
        $states = oH_ALOG_ACT_subs::getActionEntries($sqlo, $tablename);
    	
    	$this->_infoNow('MORDER:states', print_r($states,1));
    	$this->_infoNow('MORDER: expected number', $ori_cnt);
    
    	$result=0;
    	if($ori_cnt==sizeof($states)) $result=1;
    	$this->_saveTestResult('TEST01', $result); // save test result ...
        
    
    	return 1;
    }

}
