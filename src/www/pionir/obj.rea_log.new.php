<?php
/**
 * edit/create new REA_LOG entry
 - for parx[KEY]=Service : go to the edit-form, if ACCEPT>=2 (immediate fill out)
 * @package  obj.rea_log.new.php
 * @swreq UREQ:0000921: o.CHIP_READER > REA_LOG : service log abbilden 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param   $id   (CHIP_READER)
  		  $parx["NAME"]
		  $parx["KEY"]
		  $parx["NOTES"]
	$action : 
		['insert'] - do the insert
		'prep' - just show form with params without error-analysis
	@version $Header: trunk/src/www/pionir/obj.rea_log.new.php 59 2018-11-21 09:04:09Z $
 */

session_start(); 


require_once ("reqnormal.inc"); // includes all normal *.inc files
require_once ("javascript.inc" );
require_once ("o.REA_LOG.new.inc");
require_once ('subs/obj.rea_log.edform.inc');

class oREA_LOG_guiNew {

function __construct( &$sqlo, $id ) {
	$this->id = $id;
	$this->ReaLogNew = new oREA_LOG_new();
	$this->ReaLogNew->setDevice($id);
	
}

function paramCheck(&$sqlo,$parx) {
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	$answer = $this->ReaLogNew->paramCheck($sqlo, $parx);
	if ($answer!=NULL) {
		$error->set( $FUNCNAME, 2, $answer[1].' (Code:'.$answer[0].')' );
		return;
	}
}

function form1( &$sqlo, $parx ) {
	$id = $this->id;
	$A_DEVID = glob_elementDataGet( $sqlo, 'CHIP_READER', 'CHIP_READER_ID', $this->id, 'A_CHIP_READER_ID' );
	$formopt= array('A_DEVID'=>$A_DEVID);
	$dummy  = NULL;
	$EditLib   = new oChip_readerLogC ();
	$EditLib->edform( $sqlo, 'obj.rea_log.new.php?id='.$id, $parx, $dummy, $formopt );
}

function addEntry( &$sqlo, $parx ) {
	$newpos = $this->ReaLogNew->addEntry( $sqlo, $parx);
	return $newpos;
}

}

//-------------------------------------------

global $error, $varcol;

$tablename="REA_LOG";
$mothertab="CHIP_READER";
$i_tableNiceName=tablename_nice2($tablename);

$error = & ErrorHandler::get();
$sqlo  = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();

$action = $_REQUEST['action'];
$id     = $_REQUEST['id'];
$parx   = $_REQUEST['parx'];

if ($action==NULL) {
	$action='insert';
}

$title  = "New log enty";
$infoarr=array();
$infoarr["title"] = $title;
$infoarr["form_type"]= "obj";
$infoarr["obj_name"] = $mothertab;
$infoarr["obj_id"]   = $id;
$infoarr['design']   = 'norm';

$pagelib = new gHtmlHead();
$pagelib->startPage($sqlo, $infoarr);

if (!$id) htmlfoot("Error","ID of device missing");

$t_rights = tableAccessCheck( $sqlo, $mothertab );
if ( $t_rights["write"] != 1 ) {
	tableAccessMsg( $i_tableNiceName, "write" );
	htmlFoot();
}
$o_rights = access_check($sqlo, 'CHIP_READER', $id);
if ( !$o_rights["insert"]) {
	htmlFoot('ERROR',"You do not have insert permission on this device!");
}

$mainLib   = new oREA_LOG_guiNew($sqlo, $id);
$js_string = oChip_readerLogC::get_js();
echo '<script>'."\n";
echo $js_string;
echo '</script>'."\n";

if ($action=='prep') {
	// just show form params
	$mainLib->form1($sqlo, $parx);
	htmlFoot();
}

$mainLib->paramCheck($sqlo,$parx);

if ($error->Got(READONLY))  {
	$error->doLog( 0 ); // do not log these errors
	$error->printAll();
	$error->doLog( 1 );
	echo "<br />\n";
	$mainLib->form1($sqlo, $parx);
	htmlFoot();
}	

$newpos = $mainLib->addEntry($sqlo,$parx);
if ($error->Got(READONLY))  {
	$error->printAll();
	echo "<br />\n";
	$mainLib->form1($sqlo, $parx);
	htmlFoot();
}

$extraModule=$_SESSION['s_sessVars']['AppLabLibDir'].'/obj.rea_log.newxtra.inc';
if ( file_exists( $extraModule ) ) {
	require_once ( $extraModule );
	$extraLib = new oREA_LOG_newxtra();
	$extraLib->start($sqlo, $id, $parx, $newpos);
	$pagelib->chkErrStop();
}

if ($parx["KEY"]=='Service') {
	$newurl = "obj.chip_reader.logEdit.php?id=".$id."&pos=".$newpos."&backPage=1";
	js__location_replace($newurl, "next to advanced edit" );
	exit;
}

$newurl = "edit.tmpl.php?t=".$mothertab."&id=".$id;
js__location_replace($newurl, "back to device" );  

htmlFoot();

