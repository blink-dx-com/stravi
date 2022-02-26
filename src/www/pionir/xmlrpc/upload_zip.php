<?php
/**
 * upload an a ZIP-file, extract it to generated 'windows_path'
 * - Prerequisites: 
 *   - SESSION-ID must exist (given by parameter)
 *   - $_SESSION['globals']['app.upload_zip'] must be configured
 *   - generated 'windows_path' must be WRITEABLE by Partisan-Web-User!
 * - throws errors on error-log.
 * Description:
 * - $_FILES['s_file'] contains a valid ZIP-File
 * OUTPUT:
 *   header("icono-err-code: {NUM}");     // NUM: 0 : ok; NUM>0: ERROR
 *   header("icono-err-text: {MESSAGE}"); // text message
 *   header("instrument_type:".  $instrument_type);
 *   header("filename:".  $filename);
 *   header("dest_path:".  $path_arr['windows_path']);
 *   header("Content-type: text/html");
 *   
 *   echo({OUT-MESSAGE});   // OK: o.k.; icono-err-text:TEXT => error
 * @package upload_zip.php
 * @swreq UREQ:FS-INT02.R01 upload ZIP-Archive and extract it to an DFS-directory
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param   string $instrument_type e.g. 'NGAi', 'NAT' , ...
 * @param   array $_FILES['s_file'] - the uploaded file
 *   'name' : NAME of ZIP-File, must have special convention:
 *      A) YYYY-MM-DD.xxxxxx.xxxx.xxx.zip (extension ".zip" is optional)
 *      B) YYYYMMDD_time.xxxx.xxx.zip (extension ".zip" is optional)
 *   
 * @param   long $sess_id SESSION-ID
 * @version $Header: trunk/src/www/pionir/xmlrpc/upload_zip.php 59 2018-11-21 09:04:09Z $
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


$sess_id  = $_REQUEST['sess_id'];
$instrument_type = $_REQUEST['instrument_type'];
$s_file   = $_FILES['s_file'];

if ( isset($sess_id) ) session_id($sess_id); // due to XML-RPC
session_start();


require_once ('reqnormal.inc');
require_once ('lev1/f.zip_instr_upload.inc');


if( !glob_loggedin() ) {
    header("icono-err-code: 4");
    header("icono-err-text: Invalid session ID !");
    echo("icono-err-text: Invalid session ID !");
    exit(4); 
};



global $error;
$FUNCNAME= 'upload_zip.php';


$error = & ErrorHandler::get();
$sqlo  = logon2( );

$helplib = new f_zip_instr_upload($instrument_type);

if (!isset($instrument_type))  { 
	$helplib->errorout(2, 'No instrument_type provided !');
};
if (!is_array($s_file)) { 
	$helplib->errorout(3, "No file data provided !");  
};

if (!$helplib->instrument_type_allowed()) {
	$helplib->errorout(4, "instrument_type ".$instrument_type." not allowed!");
}

$filename = $_FILES['s_file']['name'];

header("session_id:".$sess_id );
header("instrument_type:".  $instrument_type);
header("filename:".  $filename);


$tmp_file = $_FILES['s_file']['tmp_name'];
$answer = $helplib->upload($filename, $tmp_file);
if ($answer[0]>0) {
	$helplib->errorout($answer[0], $answer[1]);
}

$path_arr=$answer[2];
header("dest_path:".  $path_arr['windows_path']);
header("icono-err-code: 0");
header("icono-err-text: Everything ok !");
header("Content-type: text/html");
echo("OK");
