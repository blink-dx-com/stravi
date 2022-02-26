<?php

extract($_REQUEST); 
session_start(); 


	require_once("PaXMLValidator.inc");

    if (is_null($cct_output))
        $cct_output = PaXML_TINYVIEW;

	$pxml = new PaXMLValidator($_SESSION['sec']['dbuser'], $_SESSION['sec']['passwd'], $_SESSION['sec']['db'], $_SESSION['sec']['_dbtype'], $cct_output);
    echo "<ul>";
    echo "<br><font face=Arial,Helvetica size=4 color=black><b>Paxml Validator</b></font>";
    echo "<hr width=300 noshade size=1 align=left noshade><br>";
	echo "<font face=Arial,Helvetica size=1 color=black>";
	$pxml->start($cct_file, $_FILES["cct_file"]["name"], $cct_proj);
	echo "</font>";
    echo "<ul>";

?>
