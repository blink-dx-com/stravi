<?php
/**
 *  download a document
 * @package obj.link.download.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   $id
          [$mime] show/download with special mime-type (overwrite mime of the document)
		  [$inline] show inline as text
		  [$pos]  - from version control
 */ 
session_start(); 

require_once ("reqnormal.inc"); // includes all normal *.inc files
require_once ("down_up_load.inc");
require_once ("o.LINK.subs.inc");
require_once("o.LINK.versctrl.inc");

function this_error( &$sql, $id, $errortext ) {

	$tablename ="LINK";

	$title       = "Download a document";

	$infoarr=array();
    $infoarr["title"] = $title;
    $infoarr["form_type"]= "obj";
	$infoarr["obj_name"] = $tablename;
	$infoarr["obj_id"]   = $id;
	$infoarr["show_name"]= 1;

	$pagelib = new gHtmlHead();
	$pagelib->startPage($sql, $infoarr);
	
	htmlFoot("Error", $errortext);
	
}

global $error, $varcol;

$id = $_REQUEST["id"];
$mime= $_REQUEST["mime"];
$inline= $_REQUEST["inline"];
$pos= $_REQUEST["pos"];

$error = &ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF']."?id=".$id );
if ($error->printLast()) htmlFoot();
$infoplus = "";

if (!$id)  this_error( $sql, $id, "No document-ID given." );
$doc_path = linkpath_get( $id );

if ( $pos ) {
	$versionObj = new oLINKversC();
	$versionObj->isUnderControl($sql, $id);
	$doc_path   = $versionObj->getVersPathEasy($id, $pos);
	$infoplus = " (version:pos:$pos) ";
}

if ( !file_exists($doc_path) )  this_error( $sql, $id, "Document-file ".$infoplus." not found on server." );

$img_rights = access_check($sql, 'IMG', $id);

if ( $img_rights["read"]==0) {
	this_error( $sql, $id, "No access-read-right for this document ".$infoplus );
}

$filesizex = filesize($doc_path);

$sqls = "SELECT name, mime_type FROM link WHERE link_id=".$id;
$sql->query($sqls);
$sql->ReadRow();
$docname    = $sql->RowData[0];
$mimetypeDb = $sql->RowData[1];

$docname = str_replace("\\", "/", $docname); // convert back-slashes
$docname = basename($docname); // remove slashes ...
$mime_type  = $mimetypeDb;
if ($mime!="") $mime_type = $mime; // overwrite mime-type from document

if ($mime_type=="")  {
	// guess mime_type
    $pointpos = strrpos($docname, ".");
	if ($pointpos>0) {
	    $extension = substr($docname,$pointpos+1);
		$mime_type = "doc/".$extension;
	}
}
if ($docname=="") $docname=$id;
$mimeopt=array("size"=>$filesizex);
if ($inline) $docname="";
set_mime_type ( $mime_type, $docname, $mimeopt );

readfile( $doc_path ,"r");