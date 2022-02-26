<?php
/**
 *  downloads a file in the workfile directory
 *	- security: checks file name with realpath()
 *	- work_path+SESSIONDIR can be created by using class workDir() in f.workdir.inc
 *
 * @namespace core::gui
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param string $file filename (including dir-name), relative to WORK_PATH <pre>
 *    globals['work_path'] . "/" . SESSIONDIR . "/". file;
 *    </pre>
 * @param string $name[optional]     name of shown file
 * @param string $mimetype[optional] mimetype
 */		  

session_start(); 

require_once ('reqnormal.inc');

$title = "Download session-data-file";

$file= $_REQUEST["file"];
$name= $_REQUEST["name"];
$mimetype= $_REQUEST["mimetype"];

if ( !glob_loggedin() ) {
	echo "ERROR: You are not logged in!";
	exit;
}
$file = trim($file);
if ( $file==NULL ) {
	echo "ERROR: No file given!";
	exit;
}

if ( $name==NULL ) {
	$name = basename($file);
}

if ($mimetype=="") {
    $mimetype="application/x";
}


$fullname = $_SESSION['globals']['work_path'] . "/pdir_". session_id() . "/". $file;
// check file name for "../" and so on 
if ( !file_exists($fullname) ) {
	echo "Error: $title: Filename '$file' does not exist in your work-dir!<br>\n";
	exit;
}

if ( $fullname != realpath($fullname) ) {
	echo "Error: $title: Filename '$file' not allowed!<br>\n";
	if ( $_SESSION['sec']['appuser'] == "root" ) {
		echo "fullname: '$fullname' | realpath: '".realpath($fullname) ."'<br>";
	}
	exit;
}

$sizex    = filesize($fullname);

header('Content-type: "'.$mimetype.'"');
header('Content-Disposition: attachment; filename="'.$name.'"');
header('Content-length: '.$sizex);

fpassthru(fopen($fullname, "r"));
