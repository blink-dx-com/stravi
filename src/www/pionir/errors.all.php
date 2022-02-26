<?php
/**
 * error-handling
 * shows all errors which are in error-stack -- script is to help error_handler.inc, no need to use it by yourself
 * @package errors_all.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   X
 * @version0 2002-07-02
 */
session_start(); 

require_once ('func_head.inc');
require_once ('ErrorHandler.inc');

$title = 'List of occured errors';

$infoarr			 = NULL;
$infoarr["scriptID"] = "error-handling";
$infoarr["title"]    = $title;
$infoarr["form_type"]= "tool";
$infoarr['help_url'] = 'g.error_handler.html';
$infoarr["locrow"] = array( array( $_SERVER['HTTP_REFERER'], "last page") );

$sqloDummy = NULL;
$pagelib = new gHtmlHead();
$pagelib->startPage($sqloDummy, $infoarr);

echo '<br><br><br><ul>';

$error = & ErrorHandler::get();
$error->restore(); // restore old state
$error = & ErrorHandler::get(); // make old state current state
 
$error->printAll();
 
htmlFoot('</ul><hr>');
