<?php
/**
 * execute a query
 * @package obj.link.c_query_exec.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param $id   (of object LINK)  
         $myquvarval[] : values of user-query variables    ["var name"] = "value"
         $myqu_go 0|1|2
 */


session_start(); 


require_once ('db_access.inc');
require_once ('globals.inc');
require_once ('func_head.inc');
require_once ('access_check.inc');
require_once ('table_access.inc');       
require_once ("sql_query_dyn.inc");
require_once ("subs/obj.link.c_query_sub2.inc");
require_once ("class.history.inc");
require_once ("f.objview.inc");
require_once ("f.text_html.inc");	


$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] );
$sql2  = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot();

$id = $_REQUEST["id"];
$myquvarval = $_REQUEST["myquvarval"];
$myqu_go = $_REQUEST["myqu_go"];



$title = 'Execute user query';  

$headtext = "&nbsp;[<a href=\"obj.link.c_query_mylist.php\">my-query center</a>] ";
$infoarr=array();
$infoarr['help_url'] = 'o.LINK.class.query.html';
$infoarr["title"] = $title;
$infoarr["form_type"]= "obj";
$infoarr["obj_name"] = "LINK";
$infoarr["obj_id"]   = $id;
$infoarr["checkid"]  = 1;
$infoarr['inforow']      = $headtext;
$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);

echo "<blockquote>";

if ( $_SESSION['userGlob']["g.debugLevel"]>0 ) {
    echo "<B>INFO:</B> DEBUG-mode supports 3 levels in this script.<br>\n";
} 

$nicename=tablename_nice2('LINK');
$varcol    = & Varcols::get(); 
$t_rights = tableAccessCheck( $sql, 'LINK' );
if ( $t_rights['read'] != 1 ) {
	tableAccessMsg( $nicename, 'read' );
	return;
}
 
$o_rights = access_check($sql, 'LINK', $id);
if ( !$o_rights['read'] ) htmlFoot('ERROR', 'no read permissions on document!');

$sqls="select name, extra_obj_id, notes from LINK where LINK_ID=".$id;
$sql->query("$sqls");
if ( $sql->ReadRow() ) {	
		$link_name = $sql->RowData[0];
        $extra_obj_id = $sql->RowData[1];
        $notes = $sql->RowData[2];
} else {
    htmlFoot("Error", "no document-id");
}

$hist_obj = new historyc();
$hist_obj->historycheck( "LINK", $id );
htmlShowHistory();


echo "<font color=gray>Name: </font><B>".htmlspecialchars ( $link_name )."</B><br>\n";
if ( strlen($notes) ){
    $html_notes = htmlspecialchars( $notes );
    echo "<font color=gray>Notes:</font>";
    echo "<pre>";
    f_text_html::notes_out($sql2, $html_notes);
    echo "</pre><br>";
}
echo "<br>\n";

if ( !$extra_obj_id ) htmlFoot("Error", "not of class 'query'");
if ( $extra_obj_id ) $extra_class = $varcol->obj_id_to_class_name ( $extra_obj_id );    
if ( $extra_class!="query" )  htmlFoot("Error", "not of  class 'query'"); 

$values_all = $varcol->select_by_name ($extra_obj_id);
$values = &$values_all["values"];

$queryForm = new oLinkQueryGui();

$queryForm->form_manage( 
    $sql, 
    $values,      // class values
    $myquvarval,  // input variables
    "obj.link.c_query_exec.php?id=".$id, // next URL 
    $extra_obj_id,  
    $myqu_go       // go flag, if no variables are set
    ); 


htmlFoot();
