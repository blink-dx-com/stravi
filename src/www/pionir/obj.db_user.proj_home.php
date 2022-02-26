<?php
/**
 * create new home directory
 * @package obj.db_user.proj_home.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   $user_id	
 * @version0 2001-05-21 
 */ 
session_start(); 

require_once("func_head.inc");
require_once("db_access.inc");
require_once("globals.inc");
require_once('insert.inc');
require_once('o.DB_USER.subs.inc');

$error  = & ErrorHandler::get();
$sql    = logon2( $_SERVER['PHP_SELF'] );
$user_id =$_REQUEST["user_id"];

$title = "Create home project for user";
$infoarr=array();
$infoarr["title"] = $title;
$infoarr["form_type"]= "obj";
$infoarr["obj_name"] = "DB_USER";
$infoarr["obj_id"] = "$user_id";
$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);


if ( !$user_id ) {
	echo "ERROR: no user id given.<br>\n";
	return;
}

echo "<p>Creating home directory ...<p>";

if ( $_SESSION['sec']['appuser'] != "root" ) {
  htmlFoot( "ERROR", "Creation of home directory not allowed, you are not root!");
}

$prim_id_proj = DB_userC::homeProjCreate($sql, $user_id);

if ( $error->Got(READONLY))  {
     $error->printAll();
	 htmlFoot();
}
echo "Ok: new project (ID:$prim_id_proj) created. <P>";

$tmpurl = $infoarr["back_url"];
?>

  <script>
	location.href="<? echo $tmpurl?>";
  </script>
