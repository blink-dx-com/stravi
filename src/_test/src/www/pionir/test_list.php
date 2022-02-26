<?php
/*MODULE:  _SCRIPT_NAME_.EXT (without path)
  DESCR:   Selects file to load for import.
  AUTHOR:  qbi
  INPUT:   $_REQUEST["id"]   (e.g. ARRAY_LAYOUT_ID)
  VERSION: 0.1 - 20081002
  PAX_SUBINFO: $_REQUEST done (do not use register_globals variables !!!)
*/
extract($_REQUEST); 
session_start(); 

require_once ("globals.inc"); 
require_once ("func_head2.inc"); // includes all normal *.inc files

// --------------------------------------------------- 
global $error, $varcol;

$sqlo  = logon2(  );

$_REQUEST["id"]= 234;
$id 				 = $_REQUEST["id"];
$tablename     = "EXP";
$title       		 = "Example for a list";
$infoarr			 = NULL;

$infoarr["scriptID"] = "";
$infoarr["title"]    		= $title;
$infoarr["title_sh"] 		= "example";
$infoarr["form_type"]	= "list";
$infoarr["obj_name"] 	= $tablename;
$infoarr["obj_cnt"]		= 1;
$infoarr['help_url'] 		= "o.EXAMPLE.htm";

$pagelib = new gHtmlHead();
$headarr = $pagelib->startPage($sqlo, $infoarr);

echo "<ul>";
print_r($headarr );
echo "seesion_id: ". session_id()."<br>";

echo "hallo23";

echo "</html>";


