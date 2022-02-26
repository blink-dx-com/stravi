<?php

extract($_REQUEST); 
session_start(); 


    require_once("PaXMLLib.inc");
    require_once("class.history.inc");

    echo "<html><body bgcolor=white text=black><br><ul><font face=Arial,Helvetica size=2>";
    echo "deleting temporary files ... ";
    if (file_exists($path))
        if (!PaXMLLib::removeDirRecursivly($path))
            die("<br><font color=red>Server file handling error. Removing previously used work path"
                . " at temporary directory failed.</font></font></ul>");
            
    if (file_exists($path . ".tar"))
        if (!unlink($path . ".tar"))
            die("<br><font color=red>Server file handling error. Removing temporary file failed.</font></font></ul>");

    echo "<font color=green>ok</font></font></ul>";

    $hist_obj = new historyc();
    $lastproj = $hist_obj->last_bo_get( "PROJ" );
                            
    echo "<html><meta http-equiv='refresh' content='0; URL=../../edit.tmpl.php?tablename=PROJ&id=" . $lastproj . "'></html>";
?>
