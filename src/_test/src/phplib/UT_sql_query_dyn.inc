<?php
// phplib/sql_query_dyn.inc

class UT_sql_query_dyn_inc extends gUnitTestSub {
	
    
    // return: 0 : not passed, 1: passed
    function dotest( &$sqlo, $options ) {
    
        $tablename='EXP';
        $condclean=1;
        $tableSCond='x.EXP_ID=100';
        $tabSearchCond = NULL;
        
        $opt = array("infoCond"=>'exp-id:100');
        $answer = selectGet($sqlo, $tablename, $condclean, $tableSCond, $tabSearchCond, $opt);
    	
        $this->_infoNow('INPUT:', "tablename:$tablename condclean:$condclean, tableSCond:$tableSCond");
        $this->_infoNow('OUTPUT:', print_r($answer,1));
  	
    	$this->_saveTestResult('TEST01', 1); // save test result ...
        
       
    	return 1;
    }

}
