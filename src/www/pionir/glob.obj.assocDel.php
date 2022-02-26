<?php
/**
 * delete all elements ASSOCIATED TABLE ($asoctab) belonging to mother ($tablename) with ID=$id
 * @package glob.obj.assocDel.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param  $id 		// ID of $mtable
		 $tablename	// mother object
		 $asoctab   // associated table
		 $go	0 - wait
		 		1 - yes
				2 - no
 * @version0 2007-12-13
 */
session_start(); 


require_once ("reqnormal.inc"); // includes all normal *.inc files
require_once ("visufuncs.inc");
require_once ("f.obj.assocdel.inc");



// --------------------------------------------------- 
global $error;

$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] ); // URL-link for the first db-login; e.g. for object ($_SERVER['PHP_SELF']."?id=".$id)
//$sql2  = logon2( );
if ($error->printLast()) htmlFoot();


$id = $_REQUEST["id"];
$tablename=$_REQUEST['tablename'];
$go=$_REQUEST['go'];
$asoctab=$_REQUEST['asoctab'];

$tableNiceName 	 = tablename_nice2($tablename);
$assocNice = tablename_nice2($asoctab);

$backurl  = "edit.tmpl.php?t=".$tablename."&id=".$id;
$title       		 = "Delete all elements of an associated table belonging to this mother-object";
$infoarr			 = NULL;

$infoarr["title"] = $title;
$infoarr["form_type"]= "obj";
#$infoarr['help_url'] = "o.EXAMPLE.htm";
$infoarr["obj_name"] = $tablename;
$infoarr["obj_id"]   = $id;
$infoarr["show_name"]= 1;

$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);
echo "<ul>";

if ( !$id ) htmlFoot("ERROR", "Please give the ID of the object");
if ($tablename=="")  htmlFoot("ERROR", "Need the name of mother-TABLE");
if ($asoctab=="")  htmlFoot("ERROR", "Need the name of ASSOCIATED TABLE");

// 
//   check   R I G H T S
//

$scriptLib = new gObjAssocDelC($tablename, $id, $asoctab);
$scriptLib->checkRights($sql);
if (  $error->printAll() ) {
	htmlFoot();
}
$elemnum = $scriptLib->getElemNum($sql);


$iconMoth = htmlObjIcon($tablename, 0);
$icon = htmlObjIcon($asoctab, 0);
$motherName = obj_nice_name( $sql, $tablename, $id);
echo "<font color=gray><b>Delete all elements of  <img src=\"".$icon."\"> <font color=#336699>". $assocNice.
	"</font> belonging to <img src=\"".$iconMoth."\"> <font color=#336699>".$tableNiceName . 
	"</font> '<b>".htmlspecialchars($motherName)."</b>'.</font></b>";
echo "<br><br>";

if (!$elemnum) {
	htmlFoot("No elements found for mother-object.");
}

if (!$go) {
	$scriptLib->go0( );
	htmlFoot("<hr>");
}
if ( $go == 1 ) {
	$scriptLib->go1($sql);
}

// go back to mother-object ...
js__location_href( $backurl );
return;

htmlFoot("<hr>");

