<?php
/**
 * index of database cleanHouse
 * $Header: trunk/src/www/pionir/rootsubs/db_clean/index.php 59 2018-11-21 09:04:09Z $
 * @package db_cleanIndex
 * @author  qbi
 */

extract($_REQUEST); 
session_start(); 


require_once ("reqnormal.inc");

// --------------------------------------------------- 
global $error, $varcol;

$error = & ErrorHandler::get();
$sqlo  = logon2(  );
if ($error->printLast()) htmlFoot();

$title				 = "[CleanHouse] identify/remove old data";
$title_sh			 = "CleanHouse";

$infoarr			 = NULL;
$infoarr["scriptID"] = "db_cleanIndex";
$infoarr["title"]    = $title;
$infoarr["title_sh"] = "CleanHouse";
$infoarr["form_type"]= "tool";
$infoarr["locrow"]   = array( array("../rootFuncs.php", "Administration") );

$pagelib = new gHtmlHead();
$headarr = $pagelib->startPage($sqlo, $infoarr);
echo "<ul>";

echo '<li><a href="../../p.php?mod=DEF/root/o.CCT_ACC_UP.AutoClean">AutoClean CCT_ACC_UP</a></li>';

echo "</ul>";
htmlFoot("<hr>");