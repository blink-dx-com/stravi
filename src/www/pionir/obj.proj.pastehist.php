<?php
/**
 * Paste objects from history
 * @package obj.proj.pastehist.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   $id   (PROJ_ID)
 */
session_start(); 


require_once ("reqnormal.inc");
require_once ("class.history.inc");
require_once ("o.PROJ.addelems.inc");
require_once ( "javascript.inc" );

// ---------------------------------------------------

$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] );
//$sql2  = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot();

$id 				 = $_REQUEST["id"];


$title       = 'Paste objects from history to project';
$infoarr=array();
$infoarr["title"] = $title;
$infoarr["form_type"]= "obj";
$infoarr["obj_name"] = "PROJ";
$infoarr["obj_id"]   = $id;
$infoarr["checkid"]  = 1;

$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);
echo "<ul>\n";
$accopt = array( 'tab'=>array('write'),  'tab'=>array('read', 'insert') );
$pagelib->do_objAccChk($sql, $accopt);
$pagelib->chkErrStop();

$found=0;
$hist_obj   = new historyc();
$projAddLib = new oProjAddElem($sql, $id);

$retarr = $hist_obj->getObjects();

if (sizeof ($retarr))  {

	foreach( $retarr as $pos=>$tmparr) {
		$tmp_tablename = $tmparr[0];
		$tmp_id		   = $tmparr[1];
		$retval = $projAddLib->addObj( $sql, $tmp_tablename, $tmp_id );
		$found++;
	}
}

if (!$found) {
	echo ("No object in history!");
	htmlFoot();
} else {
	echo "<B>$found</B> objects copied to project<br>\n";
}


$url ='edit.tmpl.php?t=PROJ&id='.$id;
js__location_replace($url, 'back' );

htmlFoot();
