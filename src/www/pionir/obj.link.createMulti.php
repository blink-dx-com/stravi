<?php
/**
 * create multiple file + upload
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @package obj.link.createMulti.php
 * @param $projid 
  	$go : 0,1
  	$_FILES['fx'][] - array of files
   * @version $Header: trunk/src/www/pionir/obj.link.createMulti.php 59 2018-11-21 09:04:09Z $
 */
session_start(); 


require_once ('reqnormal.inc');
require_once ('insert.inc');
require_once ('o.LINK.subs.inc');
require_once ('o.PROJ.addelems.inc');


class oLINK_creaMultiGui {

function __construct(&$sqlo, $projid) {
	global $error;
	$this->projid = $projid;
	$this->projLib = new oProjAddElem( $sqlo, $projid );
}

function formshow($parx, $go, $max_file) {

	require_once ('func_form.inc');
	
	
	$projid=$this->projid;
	
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Upload files";
	$initarr["submittitle"] = "Submit";
	$initarr["tabwidth"]    = "AUTO";
	$initarr["ENCTYPE"] 	= "multipart/form-data";
	
	$hiddenarr = NULL;
	$hiddenarr["projid"]     	= $projid;
	$hiddenarr["MAX_FILE_SIZE"] = $max_file;
	
	$formobj = new formc($initarr, $hiddenarr, 0);
	$nummax = 10;
	$i=0;
	while ( $i < $nummax ) {
	
		$fieldx = array ( "title" => "File ".($i+1), 
				"name"  => "fx[".$i."]",
				"object" => "file",
				"val"   => "",
				"fsize" => 50, 
				"inits" => "overview", 
				"namex" => 1,
				"notes" => "" );
		$formobj->fieldOut( $fieldx );
		$i++;
	}
	$formobj->close( TRUE );

}

function createDocu( &$sql, $tmpfile, $docName, $filetype) {
	global $error;
	
	$projid=$this->projid;
	$argu = NULL;
	$argu["NAME"] = $docName;
	
	$objid = insert_row($sql, "LINK", $argu);
	if ($objid<=0) {
		$error->set("this_createDocu", 1, "Creation of document-object failed");
		return;
	}
	
	$this->projLib->addObj( $sql, "LINK", $objid); 
	if ($error->Got(READONLY)) {
		$error->set("this_createDocu", 2, "Adding the new document-object to project failed");
		return;
	}
	$linkUpObj = new oLinkUpload();
	$linkUpObj->link_cp_file( $sql, $tmpfile, $objid, $docName, $filetype );

	if ( $error->Got(READONLY) ) {
		$error->set("this_createDocu", 3, "Upload or object info update failed.");
		return;
	}
}

}


// --------------------------------------------------- 
global $error, $varcol;

$error = & ErrorHandler::get();
$sql  = logon2(  );
if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();

$projid = $_REQUEST['projid'];
$go     = $_REQUEST['go'];

$title       = 'Upload a set of documents';

$infoarr["title"] = $title;
$infoarr["form_type"]= "obj";
$infoarr["obj_name"] = "PROJ";
$infoarr["obj_id"]   = $projid;
$infoarr["show_name"]= 1;
$infoarr['checkid']  = 1;

$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);
echo "<ul>";


$t_rights = tableAccessCheck($sql, "PROJ");
if (!$t_rights['insert']) {
  	tableAccessMsg("Project", 'insert');
  	htmlFoot();
}

$t_rights = tableAccessCheck($sql, "LINK");
if (!$t_rights['insert']) {
  	tableAccessMsg("Document", 'insert');
  	htmlFoot();
}

$mainLib = new oLINK_creaMultiGui($sql, $projid);
$pagelib->chkErrStop();

if ( $go ) {
	$cnt = 0;
	
	print_r($_FILES);
	echo "<br>";
	
	foreach( $_FILES['fx']['tmp_name'] as $fileid=>$val) {
	
		if ( $_FILES['fx']['tmp_name'][$fileid] == "") {
			continue;
		}
		
		echo "File-Name: <B>". $_FILES['fx']['name'][$fileid] . "</B> Size: ". $_FILES['fx']['size'][$fileid]." &nbsp;" ;
		$mainLib->createDocu( $sql, 
				$_FILES['fx']['tmp_name'][$fileid], 
				$_FILES['fx']['name'][$fileid], 
				$_FILES['fx']['type'][$fileid] 
		);
		if ($error->Got(READONLY))  {
			$error->printAllEasy();
			$error->reset();
		} else {
			$cnt++;
		}
		
		echo "<br>\n";
	}
	echo "<B>$cnt</B> files uploaded.<br><br>\n";
	

}

$max_file_size = $_SESSION['globals']["F.IMG.IMPORT.UPLOAD_MAX_SIZE"];
echo "Max-file-Size: <B>".($max_file_size*0.000001)."</B> Mbytes<br>\n";
$mainLib->formshow($parx, $go, $max_file_size);

htmlFoot();



