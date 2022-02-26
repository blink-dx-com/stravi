<?php
require_once(dirname(__FILE__).'/../../../_test_lib/http_client.inc');
require_once('date_funcs.inc');
require_once($_SESSION['s_sessVars']['loginPATH'].'/api/lib/metacall.inc');

class UT_set_xdoc_php extends gUnitTestSub {
	
function __construct() {
	$this->module_noPreLoad=1;
	//$this->GUI_test_flag = 1;
}

function _crea_exp($exp_context) {
    global $error;
    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
    
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
    $exp_dict['name'] = 'UT_'.__CLASS__;
    
    
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
    $tmpres=0;
    $exp_id = $answer['data']['objid'];
    if ($answer['data']['objid']>0) $tmpres=1;
    $this->_saveTestResult('TEST01', $tmpres, 'Created Experiment:'.$exp_id);
    
    return $exp_id;
}

function _upload_one(&$http_lib, $params) {
    
    $exp_context  = $params['context'];
    $testfilename = $params['ori_file_path'];
    
    $tmp_var = json_encode( array(
        "filename"  => "result/cycle_45.png",
        "context"   => $exp_context,
        'session_id'=> session_id(),
        'overwrite' => $params['overwrite']
    ) );
    
    $input_vars = array( 'parameters' => $tmp_var);
    
    $upload_files = array();
    // generate RANDOM file names
    $upload_files[] = array(
        'field'=>'0',
        'path'=>$testfilename,
        'shortname'=>'UnitTest.rand'.rand(1,100000).'.png'
    );
    $this->_infoNow( 'Uploaded filename:', $upload_files[0]['shortname'] );
    
    $answer     = $http_lib->send($input_vars, $upload_files);
    $header_arr = $http_lib->get_http_headers();
    $this->_infoNow( 'http-header-answer:', print_r($header_arr,1) );
    
    $status = $http_lib->get_page_status();
    if ($status==200) {
        $answer_dict = json_decode($answer,TRUE);
    } else {
        
        $err_num  = 1;
        $err_text = '?';
        $headers = $http_lib->get_http_headers();
        $error_desc_json = $headers['error-description'];
        if ($error_desc_json!=NULL) {
            $error_desc_arr = json_decode($error_desc_json,TRUE);
            $err_text = $error_desc_arr['text'];
            if ($error_desc_arr['num']) $err_num  = $error_desc_arr['num'];
        }
        
        $answer_dict = array('error'=> array(
            'num'=>$err_num,
            'page_status'=>$status,
            'text'=>$err_text
        ));
        
    }
    
    
    
    return $answer_dict;
}
	

// return: 0 : not passed, 1: passed
function dotest( &$sql, $options ) {
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	$exp_context = 'UT_'.$FUNCNAME.'_'.rand(10000, 100000000);
	$this->_infoNow( 'Exp-Context', $exp_context);
	
	$exp_id = $this->_crea_exp($exp_context);
	if ($error->Got(READONLY))  {
	    $error->set( $FUNCNAME, 1, 'UT-Exp creation failed.' );
	    return;
	}
	$this->_infoNow( 'Exp-ID', $exp_id);
	
	$test_data_dir = $this->_getTestDir_path();
	$testfilename = $test_data_dir.'/www/pionir/api/set_doc_EX01.png';
	if (!file_exists($testfilename)) {
	    throw new Exception('Test-file "'.$testfilename.'" not exists.');
	}
	$testfilename2 = $test_data_dir.'/www/pionir/api/set_doc_EX02.png';
	if (!file_exists($testfilename2)) {
	    throw new Exception('Test-file "'.$testfilename2.'" not exists.');
	}
	
	$upload_url = $_SESSION['s_sessVars']['DocRootURL']. '/pionir/api/set_xdoc.php';
	
	$this->_infoNow( 'TestData', $testfilename);
	
	$params = array(
			"srv_url"  => $upload_url,
			"hostname" => $_SERVER['SERVER_NAME'],
			// "protocol"=>'ssl'
	);
	$http_lib = new http_client($params);
	
	$this->_infoNow( 'call URL-params', print_r($params,1) );	
	
	$answer = $this->_upload_one( $http_lib, array('context'=>$exp_context, 'ori_file_path'=>$testfilename) );
	$tmpres=0;
	$exp_id = $answer['data']['objid'];
	if ($answer['data']['exp_id']>0) $tmpres=1;
	$this->_saveTestResult('TEST02', $tmpres, 'Upload File to EXP:'.$answer['data']['exp_id']);
	
	// Test 03: save image again: NO overwrite
	$answer = $this->_upload_one( $http_lib, array('context'=>$exp_context, 'ori_file_path'=>$testfilename) );
	$tmpres=0;
	if ($answer['error']['num']>0) $tmpres=1;
	$this->_saveTestResult('TEST03_error', $tmpres,  'Error-Info: '.$answer['error']['text']);
	
	// Test 04: save image again: WITH overwrite other file
	$answer = $this->_upload_one( $http_lib, array('context'=>$exp_context, 'ori_file_path'=>$testfilename2, "overwrite"=>1) );
	$tmpres=0;
	if ($answer['data']['exp_id']>0) $tmpres=1;
	$this->_saveTestResult('TEST04', $tmpres,  'Upload File to EXP:'.$answer['data']['exp_id']);
	
	
	// provoke error: upload to unknown context
	$exp_context='UT_'.$FUNCNAME.'_'.rand(10000, 100000000); // new invalid context
	$answer = $this->_upload_one( $http_lib, array('context'=>$exp_context, 'ori_file_path'=>$testfilename) );
	
	$tmpres=0;
	if ($answer['error']['num']>0) $tmpres=1;
	$this->_saveTestResult('TEST05_error', $tmpres, 'Error-Info: '.$answer['error']['text']);
	

	$retval = 1;
	return ($retval);
	
}

}