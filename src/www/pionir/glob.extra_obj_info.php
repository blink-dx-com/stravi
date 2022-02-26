<?php
/**
 * shows classes and objects in class for each class of a business object
 * @package glob.extra_obj_info.php
 * @author  mac (till 2002-06-01)
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   $tablename       : tablename of the business object
 * @version0 2002-05-30
 */
session_start(); 


require_once ('reqnormal.inc');
require_once ('visufuncs.inc');

$sql  = logon2( $_SERVER['PHP_SELF'] );
$sql2 = logon2( $_SERVER['PHP_SELF'] );

$tablename=$_REQUEST['tablename'];

$tablename_nice = tablename_nice2($tablename);
$title = "Show Information about the Classes of this table";


$infoarr=array();
$infoarr["title"] = $title;
$infoarr["form_type"]= "list";
$infoarr["locrow"]= array( array("searchAdvance.php?tablename=$tablename", "advanced Serach" ) );
$infoarr["obj_name"] = $tablename;

$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);
echo "<ul>\n";
$tabobj = new visufuncs();

$extra_obj_exist=glob_column_exists($tablename, 'EXTRA_OBJ_ID');
if ($extra_obj_exist != 1) htmlFoot('', 'The table '.$tablename_nice.' does not support classes.');

echo "<br>\n";
$headx  = array("Class", "Number of Objects");
$headOpt = array("colopt" => array( 1 => " align=right") );
$tabobj->table_head($headx, $headOpt);
$topt = NULL;

$sql->query("SELECT extra_class_id, nice_name, name FROM extra_class WHERE table_name = '$tablename' ORDER BY UPPER(nice_name)");
while($sql->ReadRow()) {
  $eclass_id = $sql->RowData[0];
  $nice_name = $sql->RowData[1];
  $sql2->query("SELECT count(extra_obj_id) FROM $tablename WHERE extra_obj_id IN ".
	       "(SELECT extra_obj_id FROM extra_obj where extra_class_id = $eclass_id)");
  $sql2->ReadRow();
  
  $tmplink = "<a href=\"edit.tmpl.php?tablename=EXTRA_CLASS&id=$eclass_id\">". (($sql->RowData[1] == "") ? $sql->RowData[2] :
  			 $sql->RowData[1]). "</a>";
  $tmpnum = "<a href=\"view.tmpl.php?&t=$tablename&condclean=1&searchClass=".$sql->RowData[2]."\">" .
		$sql2->RowData[0]."</a>";
 
  
  $tabdata = array($tmplink, $tmpnum );
  $tabobj->table_row($tabdata, $topt);
  
}
$sql2->query("SELECT count(*) FROM $tablename WHERE extra_obj_id IS NULL");
$sql2->ReadRow();

$tmplink ="<i><a href=\"view.tmpl.php?&t=$tablename&condclean=1&searchCol=x.EXTRA_OBJ_ID&searchBool=%20IS%20NULL&searchtxt=\">$tablename_nice not in a Class</a></i>";
$tmpnum = $sql2->RowData[0];
$tabdata = array($tmplink, $tmpnum );
$tabobj->table_row($tabdata, $topt);

$sql2->query("SELECT count(*) FROM $tablename");
$sql2->ReadRow();

$tmplink ="<b><i>sum of $tablename_nice</i></b>";
$tmpnum  =  $sql2->RowData[0];
$tabdata = array($tmplink, $tmpnum );
$topt["bgcolor"] = "#88CCFF";
$tabobj->table_row($tabdata, $topt);

$tabobj->table_close();

$pagelib->htmlFoot();
