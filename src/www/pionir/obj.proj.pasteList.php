<?php
/**
 * copy selected objects in list to project 
 * $Header: trunk/src/www/pionir/obj.proj.pasteList.php 59 2018-11-21 09:04:09Z $
 * @package obj.proj.pasteList.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param string $id (project-ID) 
 * @param string $tablename (tablename)
 * @param string $go
 */
session_start(); 


require_once ("reqnormal.inc");
require_once ("o.PROJ.addelems.inc");
require_once ( "javascript.inc" );
require_once ('glob.objtab.page.inc');
require_once ("f.visu_list.inc");
require_once ("f.flushOutput.inc");
require_once ('func_form.inc');
require_once ("f.sql_query.inc");	

class oPROJ_PasteList {
function __construct($projid, $tablename, $go) {
	$this->projid = $projid;
	$this->table  = $tablename;
	$this->go = $go;
	$this->sqlAfter = '';

	if ($tablename!='') { 
		$sqlQuerLib = new fSqlQueryC($tablename);
		$sqlopt["order"] = 1;
		$this->sqlAfter = $sqlQuerLib->get_sql_after($sqlopt); 
	}
}


function form1(&$sqlo) {
	
	$visuLib = new visu_listC();
	$formurl = $_SERVER['PHP_SELF'].'?id='.$this->projid;
	$visuLib->selTable( $sqlo, $formurl);
}


function form2(&$sqlo, $elemnum) {
	
	
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Prepare copy action";
	$initarr["submittitle"] = "Copy";
	$initarr["tabwidth"]    = "AUTO";

	$hiddenarr = NULL;
	$hiddenarr["tablename"]     = $this->table;
	$hiddenarr["id"]     = $this->projid;

	$formobj = new formc($initarr, $hiddenarr, 1);
	$fieldx = array ( 
		"title" => 'Copy <b>'.$elemnum.'</b> elements to project ?', 
		"object"=> "info"
		 );
	$formobj->fieldOut( $fieldx );
	$formobj->close( TRUE );
}

function copyElements(&$sqlo, &$sqlo2 ) {
	global $error;
	$FUNCNAME= 'copyElements';

	$tablesel = $this->table;
	$this->found    = 0;
	$projAddLib = new oProjAddElem($sqlo, $this->projid );

	$flushLib = new fFlushOutputC( 500, '.' );

	$primary_key = PrimNameGet2($tablesel);
	$sqlsel =  'x.'.$primary_key.' FROM '.$this->sqlAfter;
	$sqlo2->Quesel($sqlsel);
	while ( $sqlo2->ReadRow() ) {
		$objid  = $sqlo2->RowData[0];
		$retval = $projAddLib->addObj( $sqlo, $tablesel, $objid );
		if ($error->Got(READONLY))  {
     		$error->set( $FUNCNAME, 1, 'Problem at addObj('.$tmp_id.')' );
			return;
		}
		$flushLib->alivePoint();
		$this->found++;
	}
	$this->realInsert = $projAddLib->getNumInsert();
}

}
// --------------------------------------------------- 
global $error, $varcol;

$error = & ErrorHandler::get();
$sqlo  = logon2( $_SERVER['PHP_SELF'] ); 
$sqlo2 = logon2(  ); 
if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();

$id 				 = $_REQUEST["id"];
$tablesel		 	 = $_REQUEST["tablename"];
$go		 			 = $_REQUEST["go"];
$tablename			 = "PROJ";

$title				 = "Copy selected objects to project";
$infoarr			 = NULL;
$infoarr["scriptID"] = "";
$infoarr["title"]    = $title;
$infoarr["form_type"]= "obj"; // "tool", "list"
$infoarr["obj_name"] = $tablename;
$infoarr["obj_id"]   = $_REQUEST["id"];
$infoarr["checkid"]  = 1;

$pagelib = new gHtmlHead();

if ( $tablesel != '' ) {
	// get number of elements
	$tableselNice = tablename_nice2($tablesel);
	$listLib = new  visu_listC();
	$cheopt = array("doCount"=>1);
  	$answer = $listLib->checkSelection($sqlo, $tablesel, $cheopt);
	$elemcnt = $answer[2];
	$icon = htmlObjIcon($tablesel, 1);
	$infoarr["inforow"] = '<img src="'.$icon.'"> '.$tableselNice.': <b>'.$elemcnt.'</b> elements selected';
}
$headarr = $pagelib->startPage($sqlo, $infoarr);
echo '[<a href="'.$_SERVER['PHP_SELF'].'?id='.$id.'">Select other table</a>]<br>'."\n";
echo "<ul>\n";
$accopt = array( 'tab'=>array('write'),  'tab'=>array('read', 'insert') );
$pagelib->do_objAccChk($sqlo, $accopt);
$pagelib->chkErrStop();

$mainlib = new oPROJ_PasteList($id, $tablesel, $go);

if ( $tablesel == '' ) {
	$mainlib->form1($sqlo);
	htmlFoot('<hr>');
}

// table selected
if ( !$elemcnt ) {
	htmlFoot('Error', 'No element in table <b>'.$tableselNice.'</b> selected.');
}

if ( $go<2 ) {
	$mainlib->form2($sqlo,$elemcnt );
	htmlFoot('<hr>');
}

$mainlib->copyElements($sqlo, $sqlo2 );
$found = $mainlib->found;
$pagelib->chkErrStop();

if (!$found) {
	echo ("No object copied!");
	htmlFoot('<hr>');
} else {
	echo "<B>$found</B> objects copied to project<br>\n";
	if ( $mainlib->realInsert != $found) echo "<B>". $mainlib->realInsert."</B> objects were new in project<br>\n";
}


$url ='edit.tmpl.php?t=PROJ&id='.$id;
js__location_replace($url, 'back' );

htmlFoot('<hr>');
