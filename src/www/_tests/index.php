<?php
/**
 * test: INDEX
 * $Header: trunk/src/www/_tests/www/test/index.php 59 2018-11-21 09:04:09Z $
 * @package UT_index.php
 * @author  qbi 
 * @version 1.0
 */

extract($_REQUEST); 
session_start(); 


require_once ("reqnormal.inc");

global $error, $varcol;

$error = & ErrorHandler::get();
$sqlo  = logon2( ); // URL-link for the first db-login; e.g. for object ($_SERVER['PHP_SELF']."?id=".$id)
if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();



$title				 = "UNITTEST Index";

$infoarr			 = NULL;
$infoarr["scriptID"] = "UT_index";
$infoarr["title"]    = $title;
$infoarr["form_type"]= "tool"; // "tool", "list"
$infoarr["locrow"] = array( array("../pionir/rootsubs/rootFuncs.php", "Administration") );
$infoarr['help_url'] = 'ad.AppDevHOME.html'; 

$pagelib = new gHtmlHead();
$headarr = $pagelib->startPage($sqlo, $infoarr);



?>
<ul>
<ul>

<LI><a href="unitest.php">Single UnitTest</a> (select one module)</li>
<LI><a href="unittest_many.php">Many UnitTests</a></li>

<LI><a href="xml_rpc_client2.php">XML_RPC UnitTest</a> </li>
<LI><a href="validation.php">Validation Tests</a> </li>

<br>
<li><a href="../../../pionir/p.php?mod=TST/create_UT_objects">Create UnitTest objects</a></li>
<LI><a href="../../../pionir/p.php?mod=DEF/root/install/g.install.export.UT_data">Export UnitTest objects as Paxml</a></li>
<LI><a href="test_touchPages.php">Test GUI-pages<a></li>
<LI><a href="test_GUI_getList01.php">Create List of OBJECT-GUI-Files<a></li>
<br>
</ul>

</ul>
<?

htmlFoot("<hr>");
