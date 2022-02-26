<?php
/**
 * index of MEAT table exports
 * @package i_meta_export.php
 * @author  qbi
 */

session_start(); 


require_once ("reqnormal.inc");
require_once("f.textOut.inc");

// --------------------------------------------------- 
global $error;

$error = & ErrorHandler::get();
$sqlo  = logon2( $_SERVER['PHP_SELF'] ); // URL-link for the first db-login; e.g. for object ($_SERVER['PHP_SELF']."?id=".$id)
if ($error->printLast()) htmlFoot();


$title				 = "Export system table content";
$infoarr			 = NULL;
$infoarr["scriptID"] = "";
$infoarr["title"]    = $title;
$infoarr["form_type"]= "tool";
$infoarr["locrow"]   = array( array("../rootFuncs.php", "super user funcs") );


$pagelib = new gHtmlHead();
$pagelib->startPage($sqlo, $infoarr);

echo "<ul>";
?>

<LI><a href="meta-data-exp.php">Export META content</a> <P>
<? $my_url="cct_table=EXTRA_CLASS&cct_where=".rawurlencode(" NAME NOT LIKE '\_%' ESCAPE '\\' ")."&cct_back=\"../../rootsubs/init/index.php\""; ?>
<LI><a href="../../impexp/partisanxml/export.bulk.php?<?echo $my_url;?>">Export EXTRA Classes and attributes </a>
<form name="myForm" action='../../impexp/partisanxml/export.bulk.php' method=POST>
   <INPUT type=hidden name="cct_table[0]" value="H_BASE_OF_PROOF">
   <INPUT type=hidden name="cct_table[1]" value="H_KIND_OF_INTERACT">
   <INPUT type=hidden name="cct_table[2]" value="H_POA_JOB">
   <INPUT type=hidden name="cct_table[3]" value="H_PROTO_KIND">
   <INPUT type=hidden name="cct_table[4]" value="H_REF_POS_SYS">
   <INPUT type=hidden name="cct_table[5]" value="H_SPOT_SPECIALITIES">
   <INPUT type=hidden name="cct_table[6]" value="H_STATE">
   <INPUT type=hidden name="cct_table[7]" value="H_USAGE">
   <INPUT type=hidden name="cct_table[8]" value="H_UNIT">
   <INPUT type=hidden name="cct_back" value="../../rootsubs/init/index.php">
   
   <LI><a href=javascript:document.myForm.submit();>  Export LOOKUP content</a> (H_TABLES)
</form> <p>
<li><a href="h_table_export.php">Export H_TABLES using PartisanXML</A></li><br />
<? $my_url="cct_table=PROJ&cct_where=".rawurlencode(" PROJ_ID=9840 ")."&cct_back=".rawurlencode("../../rootsubs/init/index.php"); ?>
<LI><a href="../../impexp/partisanxml/export.bulk.php?<?echo $my_url;?>"> Export sandbox elements
from project "release sandbox"</a><p></li>
<?php



htmlFoot("<hr>");