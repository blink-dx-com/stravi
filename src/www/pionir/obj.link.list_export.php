<?php
/**
 * export document-files from list
 * @package obj.link.list_export.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param    $go	: 0 html form
	  	    : 1 export
			: 2 clear temp-dir
	$parx["defext"]
 */
session_start(); 

require_once('db_access.inc');
require_once('func_head.inc');
require_once('object.subs.inc');
require_once('down_up_load.inc');
require_once("sql_query_dyn.inc");
require_once("f.workdir.inc");  
require_once("Tar.inc");  
require_once("f.compress.inc"); 
require_once("o.LINK.subs.inc"); 
require_once("visufuncs.inc");
//

function this_getExtFromMIME( &$keys, $mimetype ) {

	
	foreach( $keys as $key=>$val) {
		if (strstr($mimetype,$key)!=NULL) {
			$resext = $val;
			break;
		}
	}
	reset ($keys); 
	if ($resext=="") { // get last part of mime-type
		$tmppos = strrpos($mimetype,"/");
		$resext = substr($mimetype,$tmppos+1);
	} 
	
	return ($resext);

}

function formshow($parx) {
	require_once ('func_form.inc');
	
	
	
	echo "<I>Info: Produce TAR-file from selected document-files.</I><br>";
	
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Start export";
	$initarr["submittitle"] = "Start export NOW!";
	$initarr["tabwidth"]    = "AUTO";

	$hiddenarr = NULL;

	$formobj = new formc($initarr, $hiddenarr, 0);
	
	$fieldx = array ( "title" => "Default extension", "name"  => "defext",
			"object" => "text",
			"val"   => "", 
			"notes" => "use this file extension, if document name contains no extension (e.g. 'icrun' )" );
	$formobj->fieldOut( $fieldx );
	

	$formobj->close( TRUE );
	

}

function thiserrout($imgid, $name, $text ) {
	$notes = "<font color=red><b>Error:</b></font> $text\n";
	thisPrint ($imgid, $name, $notes);
}

function thisPrint ($imgid, $name, $notes) {
    // FUNCTION: print out info text
	global $imgcount;
    echo "<font color=gray>$imgcount.</font> ID:$imgid <B>".$name."</B> ". $notes."<br>\n";
}


function thistar($path, $files, $tarfile) {
	$tar = new Tar();
	$tar->create( $tarfile );       // create tar file
	$tar->cd($path . "/");              // change to path for taring

	// read file by file from dir and add to tar archive. catch readdir
	// returns '.', '..', xml file or zip file (?)
	
	foreach( $files as $dummy=>$filename) {
		$tar->add($path . "/" . $filename);
	}
	$tar->close(); 
}


/* Open connection to DBMS */

$error      = & ErrorHandler::get();
$DB_Handle  = logon2( $_SERVER['PHP_SELF'] );
//$DB_Handle1 = logon2( $_SERVER['PHP_SELF'] );

$go = $_REQUEST["go"];
$parx= $_REQUEST["parx"];

$infox 		  = NULL;
$tablename	  = "LINK";
$tablenice 	  = "document";

$IMG_ID 	  = 0;
$workSubDir   = "img.list_export";



$title = "Export files of a selected list of documents";
$infoarr=array();
$infoarr['title'] = $title;
$infoarr["form_type"]= "list";
$infoarr['obj_name'] = $tablename;
$infoarr['sql_obj']  = &$DB_Handle;
$infoarr["obj_cnt"]  = 1;   

$keysx = array(
	"msword"   => "doc",
	"ms-word"  => "doc",
	"msexcel"  => "xls",
	"ms-excel" => "xls",
	"pdf"	   => "pdf",
	"powerpoint"=>"ppt"
	);

$pagelib = new gHtmlHead();
$pagelib->startPage($DB_Handle, $infoarr);
echo "<ul>";

$workdirObj = new workDir();
$tmpdir   = $workdirObj->getWorkDir ( $workSubDir );
if ($error->Got(READONLY))  {
	$error->set('createWorkDir()', 2, "Creation of work-dir failed");
	$error->printAll();
	htmlFoot();    # TBD special htmlFoot()
}

if ( $go == 2 ) {
	echo "remove temp files ...<br>";
	$workdirObj->removeWorkDir( );
	htmlFoot("Ready...<br><hr>");
}

//
// check exp list
//
$sqlopt=array();
$sqlopt["order"] = 1;
$sqlAfter  = get_selection_as_sql( $tablename, $sqlopt);

$tmp_info   = $_SESSION['s_tabSearchCond'][$tablename]["info"];
if ($tmp_info=="") 
	htmlFoot("Attention", $stopReason." Please select elements of '".$tablenice."'!");


$sqlsLoop = "SELECT x.LINK_ID FROM ".$sqlAfter;
$DB_Handle->query($sqlsLoop);
if ($DB_Handle->ReadRow()) {
	$IMG_ID = $DB_Handle->RowData[0];
}
if ( !$IMG_ID ) 
	htmlFoot("Attention", " Please select elements of '".$tablenice."'!");

	
