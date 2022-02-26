<?php
/**
 * show image from IMG_PATH
 * @package glob.obj.img_show.php

 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param    tablename
  	    primid
	    extension
 */
session_start(); 

require_once 'globals.inc';

if(!glob_loggedin()) {
    echo "ERROR: Not logged in.";
    exit;
}

Header("Content-type: image/jpg");

$tablename=$_REQUEST['tablename'];
$primid=$_REQUEST['primid'];
$extension=$_REQUEST['extension'];



$img_path=$_SESSION['globals']['data_path'] ."/". $tablename.".".$primid.".".$extension;
$retVal = readfile ( $img_path );
