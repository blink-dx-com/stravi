<?
extract($_REQUEST); 
session_start(); 


    header("Content-type: application/xml");
    header("Content-Disposition: attachment; filename=paxml.xml");
  
    fpassthru(fopen($_SESSION['globals']["work_path"] . "/pxmlexport." . session_id() . "/export.xml", "r"));
?>
