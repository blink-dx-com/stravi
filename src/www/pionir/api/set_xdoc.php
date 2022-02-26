<?php
/**
 * upload an a file, extract it to generated 'windows_path'
 * - Prerequisites: 
 *   - SESSION-ID must exist (given by parameter)
 *   - $_SESSION['globals']['app.upload_zip'] must be configured
 *   - generated 'windows_path' must be WRITEABLE by Partisan-Web-User!
 * - throws errors on error-log.
 * - cache number of files: $_SESSION["s_sessVars"]['set_xdoc'] = array(context=> CNT)
 * Description:
 * - $_FILES[0] contains a File
 * - OUTPUT: <pre>
 *   header("err-code: {NUM}");     // NUM: 0 : ok; NUM>0: ERROR
 *   header("text: {MESSAGE}"); // text message
 *   header("instrument_type:".  $instrument_type);
 *   header("filename:".  $filename);
 *   header("dest_path:".  $path_arr['windows_path']);
 *   header("Content-type: text/html");
 *   
 *   echo({OUT-MESSAGE});   // OK: o.k.; icono-err-text:TEXT => error
 * </pre>
 * @package set_xdoc.php (30-01-2020: file was renamed set_doc.php to set_xdox.php because of apache rewrite problems
 * @swreq UREQ:upload ZIP-Archive and extract it to an DFS-directory
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param   array $_FILES[0] - the uploaded file
 *   'name' : NAME of File
 *   'tmp_name' : temporary uploaded file
 * @param string $parameters   JSON-dict
 *   "filename": "result/cycle_45_ramp_01_temp_058_row1_col1_results.txt", 
 *   "context": "15c06564-2966-11e9-ac6c-4cc"
 *   "overwrite" : [OPTIONAL] [0],1 overwrite existing file
 *   'session_id' SESSION-ID 
 * @version 
 * @unittest exists
 * @throws errors:
 *  
 *  2: No instrument_type provided !
 *  5: ZIP-Filename not allowed!
 *  6, Error on ZIP-File test!
 *  7, Too many files in ZIP!
 *  8, Too much data in ZIP!
 *  9, Error on ZIP-File extraction!
 *  10, MKDIR of new dir failed!
 *  12, Dest_dir already exists
 *  13, Error on Upload-Log-File
 */
require_once 'lib/rest_lib.inc';



$params_json = $_REQUEST['parameters'];
$params = json_decode($params_json, TRUE);
if ($params['context']==NULL) {
    rest_lib::error_out(1, 'no context in input-vars.');
}

$sess_id  = $params['session_id'];
if ($sess_id==NULL) {
    rest_lib::error_out(2, 'no session_id in input-vars.');
}

$output_debug = $_SESSION['globals']["xml_rpc.debug_level"];
if ($output_debug>0) {
    $my_file = fopen( '/tmp/xml_rpc_log.log', 'a' );
    if ($my_file) {
        fwrite($my_file,"<XXX_set_doc date=\"".date("Y-m-d H:i:s")."\" \n");
        fwrite($my_file,"<INPUT: ".print_r($_REQUEST,1).">\n");
        fclose($my_file);
    }
}

$instrument_type = 'blink.one';      //$_REQUEST['instrument_type'];

if ( isset($sess_id) ) session_id($sess_id); // due to XML-RPC
session_start();

$one_file_arr = $_FILES[0];


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

$context = $params['context'];
$MAX_CACHE=10;
// cache number of uploaded files for $params['context']
if ( !is_array($_SESSION["s_sessVars"]['set_xdoc']) ) {
    $_SESSION["s_sessVars"]['set_xdoc']=array();
}
if (!$_SESSION["s_sessVars"]['set_xdoc'][$context]) {
    if (sizeof($_SESSION["s_sessVars"]['set_xdoc'])>$MAX_CACHE) {
        // delete oldest element
        reset($_SESSION["s_sessVars"]['set_xdoc']);
        $first_key=key($_SESSION["s_sessVars"]['set_xdoc']);
        unset($_SESSION["s_sessVars"]['set_xdoc'][$first_key]);
    }
    $_SESSION["s_sessVars"]['set_xdoc'][$context] = 0;
}
$_SESSION["s_sessVars"]['set_xdoc'][$context]= $_SESSION["s_sessVars"]['set_xdoc'][$context] + 1;
    


$helplib = new f_instr_upload_blk($instrument_type);

if (!is_array($one_file_arr)) { 
	rest_lib::error_out(7, "No file data provided !");  
};

if (!$helplib->instrument_type_allowed()) {
    $tmp = print_r($helplib->get_types(),1);
	rest_lib::error_out(8, "instrument_type ".$instrument_type." not allowed! ($tmp)");
}

// $filename = $one_file_arr['name'];


$tmp_file  = $one_file_arr['tmp_name'];
$overwrite = $params['overwrite'];
$upload_out   = $helplib->upload($sqlo, $params['context'], $tmp_file, $params['filename'], $overwrite);
if ($error->Got(READONLY))  {
    $errLast   = $error->getLast();
    $error_txt = $errLast->text;
    $error_id  = $errLast->id;
    $error->set( $FUNCNAME, 1, 'Error on upload. Context:'. $params['context'].' FILE-CNT:'.
        $_SESSION["s_sessVars"]['set_xdoc'][$context]. ' filename: "'.$params['filename'].'" OVW?:'.$overwrite );
    rest_lib::error_out($error_id, $error_txt);
}

//$error->logx("INFO", __FILE__, 'Info: Input: ext_id:'.$params['context'].' filename:'.$params['filename'].' Output:'.print_r($upload_out,1) );

$data_out = array('path'=>$upload_out['windows_path'], 'exp_id'=>$upload_out['exp_id']);
rest_lib::output_start($sess_id);

echo json_encode(  array( 'data'=> $data_out)  );