if ( !$go ) {
	formshow($parx);
	echo "<font size=+1><b>I. Analyse documents.</b></font><br><br>\n";
} else {
	echo "<font size=+1><b>II. Collect documents now. Please wait ......</b></font> [<a href=\"".$_SERVER['PHP_SELF']."\">Start again</a>]<br><br>\n";
	if ($parx["defext"]!="") echo "Default document-extension: <b>".$parx["defext"]."</b><br>\n";
}


$tarfileSingle = "docfiles.tar";
$tarfile = $tmpdir."/".$tarfileSingle;
$files = NULL;

$badchars = array( "\\","/", "*", "?", "\"", ":", ">", "<", "|" );
$imgcount=0;
$sum     = 0;
$goodCnt = 0;

$sqlsLoop = "SELECT x.LINK_ID, x.NAME, x.MIME_TYPE FROM ".$sqlAfter;
$DB_Handle->query($sqlsLoop);
while ($DB_Handle->ReadRow()) {
	$errmess     = "";
	$extIsInName = 0;
	$imgcount++;
	$IMG_ID 	= $DB_Handle->RowData[0];
	$IMG_NAME 	= $DB_Handle->RowData[1];
	$IMG_MIME   = $DB_Handle->RowData[2];
	$htmlname   = " ".htmlspecialchars($IMG_NAME);
	$FileNameNew = trim($IMG_NAME);	// generate a nicve file-name ...
	$FileNameNew = str_replace( $badchars, "_", $FileNameNew);
	
	$imgext = "";
	do {
	
		$imageOriName = linkpath_get($IMG_ID);
		if ( !file_exists($imageOriName) ) {
			$errmess = "document does not exist on server. ";
			break;	
		}
		 
		if ($IMG_NAME!="") {
			$imgext = "";
			$tmppos = strrpos($IMG_NAME,".");
			if ($tmppos) {
				$imgext = substr($IMG_NAME,$tmppos+1);
				$extIsInName = 1;
				break;
			}
				
		} 
		
		if ($IMG_MIME!="" AND $parx["defext"]=="") {
			$imgext = this_getExtFromMIME( $keysx, $IMG_MIME );
			$htmlname .= " <font color=gray>add extension:</font> <B>".$imgext."</B>";
		}
		
	} while (0);
	
	if ($errmess!="") {
		thiserrout($IMG_ID, "",$errmess."<font color=gray>docname: </font>".$htmlname);
		continue;
	}
	$filenameSingle = $FileNameNew;
	if ( $imgext=="" AND $parx["defext"]!="") $imgext = $parx["defext"];
	// echo "extIsInName:$extIsInName <br>"; 
	if ( ($imgext!="") AND !$extIsInName ) $filenameSingle .= ".".$imgext;
	$filenameTmp    = $tmpdir."/".$filenameSingle;

	if ( $go ) {
		if (!copy ($imageOriName, $filenameTmp) ) {
			thiserrout($IMG_ID, $filenameSingle, "collecting file failed. origin: ".$htmlname);
			continue;
		} 
	}
	
	$goodCnt++;
	$tmpsizeMb = filesize($imageOriName) * 0.000001;
	$notes =  "<font color=green><b>ok:</b></font>, size:$tmpsizeMb Mb)\n";
	thisPrint ($IMG_ID, $filenameSingle, $notes);
	$sum = $sum + $tmpsizeMb;
	$files[] = $filenameSingle;
	// echo $filenameTmp."<br>\n";
	
	while (@ob_end_flush());
	
}


echo "<br>";
$tmparr = NULL;
$tmparr[] = array ( "Analysed documents:", "<b>$imgcount</b>");
$tmparr[] = array ( "Good images:", "<b>$goodCnt</b>");
$tmparr[] = array ( "Sum:", "<b>$sum</b> Mb");
$optx = array("title"  =>"Results");
$dummy=NULL;
visufuncs::table_out( $dummy, $tmparr,  $optx);
echo "<br>";

if ( !$goodCnt ) {
	htmlFoot("Info", "No documents collected!");
}

if (!$go) {
	htmlFoot("<br><hr>");
}

// tar files
thistar($tmpdir, $files, $tarfile);

if ( ($fhd = fopen($tarfile , "r")) == FALSE) {
	htmlFoot("Error", "ERROR: could not read TAR-file $tarfile");
}
fclose($fhd);

//$ret = $compressObj->zip($tarfile);
//if ( $ret>0 ) $tarfileSingle = $tarfileSingle . ".gz";
echo "TAR Ready ...<br>\n";

$tmpsizeMb = filesize($tarfile) * 0.000001;

echo "<br><b>1.</b> <a href=\"f_workfile_down.php?name=".$tarfileSingle."&file=".$workSubDir."/".$tarfileSingle."\"><B>Download TAR-file</B></a> (size: <b>$tmpsizeMb</b> Mb)<br>\n";
echo "<br><b>2.</b> <a href=\"".$_SERVER['PHP_SELF']."?go=2\"><B>Clean temporary files</B></a><br>\n";

htmlFoot("<br><hr>");


// tar: $tarfile
// tar -cvf $tarfile < $tempdir
// header ....
// fertsch
