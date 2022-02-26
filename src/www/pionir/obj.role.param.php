<?
/**
 *  update/add rights in a role
 * - called by obj.role.xedit.php
 * - DB_MODIFIED: RIGHT_IN_ROLE
 * @package obj.role.param.php
 * @author  Adrian, Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param $id of the role
          $_POST - id's of the right
 */


session_start(); 

require_once ('reqnormal.inc'); // includes all normal *.inc files
require_once 'o.ROLE.mod.inc';

$id=$_REQUEST['id'];

$table = "ROLE";
$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot();



$title       = "Update/add rights in a role";

$infoarr=array();
$infoarr["title"] = $title;
$infoarr["form_type"]= "obj";
$infoarr["obj_name"] = $table;
$infoarr["obj_id"]   = $id;
$infoarr["show_name"]= 1;

$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);


if (!$id) {
	htmlFoot("Error", "Need a role_id");
}

if ( $_SESSION['sec']['appuser'] != "root" ) { // !$_SESSION['s_suflag'] 
     htmlErrorBox( "Error",  "Only root can execute this!");
     htmlFoot();
}

$mq="select user_right_id from right_in_role where role_id=$id";
$sql->query($mq);
$right_vector=array();
$i=0;
while($sql->ReadRow()) {
    $right_vector[$i++]=$sql->RowData[0];
}


$mod_lib = new oROLE_mod($sql, $id);
$pagelib->chkErrStop();

foreach($_POST as $var_name=> $var) {
      if($var_name!="id") {
          $user_right_id = $var_name;
          if (($var=="on") and (!in_array($var_name,$right_vector))) {	
				$mod_lib->add_user_right($sql, $user_right_id);
          }

         if (($var=="off") and (in_array($var_name,$right_vector))) {
             $mod_lib->remove_user_right($sql, $user_right_id);
         }
         
         if ($error->Got(READONLY))  {
             $mod_lib->close();
             $pagelib->chkErrStop();
         }
      }
}
$mod_lib->close($sql);

$url="edit.tmpl.php?t=ROLE&id=".$id;
js__location_replace($url);


htmlFoot();
