<?php
/**
 * add selected OBJECTS ($tablename) to an ASSOCIATED TABLE ($asoctab) belonging to mother ($mtable) with ID=$id
 * @package glob.obj.assocadd.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param $id 		// ID of $mtable
		 $mtable	// table name of destination object
		 $asoctab   // associated table
		 $asoccol   // column of running number
		 $tablename	// tablename of selected Assoc BOs
		 selorder  // e.g. x.EXP_ID ; OPTIONAL: order criteria of selected Assoc BOs
		 [$go]    ==   [0] | "1"
 */

// extract($_REQUEST); 
session_start(); 

	
require_once ('reqnormal.inc');
require_once ('func_form.inc');
require_once ('f.msgboxes.inc');
require_once ("f.objview.inc");	
require_once ("lev1/glob.obj.assocadd.inc");	


function this_info ($key, $text, $notes=NULL ) {
    // FUNCTION: print out info text
    if ($notes!="")  $notes = " &nbsp;<I>".$notes."</I>";
    echo "<font color=gray>".$key.":</font> ".$text."".$notes."<br>\n";

}


global $error;
$error = & ErrorHandler::get();
$sql  = logon2();
$sql2 = logon2();


$id       = $_REQUEST['id'];		
$mtable	  = $_REQUEST['mtable'];		
$asoctab  = $_REQUEST['asoctab'];	
$asoccol  = $_REQUEST['asoccol'];	
$tablename= $_REQUEST['tablename'];	
$selorder = $_REQUEST['selorder'];	
$go      = $_REQUEST['go'];	

$i_tableNiceName = tablename_nice2($mtable);
$i_selTableNice  = tablename_nice2($tablename);

$title = "Map selection of ".$i_selTableNice." on this ".$i_tableNiceName;

$MAXATTACH = 2000; // current max elements

$infoarr=NULL;
$infoarr["obj_name"] = $mtable;
$infoarr["title"]    = $title;
$infoarr["title_sh"] = 'Map children';
$infoarr["form_type"]= "obj";
$infoarr["obj_id"]   = $id;
$infoarr["show_name"]= 1; 
$infoarr['design']   = 'norm';

$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);
$accCheckArr = array('tab'=>array('write'), 'obj'=>array('insert') );
$pagelib->do_objAccChk($sql, $accCheckArr);
$pagelib->chkErrStop();

 
if ( !$id ) {
	htmlFoot("Error", 'ID of mother not defined ! <BR> Please select a mother object first!');
}
if ( $mtable=="" ) {
	htmlFoot("Error", 'Name of mother object type not given!');
}
if ( $tablename=="" ) {
	htmlFoot("Error", 'Name of selected BusinessObject-Type not given!');
}
if ( $asoctab=="" or $asoccol=="") {
	htmlFoot("Error", 'Name of "asocciated table" or "column name" of running number not given!');
}

$mainLib = new gObj_assocAdd($sql, $mtable, $id, $asoctab, $asoccol, $tablename);
$pagelib->chkErrStop();

$infox = $mainLib->getInfox();




// ASSOC TABLE ...  
$sqlopt=NULL;
if ($selorder=="" ) $sqlopt["order"] = 1;
else {
	$orderExt = query_sort_get( $tablename, $selorder );
}
$sqlAfterNoOrder = get_selection_as_sql( $tablename, $sqlopt );

$sqlAfter = $sqlAfterNoOrder;
if ($selorder!="" ) {
	$sqlAfter .= $orderExt;
}


$sqls = "select count(1) from ".$tablename; /* get number of rows */
$sql->query($sqls);
$sql->ReadRow();
$probe_cnt = $sql->RowData[0];

$sqls = 'select count(1) from '. $sqlAfterNoOrder; /* get number of rows */

$sql->query($sqls);
$sql->ReadRow();
$probe_sel_cnt = $sql->RowData[0];

$objLinkLib = new fObjViewC();
$childTableSelHtml = $objLinkLib->tableViewLink($tablename);

if ( $probe_sel_cnt >= $probe_cnt ) {
	echo "<B>ERROR</B>: No ".$infox["selbo_tabNiceName"]." selected. Please select them from list: ".
		'<b>'.$childTableSelHtml."</b><br>";
	return 0;
}

if ($probe_sel_cnt>$MAXATTACH) {
	$pagelib->htmlFoot('ERROR','Max '.$MAXATTACH.' elements allowed. Please ask your admin.');
}


if ( !$go ) {
	//
} else {
	echo "<b>Add Selected ".$infox["selbo_tabNiceName"]."(s) NOW! </b><br><br>";
}

$icon1 = htmlObjIcon($tablename);
this_info( "map objects",  "<font size=+1><b>".$probe_sel_cnt.
	"</b></font> <font color=gray>objects of</font> <img src=\"$icon1\"> ".$infox["selbo_tabNiceName"]);
this_info( "select other objects", '==&gt; '.$childTableSelHtml );
this_info( "Running number",$asoccol);
this_info( "Sort criteria",$selorder, "(of linked objects)");

echo "<br>";

if ( !$go  ) {
	
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Map the following objects";
	$initarr["submittitle"] = "Map now!";
	$initarr["tabwidth"]    = "AUTO";

	$hiddenarr = NULL;
	$hiddenarr["tablename"]= $tablename;
	$hiddenarr["id"]       = $id;
	$hiddenarr["mtable"]   = $mtable;
	$hiddenarr["asoctab"]  = $asoctab;
	$hiddenarr["asoccol"]  = $asoccol;
	$hiddenarr["selorder"]  = $selorder;
	
	
	$formobj = new formc($initarr, $hiddenarr, 0);

	$formobj->close( TRUE );
	echo "<br>";
	
} 
 	
echo "<font color=gray>Selected ".$infox["selbo_tabNiceName"]."(s): </font><br><br>";

$add_result = $mainLib->add_loop($sql, $sql2, $sqlAfter, $go );
$pagelib->chkErrStop();

 
echo "<hr>";
echo "Sum: <B>".$add_result['cnt']."</B> objects analysed.";
if ($add_result['existSum']) echo " <B>".$add_result['existSum']."</B> object-mappings already exist!";
echo "<br><br>";

if ( $go>0 ) {
    cMsgbox::showBox("ok", "<B>".$add_result['inserted']."</B> objects mapped.");  // 
}

echo "<hr>";	

htmlFoot();

