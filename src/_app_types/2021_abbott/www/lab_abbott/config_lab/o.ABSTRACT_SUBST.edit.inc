<?php
// INPUT: see edit.tmpl.inc => o.{TABLE}.edit.inc


if(empty($edit_tabs)) $edit_tabs=array('modes'=>array());
$edit_tabs['modes']["xlabasr"]   = array('n'=>"Blink Info");
unset($edit_tabs['modes']["bom"]);
unset($edit_tabs['modes']["vers"]);

