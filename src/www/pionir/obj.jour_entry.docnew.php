<?php
/**
 * Create new docBook
 * @package obj.jour_entry.docnew.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   $go
 *   array $parx
 */
session_start(); 

require_once ("reqnormal.inc"); // includes all normal *.inc files
require_once ("subs/obj.jour_entry.ed.inc");
require_once ("insertx.inc");
require_once ("o.PROJ.addelems.inc");
require_once ("f.objview.inc");	

class oLINKDocbookCreaC {

function __construct(&$sqlo, $parx) {
	global $error;
	$FUNCNAME= "oLINKDocbookCreaC";
	
	$this->docbookNice = "LabBook";
	$this->parx = $parx;
	$action  = "";
	$iddummy = NULL;
	$action  = NULL;
	$this->labLib = new oEXPlabjour1($iddummy, $action);
	
	if ( !$this->labLib->bookclassid ) {
		$error->set( $FUNCNAME, 1, "No Class for 'LabBook' found." );
		return;
	}
	$this->labLib->getFromProjProfile( $sqlo );
	if ( !$this->labLib->projBookId ) {
		$error->set( $FUNCNAME, 2, "No project for ".$this->docbookNice." found." );
		return;
	}
	
	$this->bookclassid = $this->labLib->bookclassid;
	$this->projBookId = $this->labLib->projBookId;
	
	$o_rights = access_check($sqlo, "PROJ", $this->projBookId);
	if ( !$o_rights["write"]) htmlFoot("ERROR", "You do not have write permission on project ".$this->projBookId."!");

}

function form(&$sql) {
	require_once ('func_form.inc');
	
	
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Give LabBook-name";
	$initarr["submittitle"] = "Create";
	$initarr["tabwidth"]    = "AUTO";

	$hiddenarr = NULL;

	$formobj = new formc($initarr, $hiddenarr, 0);

	$fieldx = array ( 
		"title" => "Name of LabBook", 
		"name"  => "name",
		"object"=> "text",
		"val"   => $parx["name"], 
		"notes" => "a name for the LabBook"
		 );
	$formobj->fieldOut( $fieldx );

	$formobj->close( TRUE );
}

function checkName(&$sqlo) {
	global $error;
	$FUNCNAME= "checkName";
	
	if ($this->parx["name"]=="") {
		$error->set( $FUNCNAME, 1, "Please give a name" );
		return;
	}
	
	echo "create a new ".$this->docbookNice.".<br>";
	
	$sqls = "select LINK_ID from LINK x  JOIN EXTRA_OBJ o ON x.EXTRA_OBJ_ID=o.EXTRA_OBJ_ID ".
			"where x.NAME='".$this->parx["name"]."' AND o.EXTRA_CLASS_ID=".$this->bookclassid;
	$sqlo->query($sqls);
	$sqlo->ReadRow();
	$existID = $sqlo->RowData[0];
	if ($existID) {
		$error->set( $FUNCNAME, 3, $this->docbookNice." '".$this->parx["name"]."' already exists." );
		return;
	}
}

function doit(&$sqlo) {
	global $error;
	$FUNCNAME= "doit";
	
	$creLib = new insertC();
	$tablename="LINK";
	$args  = array( "vals"=>array("NAME"=>$this->parx["name"]),
			        "xobj"=>array("extra_class_id"=>$this->bookclassid) );
	$objid = $creLib->new_meta($sqlo, $tablename, $args );
	
	if ($error->Got(READONLY))  {
		$error->set( $FUNCNAME, 1, "object creation failed." );
		return;
	}
	echo "".$this->docbookNice." ".fObjViewC::bo_display( $sqlo, $tablename, $objid )."   created.<br>";
	
	
	$projLib = new oProjAddElem( $sqlo, $this->projBookId );
	if ($error->got(READONLY)) {
		$error->set($FUNCNAME, 7, "Copy to project ".$this->projBookId." failed.");
    	return;
    }
	$projLib->addObj( $sqlo,  $tablename, $objid); 
}

}
// --------------------------------------------------- 
global $error, $varcol;

$error = & ErrorHandler::get();
$sqlo  = logon2( $_SERVER['PHP_SELF'] ); // URL-link for the first db-login; e.g. for object ($_SERVER['PHP_SELF']."?id=".$id)
if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();

$go = $_REQUEST["go"];
$parx = $_REQUEST["parx"];

$tablename			 = "LINK";

$title       		 = "Create a new LabBook for LabJournal-entries";
$infoarr			 = NULL;

$infoarr["title"] = $title;
$infoarr["form_type"]= "tool";
$infoarr["scriptID"] = "";
$infoarr["locrow"]   = array(array("obj.jour_entry.list.php", "LabJournal Report"));

$pagelib = new gHtmlHead();
$pagelib->startPage($sqlo, $infoarr);
echo "<ul>";
$mainLib = new oLINKDocbookCreaC($sqlo, $parx);
if ($error->Got(READONLY))  {
	$error->printAll();
	htmlFoot();
}

if ( !$go ) {
	$mainLib->form($sqlo);
	htmlFoot();
}

$mainLib->checkName($sqlo);
if ($error->Got(READONLY))  {
	$error->printAll();
	echo "<br>";
	$mainLib->form($sqlo);
	htmlFoot();
}


$mainLib->doit($sqlo);
if ($error->Got(READONLY))  {
	$error->printAll();
	echo "<br>";
	htmlFoot();
}
htmlFoot("<br><hr>");


