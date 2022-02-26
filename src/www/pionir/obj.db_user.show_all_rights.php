<?php
/**
 * - show all role-righs for a user
 * - analyse table USER_RIGHT
 * @package obj.db_user.show_all_right.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param $user_id - id of the db_user
 */
session_start(); 

require_once ('reqnormal.inc');
require_once 'gui/o.DB_USER_rights_show.inc';

$user_id = $_REQUEST['user_id'];

$error = & ErrorHandler::get();
$sqlo  = logon2( $_SERVER['PHP_SELF'] ); 
$title       = 'Show role rights of a user';
$infoarr=array();
$infoarr['help_url'] = "o.role.editor.html";
$infoarr["title"]    = $title;
$infoarr["form_type"]= "obj";
$infoarr["obj_name"] = "DB_USER";
$infoarr["obj_id"]   = $user_id;
$infoarr["checkid"]  = 1;
$infoarr['design']   = 'norm';

$pagelib = new gHtmlHead();
$pagelib->startPage( $sqlo, $infoarr );

$MainLib = new oDB_USER_rigAllRig( $user_id );
$MainLib->sh_objects( $sqlo );
$MainLib->sh_functions( $sqlo );                               

$pagelib->htmlFoot();
