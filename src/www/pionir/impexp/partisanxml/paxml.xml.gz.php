<?
extract($_REQUEST); 
session_start(); 


    // require_once("down_up_load.inc");
    
    header("Content-Type: application/x-gzip");
    header("Content-Disposition: attachment; filename=paxml.xml.gz");

    // does not work, because header("Expires: 0"); makes problems to IE
    // set_mime_type ("application/x-gzip", "paxml.xml.gz")
    fpassthru(fopen($_SESSION['globals']["work_path"] . "/pxmlexport." . session_id() . "/export.xml.gz", "r"));
?>
