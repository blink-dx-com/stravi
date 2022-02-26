<?php
/*MODULE:  export.clean.php
  DESCR:   clean Paxml-files
  AUTHOR:  qbi
  INPUT:   ---
  VERSION: 0.1 - 20080401
*/
extract($_REQUEST); 
session_start(); 


    require_once("PaXMLLib.inc");
    require_once("class.history.inc");
	require_once("javascript.inc");
	
	$htmlOutput = NULL;
	$dummyClass = NULL;
	$paxLib = new PaXMLLib($htmlOutput, $_SESSION['sec']['dbuser'], $_SESSION['sec']['passwd'], $_SESSION['sec']['db'], $_SESSION['sec']['_dbtype'], $dummyClass);
	
	$path = $paxLib->getPaxmlWorkPath();

    echo "<html><body bgcolor=white text=black><br><ul><font face=Arial,Helvetica size=2>";
    echo "deleting temporary files (".$path.") ... <br>";
    if (file_exists($path)) {
		echo "deleting directory ... <br>";
        if (!PaXMLLib::removeDirRecursivly($path))
            die("<br><font color=red>Server file handling error. Removing previously used work path"
                . " at temporary directory failed.</font></font></ul>");
    }       
    if (file_exists($path . ".tar")) {
		echo "deleting TAR-file ... <br>";
        if (!unlink($path . ".tar"))
            die("<br><font color=red>Server file handling error. Removing temporary file failed.</font></font></ul>");
	}
    echo "<font color=green>ok</font></font></ul>";

    $hist_obj = new historyc();
    $lastproj = $hist_obj->last_bo_get( "PROJ" );

    if ($lastproj != null) {
		$url = "../../edit.tmpl.php?t=PROJ&id=" . $lastproj ;
        js__location_replace($url, "project");
	}
	
	echo "</html>\n";
