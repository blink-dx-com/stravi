<?php
// kill session
session_start(); 
require_once("reqnormal.inc");
require_once("javascript.inc");
$pagelib = new gHtmlHead();
$pagelib->PageHeadLight('Session kill');
session_destroy();
js__history_back2();
$pagelib->htmlFoot(' ');
