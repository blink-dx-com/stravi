<?php
require_once('date_funcs.inc');
require_once($_SESSION['s_sessVars']['loginPATH'].'/api/lib/metacall.inc');


class UT_f_instr_upload_blk_inc extends gUnitTestSub {
	
function __construct() {
	$this->module_noPreLoad =0;  // 0,1 - do NOT preload ? can be set in constructor of class
	$this->GUI_test_flag    =0;
}


// return: 0 : not passed, 1: passed
function dotest( &$sqlo, $options ) {
    global $error;
    $FUNCNAME= __CLASS__.':'.__FUNCTION__;

    $this->_testLib = new f_instr_upload_blk('blink.one');
    
    
	
    /**
	 * @param string $tmp_file temporary file to be uploaded  /tmp/phpk61OAI
	 * @param string $relpath   "segmentation/cycle_08_ramp_01_temp_058_row1_col1_z1_fluor_seg.png"
    */
    $tmpfile  = tmpfile();
    $tmp = stream_get_meta_data($tmpfile);
    $tmpfile_path = $tmp['uri'];
    fwrite($tmpfile, "UnitTest-content");

    $exp_context = 'UT_'.$FUNCNAME.'_'.rand(10000, 100000000);
    
    $answer = $this->_testLib->upload($sqlo, $exp_context, $tmpfile_path, "segmentation/file1.png");
    if ($error->Got(READONLY))  {
        $errLast   = $error->getLast();
        $error_txt = $errLast->text;
        $error_id  = $errLast->id;
        $this->_compareTestResult('TEST01', 25, $error_id, 'Error_now:'. $error_txt);
        $error->reset();
    } else {
        $this->_saveTestResult('TEST01', 0, 'Expected error');
    }
    
    
    
    $test_data_dir = $this->_getTestDir_path();
    $json_file = $test_data_dir.'/www/lab_blk/api/mods/UT_oEXP_create.json';
    if (!file_exists($json_file)) {
        throw new Exception('Test-file "'.$json_file.'" not exists.');
    }
    
    $this->_infoNow('UT-JSON-file', $json_file);
    
    $exp_content = file_get_contents($json_file);
    $exp_dict = json_decode($exp_content, TRUE);
    
    // manipulate DICT to create a new unique path
    $exp_dict['run']['start'] = date_unix2datestr(time(),1);
    
    
    $args_use=array(
        'args'=>array(
            'vals'=>array(
                'NOTES'=>'UnitTest '.$FUNCNAME,
                'EXT_ID'=>$exp_context
            )
        ),
        'json'=>$exp_dict
    );
    $metalib = new metacall('LAB/oEXP_create', $args_use);
    $answer = $metalib->run();
    if ($error->Got(READONLY))  {
        $error->set( $FUNCNAME, 1, 'Error on UT-experiment creation' );
        return;
    }
    
    $test_name='TEST02';
    $tmpres=0;
    $exp_id = $answer['data']['objid'];
    if ($answer['data']['objid']>0) $tmpres=1;
    $this->_infoNow( 'Result:'.$test_name, print_r($answer,1) );
    if ( is_array($answer['error']) ) {
        $this->_infoNow( 'Error_from_call', print_r($answer['error'],1) );
    }
    $this->_saveTestResult('TEST02', $tmpres, 'Created Experiment:'.$exp_id);
    

    $answer = $this->_testLib->upload($sqlo, $exp_context, $tmpfile_path, "segmentation/file2.png");
    $tmpres=0;
    if ($answer['exp_id']>0) $tmpres=1;
    $this->_saveTestResult('TEST03', $tmpres, 'Found Experiment:'.$answer['exp_id']);
 
    

	fclose($tmpfile); // dies entfernt die Datei
	
	return (1);
}

}
