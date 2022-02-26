<?php
/**
 * - starts the PAXML import
   - advanced: if you want to import a LOCAL file on the server like '/tmp/experiment_data.paxml.tar'
           then: give the file as $cct_file
 * @package import.php
 * @author  Rogo, Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param    
 *  $_FILES['cct_file']  (the uploaded file in the temp-directory) 
            or 
    $cct_file : STRING: if $cctopt["tempdir"] is set
    $cct_proj  (-1 for root project)
	$cct_output : 0,1,2,3,4, ... output level
	$cctopt array of options
	  ["update"]["files"]  = 1 : update attached files
	  ["asarchive"]        = 1 : import as archive
	  ["tempdir"]          = 
	  		- use the "tempdir" (after WORK_PATH) instead of default path with SESSION_ID
		 	- e.g. "pxmlexport.76b7a67c38582b823e732fa6e141917e" 
  			- do NOT remove the files on this "tempdir"
  							
 	
 * @version $Header: trunk/src/www/pionir/impexp/partisanxml/import.php 59 2018-11-21 09:04:09Z $
 */

extract($_REQUEST); 
session_start(); 

require_once ('reqnormal.inc');
require_once("PaXMLReader.inc");

$error = & ErrorHandler::get();
$sql   = logon2(  );
if ($error->printLast()) htmlFoot();

$flushLib = new fProgressBar( 100 ); // default 100

$title   = "Paxml Import";
$infoarr = NULL;
$infoarr["help_url"] = "partisanxml.import.html";
$infoarr["title"]    = $title;
if ($cct_proj) {
	$infoarr["form_type"]= "obj";
	$infoarr["obj_name"] = "PROJ";
	$infoarr["obj_id"]   = $cct_proj;
}
$infoarr["css"] = $flushLib->getCss(1);
$infoarr["javascript"] = $flushLib->getJS();

$pagelib = new gHtmlHead();
$headarr = $pagelib->startPage($sql, $infoarr);

    if (is_null($cct_output))
        $cct_output = PaXML_QUIET; // old: TINY

    $cctopt = $_REQUEST['cctopt'];
    
	$xopt = NULL; // options
	if ($cctopt["update"]["files"]) $xopt["update"]["files"] = 1;
	if ($cctopt["asarchive"]) 		$xopt["asarchive"] = 1;
	if ($cctopt["tempdir"]!="") 	$xopt["tempdir"]   = $cctopt["tempdir"];
	
	$cctopt["isp"] = trim($cctopt["isp"]);
	if ($cctopt["isp"]!="")		{
		$tmparr = explode(':',$cctopt["isp"]);
		$xopt["isp"]   = array('WIID'=>'http://www.clondiag.com/magasin/?db='.$tmparr[0],'ROID'=>$tmparr[1]);
	}
	
	$html_lib    = new PaXMLHTMLOutputImport(0);
	
	
	$remoteClass = NULL;
	$lib_paxml_help = new PAXMLLib($html_lib, $_SESSION['sec']['dbuser'], $_SESSION['sec']['passwd'], $_SESSION['sec']['db'], $_dbtype, $remoteClass);
	
	$pxml = new PaXMLReader($_SESSION['sec']['dbuser'], $_SESSION['sec']['passwd'], $_SESSION['sec']['db'], $_dbtype, $cct_output, $xopt);
	$pxml->html->setProgressLib($flushLib);
    
	
	
	echo "<ul>";
    
    
	
	if ($cctopt['tempdir']!=NULL) {
		$cct_file    = $lib_paxml_help->getWorkPath().'/'. $cctopt['tempdir'].'/'.$_REQUEST['cct_file'];
		$orgFilename = $_REQUEST['cct_file'];
	} else {
		$cct_file    = $_FILES["cct_file"]["tmp_name"];
		$orgFilename = $_FILES["cct_file"]["name"];
	}
	
	if (!file_exists($cct_file)) {
		$pagelib->htmlFoot('ERROR', 'Uploaded file not found. May be too large?');
	}
	
	$pxml->start($cct_file, $orgFilename, $cct_proj);
	
    echo "<ul>";

$pagelib->htmlFoot();
