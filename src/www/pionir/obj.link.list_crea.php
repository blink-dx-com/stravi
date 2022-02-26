<?php
/**
 * upload zip-file, create a set of documents 
 * - documents are linked to current project
 * @package obj.link.list_crea.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param $userfile
  		   $go
  		   $parx
 */
session_start(); 


require_once ("reqnormal.inc"); // includes all normal *.inc files
require_once ("f.workdir.inc"); 
require_once ("f.directorySub.inc"); 
require_once ('func_form.inc');
require_once ("class.history.inc");
require_once ("insert.inc");
require_once ("o.LINK.subs.inc");
require_once ('o.PROJ.addelems.inc');

class oLINKlistcrea {

function __construct(&$sql, $go) {
	global $error;
	
	$workdirObj = new workDir();
	
	$noclean = 1;
	if (!$go) $noclean = 0;
    $this->sessdir   = $workdirObj->getWorkDir ( "o.LINK.list_crea", $noclean );
    if ($error->Got(READONLY))  {
        $error->set('oLINKlistcrea()', 2, "Creation of temp-dir failed");
        return;    
    }
	
	// get current project
	$hist_obj = new historyc();
	$this->projid  = $hist_obj->last_bo_get('PROJ');
	if (!$this->projid) {
		 $error->set('oLINKlistcrea()', 3, "Please touched a writeable project");
		 return;   
	}
	
	$o_rights = access_check($sql, "PROJ", $this->projid);
	if ( !$o_rights["write"]) {
		$error->set('oLINKlistcrea()', 4, "You do not have 'write' permission on project [ID:".$this->projid."]!");
		return;   
	}

}

function  getUpZipFile( $userfile, $userfile_name ) {
// FUNCTION: move uploaded file to SESION-Temp-dir
//			1. create temp-dir
//		    2. UNZIP file to temp-dir
//			3. return temp-path
     global $error;
     
     if ( !file_exists($userfile) ) { 
     	$error->set('getUpZipFile()', 1, "Could not find uploaded file '$userfile' original_name:'$userfile_name'");
        return;
     }
	 
	 $newUserFile = $userfile; // $this->sessdir."/".$userfile_name;
	 /*if (!copy( $userfile, $newUserFile ))  {
	 	$error->set('getUpZipFile()', 2, "Copy failed of uploaded-file name:[$userfile_name] from:[$userfile] to session-directory [".
			$this->sessdir."] !");
        return;
     }
	 */
	 
	 $tmpsize = filesize($newUserFile);
	 
     echo "<font color=gray>... unzip the uploaded file '$userfile_name':  (size: $tmpsize bytes)</font><pre>";
     $exestr = "unzip \"".$newUserFile."\" -d \"".$this->sessdir."\"";
     $answer = system ( $exestr , $tartra); 
     echo "</pre>\n";
	 
     $docarr = $this->getFilesFromTemp();
     
	 return ($docarr);

}

function getFilesFromTemp() {
	 global $error;
	 $scanObj = new fDirextoryC();
	 
     $docarr = $scanObj->scanDir( $this->sessdir );
	 if ( $error->Got(READONLY) ) {
	 	$error->set('getFilesFromTemp()', 1, "Error during temp-dir scanning");
        return;
	 }
	 
     if ( !sizeof($docarr) ) {
     	$error->set('getFilesFromTemp()', 2, "No documents found.");
        return; 
     }
	 
	 return ($docarr);
}

function form0( &$sql, $parx ) {
	// upload zip file
	global $error;
	
	
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Upload ZIP-file";
	$initarr["submittitle"] = "Upload";
	$initarr["tabwidth"]    = "AUTO";
	$initarr["ENCTYPE"]     = "multipart/form-data";

	$hiddenarr = NULL;
	$formobj = new formc($initarr, $hiddenarr, 0);
	
	$fieldx = array ( "title" => "ZIP-file", "name"  => "userfile",
			"object" => "file", "namex" => 1,
			"notes" => "zipped documents" );
	$formobj->fieldOut( $fieldx );
	
	$formobj->close( TRUE );
	
}

function form1( &$sql, $parx, $userfile, $userfile_name) {
	// upload zip file
	global $error;
	
	$docarr = $this->getUpZipFile( $userfile, $userfile_name );
	if ( $error->Got(READONLY) ) {
		$error->set('form2()', 1, "Upload ZIP-file failed.");
		return;
	}
	
	
	
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Prepare creation";
	$initarr["submittitle"] = "Create now!";
	$initarr["tabwidth"]    = "AUTO";

	$hiddenarr = NULL;

	$formobj = new formc($initarr, $hiddenarr, 1);
	$formobj->close( TRUE );
	
	echo "<br>Documents:<ul>";
	foreach( $docarr as $key=>$val) {
		echo "<li>".htmlspecialchars($val)."</li>";
	}
	echo "</ul>";
	reset ($docarr); 
}

function form2( &$sql, $parx) {
	global $error;
	
	$docarr = $this->getFilesFromTemp( );
	if ( $error->Got(READONLY) ) {
		$error->set('form2()', 1, "Error during analysing document files.");
		return;
	}
	
	$projLib = new oProjAddElem( $sql, $this->projid );
	if ($error->got(READONLY)) {
		$error->set($FUNCNAME, 7, "Init of project ".$this->projid." failed.");
    	return;
    }
	
	
	// create docs now
	echo "<br>Documents:<ul>";
	foreach( $docarr as $key=>$docname) {
		echo "<li>".htmlspecialchars($docname);
		
		$argu = NULL;
		$argu["NAME"] = $docname;
		$tmpid = insert_row($sql, "LINK", $argu );
		if ( !$tmpid ) {
			$error->set('form2()', 2, "Error during SQL-creation of document.");
			return;
		}
		
		$tempFileName = $this->sessdir."/".$docname;
		$linkUpObj = new oLinkUpload();
		$linkUpObj->link_cp_file( $sql, $tempFileName, $tmpid, $docname, "" );
		if ($error->Got(READONLY)) {
			$error->set('form2()', 3, "Connecting document attachment failed. ");
			return;
		}
		
		$projLib->addObj( $sql,  "LINK", $tmpid);
		echo "</li>";
		
	}
	echo "</ul>";
	reset ($docarr); 
}

}


