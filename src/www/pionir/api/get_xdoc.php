<?php
/**
 * 
 * download a file form an experiment base folder
 * - Prerequisites: 
 *   - SESSION-ID must exist (given by parameter)
 *   - $_SESSION['globals']['app.upload_zip'] must be configured

 * - OUTPUT:<pre>
 *   header("http... 200 OK")
 *     on_error:  header("http... 500 Internal server error")
 *   header("error-description: {STRING}"); 
 *   header("Content-type: text/html");
 * </pre>  
 * @package get_xdoc.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param string $filename "result/cycle_45_ramp_01_temp_058_row1_col1_results.txt", 
 * @param int $exp_id: 56
 * @param string session_id: SESSION-ID 
 * @unittest exists
 * @throws errors:
 *  
 */
require_once 'lib/rest_lib.inc';
require_once 'lib/api_debug.inc';

//$params_json = $_REQUEST['parameters'];
$filename  = $_REQUEST['filename'];
$exp_id    = $_REQUEST['exp_id'];

/*
$my_file = fopen( '/tmp/xml_rpc_log.log', 'a' );
if ($my_file) {
    fwrite($my_file,"<GET_DOC: date=\"". date("Y-m-d H:i:s"). ": ". print_r($_REQUEST,1) . "\n");
    fclose($my_file);
}
*/


$sess_id  = $_REQUEST['session_id'];
if ($sess_id==NULL) {
    rest_lib::error_out(2, 'no session_id in input-vars.');
}

if ($exp_id==NULL) {
    rest_lib::error_out(3, 'input missing: exp_id');
}
if ($filename==NULL) {
    rest_lib::error_out(3, 'input missing: filename');
}


$instrument_type = 'blink.one';      //$_REQUEST['instrument_type'];

if ( isset($sess_id) ) session_id($sess_id); // due to XML-RPC
session_start();

$output_debug = 2;
$logging_lib = new api_debug($output_debug);
$logging_lib->log_input('get_doc', $_REQUEST);


require_once ('reqnormal.inc');
require_once ('import/f.instr_upload_blk.inc');


if( !glob_loggedin() ) {
    rest_lib::error_out(4, 'Invalid session ID');
}

global $error;
$FUNCNAME = basename(__FILE__);


$error = & ErrorHandler::get();
$sqlo  = logon2( );

if (!isset($instrument_type))  {
    rest_lib::error_out(6, 'No instrument_type provided !');
};

$helplib = new f_instr_upload_blk($instrument_type);


if (!$helplib->instrument_type_allowed()) {
    $tmp = print_r($helplib->get_types(),1);
	rest_lib::error_out(8, "instrument_type ".$instrument_type." not allowed! ($tmp)");
}



$filepath_full   = $helplib->download($sqlo, $exp_id, $filename);
if ($error->Got(READONLY))  {
    $errLast   = $error->getLast();
    $error_txt = $errLast->text;
    $error_id  = $errLast->id;
    rest_lib::error_out($error_id, $error_txt);
}

$file_length = filesize($filepath_full);

$headerarr   =array();
$headerarr[] = "Content-Length: ".$file_length;
$headerarr[] = 'Content-Disposition: attachment; filename="'.basename($filepath_full).'"';
$headerarr[] = "Content-type: application/octet-stream";
rest_lib::output_start($sess_id, $headerarr);
readfile($filepath_full);






