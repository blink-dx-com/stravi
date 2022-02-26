<?php
/**
 * Export H_TABLES using PartisanXML
 * $Header: trunk/src/www/pionir/rootsubs/init/h_table_export.php 59 2018-11-21 09:04:09Z $
 * @package h_table_export.php
 * @author  qbi
 * @version 1.0
 */
extract($_REQUEST); 
session_start(); 


require_once ("reqnormal.inc");

// --------------------------------------------------- 
global $error, $varcol;

$error = & ErrorHandler::get();
$sqlo  = logon2( $_SERVER['PHP_SELF'] ); // URL-link for the first db-login; e.g. for object ($_SERVER['PHP_SELF']."?id=".$id)
if ($error->printLast()) htmlFoot();

$title = "Export H_TABLES using PartisanXML";

$infoarr			 = NULL;
$infoarr["scriptID"] = "script.php";
$infoarr["title"]    = $title;
$infoarr["title"]    = "Export H_TABLES";
$infoarr["form_type"]= "tool"; // "tool", "list"
$infoarr["locrow"] = array( 
			array("../rootFuncs.php", "Administration"),
			array("i_meta_export.php", "Export home")
						  );

$pagelib = new gHtmlHead();
$headarr = $pagelib->startPage($sqlo, $infoarr);



 ?>
<br>
        <table border=0>
         <tr>
           <td> &nbsp;&nbsp; </td>
           <td colspan=3>
             <font face="Helvetica" size=+1> H_TABLES found in the database </font>
           </td>
         </tr>

         <tr>
           <td colspan=4> <hr size="2" noshade> </td>
         </tr>

         <tr>
          <td> &nbsp;&nbsp; </td>
          <td> <font face="Helvetica"><b>H_BASE_OF_PROOF</font> </td>
          <td> &nbsp; </td>
          <td> <font face="Helvetica"><b><a href="../../impexp/partisanxml/export.bulk.php?cct_table[0]=H_BASE_OF_PROOF">export</a></font></td>
         </tr>

         <tr>
          <td> &nbsp;&nbsp; </td>
          <td> <font face="Helvetica"><b>H_EXP_RAW_DESC</font> </td>
          <td> &nbsp; </td>
          <td> <font face="Helvetica"><b><a href="../../impexp/partisanxml/export.bulk.php?cct_table[0]=H_EXP_RAW_DESC">export</a></font></td>
         </tr>

         <tr>
          <td> &nbsp;&nbsp; </td>
          <td> <font face="Helvetica"><b>H_EXP_RAW_DESC_COL</font> </td>
          <td> &nbsp; </td>
          <td> <font face="Helvetica"><b>Cannot be exported !</font> </td>
         </tr>

         <tr>
          <td> &nbsp;&nbsp; </td>
          <td> <font face="Helvetica"><b>H_SIS_JOB</font> </td>
          <td> &nbsp; </td>
          <td> <font face="Helvetica"><b><a href="../../impexp/partisanxml/export.bulk.php?cct_table[0]=H_SIS_JOB">export</a></font></td>
         </tr>

         <tr>
          <td> &nbsp;&nbsp; </td>
          <td> <font face="Helvetica"><b>H_KIND_OF_INTERACT</font></td>
          <td> &nbsp; </td>
          <td> <font face="Helvetica"><b><a href="../../impexp/partisanxml/export.bulk.php?cct_table[0]=H_KIND_OF_INTERACT">export</a></font></td>
         </tr>

         <tr>
          <td> &nbsp;&nbsp; </td>
          <td> <font face="Helvetica"><b>H_POA_JOB </font></td>
          <td> &nbsp; </td>
          <td> <font face="Helvetica"><b><a href="../../impexp/partisanxml/export.bulk.php?cct_table[0]=H_POA_JOB">export</a></font></td>
         </tr>

         <tr>
          <td> &nbsp;&nbsp; </td>
          <td> <font face="Helvetica"><b>H_PROTO_KIND </font></td>
          <td> &nbsp; </td>
          <td> <font face="Helvetica"><b><a href="../../impexp/partisanxml/export.bulk.php?cct_table[0]=H_PROTO_KIND">export</a></font></td>
         </tr>

         <tr>
          <td> &nbsp;&nbsp; </td>
          <td> <font face="Helvetica"><b>H_REF_POS_SYS</font></td>
          <td> &nbsp; </td>
          <td> <font face="Helvetica"><b><a href="../../impexp/partisanxml/export.bulk.php?cct_table[0]=H_REF_POS_SYS">export</a></font></td>
         </tr>

         <tr>
          <td> &nbsp;&nbsp; </td>
          <td> <font face="Helvetica"><b>H_SPOT_SPECIALITIES </font></td>
          <td> &nbsp; </td>
          <td> <font face="Helvetica"><b><a href="../../impexp/partisanxml/export.bulk.php?cct_table[0]=H_SPOT_SPECIALITIES">export</a></font></td>
         </tr>

         <tr>
          <td> &nbsp;&nbsp; </td>
          <td> <font face="Helvetica"><b>H_STATE </font></td>
          <td> &nbsp; </td>
          <td> <font face="Helvetica"><b><a href="../../impexp/partisanxml/export.bulk.php?cct_table[0]=H_STATE">export</a></font></td>
         </tr>

         <tr>
          <td> &nbsp;&nbsp; </td>
          <td> <font face="Helvetica"><b>H_UNIT </font></td>
          <td> &nbsp; </td>
          <td> <font face="Helvetica"><b><a href="../../impexp/partisanxml/export.bulk.php?cct_table[0]=H_UNIT">export</a></font></td>
         </tr>

         <tr>
          <td> &nbsp;&nbsp; </td>
          <td> <font face="Helvetica"><b>H_VAL_INIT </font></td>
          <td> &nbsp; </td>
          <td> <font face="Helvetica"><b>Cannot be exported ! </font></td>
         </tr>

         <tr>
          <td> &nbsp;&nbsp; </td>
          <td> <font face="Helvetica"><b>H_WIID </font></td>
          <td> &nbsp; </td>
          <td> <font face="Helvetica"><b>Cannot be exported ! </font></td>
         </tr>

         <tr>
          <td> &nbsp;&nbsp; </td>
          <td> <font face="Helvetica"><b>H_LAY_SPOT_JOB </font></td>
          <td> &nbsp; </td>
          <td> <font face="Helvetica"><b><a href="../../impexp/partisanxml/export.bulk.php?cct_table[0]=H_LAY_SPOT_JOB">export</a></font></td>
         </tr>


        </table>
        <p>
        <p>
        &nbsp;&nbsp;
        <a href="../../impexp/partisanxml/export.bulk.php?cct_table[0]=H_BASE_OF_PROOF&cct_table[1]=H_EXP_RAW_DESC&cct_table[2]=H_SIS_JOB&cct_table[3]=H_KIND_OF_INTERACT&cct_table[4]=H_POA_JOB&cct_table[5]=H_PROTO_KIND&cct_table[6]=H_REF_POS_SYS&cct_table[7]=H_SPOT_SPECIALITIES&cct_table[8]=H_STATE&cct_table[9]=H_UNIT&cct_table[10]=H_LAY_SPOT_JOB">
        <font face="Helvetica"><b>Export all tables</font>
        </a>
        </body>
</html>
