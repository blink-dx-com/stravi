<?php
/**
 * execute a SQL-query, use object: LINK; class: queryAdv
 * @package obj.link.c_KeyGen_exec.php
 * @author  Michael Brendel (Michael.Brendel@clondiag.com)
 * @param int $id (LINK_ID) 
 * @param int $go (0,1)
 * @param array $myquvarval[] : values of user-query variables    ["var name"] = "value"
 */
session_start(); 

require_once ('reqnormal.inc');
require_once ('obj.link.c_KeyGen_sub2.inc');

// --------------------------------------------------- 
global $error, $varcol;

$error = & ErrorHandler::get();
$sqlo  = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();

$id 		= $_REQUEST['id'];
$go 		= $_REQUEST['go'];
$myquvarval = $_REQUEST['myquvarval'];

$tablename  = 'LINK';
$title		= 'generate an authorization key allowing the execution of an advanced SQL-query';

$infoarr			 = NULL;
$infoarr['scriptID'] = 'obj.link.c_KeyGen_exec.php';
$infoarr['title']    = $title;
$infoarr['title_sh'] = 'advanced SQL';
$infoarr['form_type']= 'obj';
$infoarr['design']   = 'norm';
$infoarr['obj_name'] = $tablename;
$infoarr['obj_id']   = $id;
$infoarr['checkid']  = 1;

$pagelib = new gHtmlHead();
$headarr = $pagelib->startPage($sqlo, $infoarr);

$mainlib = new oLINK_c_KeyGen_gui( $id, $go, $myquvarval );
echo '<i>To use this tool, you need the role-right: "'.$mainlib->getRoleRightName().'"</i><br /><br />'."\n";
$mainlib->initChecks( $sqlo );
$pagelib->chkErrStop();

$mainlib->updateKey( $sqlo );
$pagelib->chkErrStop();

$pagelib->htmlFoot();