// --------------------------------------------------- 
global $error, $varcol;

$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] );
// $sql2  = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot();

$go = $_REQUEST["go"];
$parx= $_REQUEST["parx"];

$tablename="LINK";

$title       = "Upload a set of documents (ZIP)";

#$infoarr['help_url'] = 'o.EXAMPLE.htm';
$infoarr=array();
$infoarr["title"] = $title;
$infoarr["form_type"]= "list";
$infoarr["obj_name"] = $tablename; 


$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);

echo "<ul>";
$creaLinkObj = new oLINKlistcrea($sql, $go);
if ($error->printLast()) htmlFoot();


echo "Destination project: <img src=\"images/icon.PROJ.gif\"> <B><a href=\"edit.tmpl.php?t=PROJ&id=".$creaLinkObj->projid."\">";
$projname = obj_nice_name( $sql, "PROJ", $creaLinkObj->projid );
echo $projname;
echo "</a></B><br>";



if ( !$go ) {
	$creaLinkObj->form0( $sql, $parx);
	 $error->printAll();
	htmlFoot("<hr>");
}

if ( $go==1 ) {
	
	$userfile  = $_FILES['userfile']['tmp_name'];
	$userfile_name = $_FILES['userfile']['name'];
	
	$creaLinkObj->form1($sql, $parx, $userfile, $userfile_name);
	 $error->printAll();
	htmlFoot("<hr>");
}

if ( $go==2 ) {
	$creaLinkObj->form2($sql, $parx);
	$error->printAll();
	htmlFoot("<hr>");
}







