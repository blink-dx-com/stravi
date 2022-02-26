<?php
/**
 * Show download links for selected documents
 * @package obj.link.list_dlnk.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   [$showinfo]
  		  [$proj_id] - alternative project-ID
 */ 
session_start(); 


require_once('db_access.inc');
require_once('globals.inc');
require_once('func_head.inc');
require_once('access_check.inc');
require_once('table_access.inc');
require_once('sql_query_dyn.inc');
require_once ("f.head_proj.inc");
require_once ("o.LINK.subs.inc");


$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] );
//$sql2  = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot();

$showinfo = $_REQUEST["showinfo"];
$proj_id = $_REQUEST["proj_id"];


$title = 'Show download links for selected documents';
$tablename = "LINK";
$infoarr=array();
$infoarr['back_url'] = 'view.tmpl.php?t='.$tablename;

$pageHeadListObj = new gHtmlHeadProjC();
$obj_arr=array();
$answerArr = $pageHeadListObj->showHead( $sql, $tablename, $proj_id, $obj_arr, $title, $infoarr);

$sqlAfter  = $answerArr["sqlAfter"];

echo "<ul>\n";

$t_rights = tableAccessCheck( $sql, $tablename );
if ( $t_rights['read'] != 1 ) {
	tableAccessMsg( "document", 'read' );
	return;
}


// echo "<font color=gray>Selected experiments: </font><font size=+1><b>$num_elem</b></font><br><br>\n";


$sqls = "select x.LINK_ID, x.NAME from ".$sqlAfter;
$sql->query("$sqls");
$sel_str="";
$cnt = 1;
$fixdown = "http://".$_SERVER['HTTP_HOST'].$_SESSION["s_sessVars"]["loginURL"]."/obj.link.download.php?id=";
$fixobj = "http://".$_SERVER['HTTP_HOST'].$_SESSION["s_sessVars"]["loginURL"]."/edit.tmpl.php?t=LINK&id=";
$grafurl_arr= "http://".$_SERVER['HTTP_HOST'].$_SESSION["s_sessVars"]["loginURL"]."/images/arrow.but.gif";
echo "<table cellpadding=0 cellspacing=1 border=0>";
echo "<tr bgcolor=#EFEFEF><td><b><font color=gray>Object&nbsp;</b></td>".
 "<td align=center><b><font color=gray>&nbsp;Download</b></td>".
 "<td align=center><b><font color=gray>&nbsp;Info</b></td>".
 "</tr>\n";
 echo "<tr><td>&nbsp;</td></tr>";

while ($sql->ReadRow() ) {
	$tmperror = "";
	$linkcss  = "";
	$link_id	= $sql->RowData[0];
	$doc_name	= $sql->RowData[1];
	
	$linkpath = linkpath_get( $link_id );
	if (!file_exists($linkpath)) {
		$linkcss  = " id=yNavGray";
		$tmperror = "file does not exist.";
	}
	
	$outname    = htmlspecialchars($doc_name);
	$downlink   = $fixdown . $link_id;
	$objlink    = $fixobj  . $link_id;
	
	echo "<tr><td align=right>";
	echo "<a href=\"".$objlink."\"><img src=\"".$grafurl_arr."\" border=0></a>&nbsp;</td><td>";
	echo "<a href=\"".$downlink."\"".$linkcss.">".$doc_name."</a>";
	echo "</td><td>";
	if ($tmperror!="") echo "&nbsp;&nbsp;<font color=red>Problem:</font> ".$tmperror.""; 
	echo "</td></tr>\n";
	
}
echo "</table>";
echo "<hr></ul>";
htmlFoot();