<?php
// INPUT: see edit.tmpl.inc => o.{TABLE}.edit.inc


if(empty($edit_tabs)) $edit_tabs=array('modes'=>array());
$tmp_modes = array(
    "xlabEditExt" => array('n'=>"Edit for customers"),
);
$edit_tabs['modes'] = array_merge($edit_tabs['modes'], $tmp_modes);