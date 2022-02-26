<?php
/**
 * paste ASSOC table elements to destination object
 * @package glob.obj.assocpaste.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   $tablename  ($object )
  		 [$id]       (dest_id )
		 $parx["mode"]    "single" | "list"
         $parx["assoc_name"] (ASSOC_table )
		 $parx["assocclip"] take elems from clipboard
		 $parx["src_id"]
		 $go
		 from clipboard: object
 * @version0 2002-02-07
 */
session_start(); 


require_once ('reqnormal.inc');
require_once('insert.inc');
require_once('func_form.inc');
require_once('subs/glob.obj.assocpaste.inc');

function this_rowOut($tablename, $cntObj, $infoDest, $error_txt, $errorflag) {
	if ( !$errorflag ) $errorflag=0;
	switch ( $errorflag) {
		case 0: $error_txt = "<font color=green>OK</font>"; break;
		case 1: $error_txt = "<B><font color=#808000>Info:</font></B> ". $error_txt; break;
		case 2: $error_txt = "<B><font color=red>Error:</font></B> ". $error_txt; break;
	}

	echo "<tr bgcolor=#EFEFEF><td>".($cntObj+1).". </td><td>".$infoDest["name"].
		" [<a href=\"edit.tmpl.php?t=$tablename&id=".$infoDest["id"]."\">".$infoDest["id"]."</a>]</td>";
	echo "<td>".$infoDest["steps"]."</td>";
	echo "<td>".$infoDest["insertCnt"]."</td>";
	echo "<td>".$error_txt."</td></tr>\n";  
}

$error = & ErrorHandler::get();
$sql  = logon2( $_SERVER['PHP_SELF'] );
$sql2 = logon2( $_SERVER['PHP_SELF'] );
$sql3 = logon2( $_SERVER['PHP_SELF'] );

$id = $_REQUEST["id"];
$tablename=$_REQUEST['tablename'];
$go=$_REQUEST['go'];
$parx=$_REQUEST['parx'];

$title      = 'Insert associated elements from clipboard';

$infoarr = NULL;

$infoarr["title"] = $title;
$infoarr["form_type"]= "obj";

$nice_name  = tablename_nice2($tablename);

if ($parx["mode"]=="") $parx["mode"] = "single";

if ($parx["mode"]=="single") {
	//$headInf1 = 'edit.tmpl.php?t='.$tablename.'&id='.$id;
	$infoarr["obj_id"]   = $id;
	$infoarr["show_name"]= 1;
} else {
	//$headInf1 = 'view.tmpl.php?t='.$tablename;
	$infoarr["obj_cnt"]  = 1;  
	gHtmlMisc::func_hist("glob.obj.assocpaste", $title, $_SERVER['PHP_SELF']."?tablename=".$tablename."&parx[mode]=list" );
}

$infoarr["obj_name"] = $tablename;

$pagelib = new gHtmlHead();
$headarr = $pagelib->startPage($sql, $infoarr);
echo '<ul>';

if ($tablename=='') htmlFoot('ERROR', 'No name for object table given!');

$asocObj = new assocPasteC();

$assocTabs = get_assoc_tables2( $sql, $tablename );

$t_rights = tableAccessCheck( $sql, $tablename );
if ( $t_rights['insert'] != 1 ) {
	tableAccessMsg( $nice_name, 'insert' );
	return;
}

if ( !$go ) {
	list($parxt, $go) = $asocObj->check1($tablename, $assocTabs);
	$parx  = array_merge( $parx, $parxt );
} 

$src_id 	= $parx["src_id"];
$assoc_name = $parx["assoc_name"];
$delflag	= $parx["delflag"];

$nice_assoc = tablename_nice2($assoc_name);

if ( empty($src_id) ) htmlFoot('ERROR', 'Source object not defined');

$srcNice  = obj_nice_name($sql, $tablename, $src_id);
$tmpstr1=array();
$tmpstr2=array();

$tmpstr1[0] = "<B>" .$srcNice. "</B> [ID:".$src_id."]\n";
$idname   = PrimNameGet2($tablename);

