<?php

class UT_f_read_file_meta_inc extends gUnitTestSub {

    
    function _read_file($testid, $filename, $file_type) {
        
        $file_lib = new f_read_file_meta($filename, $file_type);
        $file_lib->open_file();
        $headers   = $file_lib->get_headers();
        $comments = $file_lib->getComments();
        
        $this->_infoNow( 'COMMENTS: ', print_r($comments,1) ); // just info for the test
        $this->_infoNow( 'HEADER: ', print_r($headers,1) ); // just info for the test
        
        while ($file_lib->read_line()) {
            $row = $file_lib->get_data_row();
            $this->_infoNow( 'row:', print_r($row,1) ); // just info for the test
        }
        
        $result=0;
        if (!empty($headers))  $result=1;
        $this->_saveTestResult($testid, $result); // save test result ...
    }
    
    // return: 0 : not passed, 1: passed
    function dotest( &$sqlo, $options ) {
        
        $data_path = $this->_getTestDir_path();
        $filename = $data_path.'/phplib/import/f.read_file_meta.01.csv';
        $file_type='csv';
        $this->_read_file('TEST01', $filename, $file_type);
        
        
        
        $filename = $data_path.'/phplib/import/f.read_file_meta.02.xlsx';
        $file_type='excel';
        $this->_read_file('TEST02', $filename, $file_type);
        
        

        return (1);
    }
    
}