// single objext !!!!
if ($parx["mode"]=="single") {

	if ( empty($id) )     htmlFoot('ERROR', 'Destination object not defined');
	$destNice   = obj_nice_name($sql, $tablename, $id);
	$tmpstr2[0] = "<B>" .$destNice."</B> [ID:".$id.    "]\n";
	$o_rights = access_check($sql, $tablename, $id);
	if (!$o_rights['insert']) htmlFoot('ERROR', 'No insert permission to this object.');
	if ( $src_id==$id ) {
		htmlFoot('ERROR', 'Source and destination object must be different');
	}
	
} else {
    $sqlopt=array();
	$sqlopt["order"] = 1;
	$sqlAfter  = get_selection_as_sql( $tablename, $sqlopt);
	
	$stopReason = "";
	$tmp_info   = $_SESSION['s_tabSearchCond'][$tablename]["info"];
	if ($tmp_info=="") $stopReason = "No elements selected.";
	if ($headarr["obj_cnt"] <= 0) $stopReason = "No elements selected.";
	if ($stopReason!="") {
		htmlFoot("Attention", $stopReason." Please select elements of '".$nice_name."'!");
	}
}

if ( !$go ) {
	$asocObj->form($sql, $tablename, $id, $tmpstr1, $tmpstr2, $go, $parx);
	htmlFoot();
}

if ( empty($assoc_name) )  htmlFoot('ERROR', 'Associated tyble type not given');
echo 'Copy elements of <B>&quot;',$nice_assoc,'&quot;</B><br>';


$asocObj->assoc_name = $assoc_name;
$asocObj->idname     = $idname;
$asocObj->src_id     = $src_id;
$asocObj->tablename  = $tablename;

$infox=array();

$infox["srcNum_steps"] = $asocObj->srcObjInfo($sql, $parx);

if ($parx["mode"]=="single") {
	$infoDest = $asocObj->destObjInfo($sql, $id);
	$infox["destNum_steps"]=$infoDest["steps"];
}

$tmpstr1[1] = " <B>".$infox["srcNum_steps"]."</B> elements";
$tmpstr2[1] = " <B>".$infox["destNum_steps"]."</B> elements";


if ($parx["mode"]=="single") {
	if ($go<2) {
		$xopt = NULL;
		if ( $infox["destNum_steps"]>0 ) $xopt["showDelflag"] = 1;

		$asocObj->form($sql, $tablename, $id, $tmpstr1, $tmpstr2, $go, $parx, $xopt);
		htmlFoot();
	}
	
	$cntelem = $asocObj->oneObjInsert( $sql, $sql2, $parx, $infox, $id);
	if ($error->Got(READONLY))  {
		$error->printAll();
	}
	echo "Ready: <font color=green>Inserted <B>".$cntelem."</B> elements.</font><br>";
	
} else  { // LIST !!!

	$xopt = NULL;
	$xopt["showDelflag"] = 1;
	
	if ($go<2) {
		$asocObj->form($sql, $tablename, $id, $tmpstr1, $tmpstr2, $go, $parx, $xopt);
	}

	echo "<table cellpadding=1 cellspacing=1 border=0>";
	echo "<tr bgcolor=#D0D0D0><td>&nbsp;</td><td>NAME</td><td>has elements</td><td>Inserted</td><td>Info</td></tr>\n";   
	
	$cntObj = 0;
	$sqlsLoop = "SELECT x.".$idname." FROM ".$sqlAfter;
	$sql3->query($sqlsLoop);
	while ( $sql3->ReadRow() ) {
		$error_txt = "";
		$erroris   = 0;
		$infoDest  = "";
		$thisDestId = $sql3->RowData[0];
		
		do {
			$infoDest   = $asocObj->destObjInfo($sql, $thisDestId);
			if ($error->Got(READONLY))  {
				$errLast = $error->getLast();
				$error_txt = $errLast->text;
				$error->reset();
				$erroris=2;
				break;
			}
			if (!$infoDest["access"]) {
				$error_txt = "no access";
				$erroris=1;
				break;
			}
			
			if ($go==2) {
				$cntelem = $asocObj->oneObjInsert( $sql, $sql2, $parx, $infox, $thisDestId);
				$infoDest["insertCnt"] = $cntelem;
				if ($error->Got(READONLY))  {
					$errLast = $error->getLast();
					$error_txt = $errLast->text;
					$errLast = $error->getLast();
					$error_txt = $error_txt." ".$errLast->text;
					$error->reset();
					$erroris=2;
				}
			}
		} while (0); 
		this_rowOut($tablename, $cntObj, $infoDest, $error_txt, $erroris);
		$cntObj++;
	}
	echo "</table>";

}

htmlFoot();
