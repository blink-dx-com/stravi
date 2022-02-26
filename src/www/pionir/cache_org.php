<?php
/**
 * organize cache clear 
 * @package cache_org.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   cache_file  ( no directory-PATH, due to security! )
	      [cache_file2] (same as above, use it if you need to delete two files)
		  [$backurl] : urlencode(url)
 */
session_start(); 


require_once ('reqnormal.inc');
require_once("javascript.inc");

$cache_file = $_REQUEST["cache_file"];
$cache_file2= $_REQUEST["cache_file2"];
$backurl= $_REQUEST["backurl"];

$pagelib = new gHtmlHead();
$pagelib->PageHeadLight('cache org');

if (!empty($cache_file)) {
  $cache_file_path= $_SESSION['globals']['http_cache_path'] .'/'. $cache_file;
  if (file_exists($cache_file_path)) {
	unlink ($cache_file_path);
  }
}
if (!empty($cache_file2)) {
  $cache_file_path= $_SESSION['globals']['http_cache_path'] .'/'. $cache_file2;
  if (file_exists($cache_file_path)) {
	unlink ($cache_file_path);
  }
}

$urlnew = "";
if ($backurl!="") $urlnew = urldecode($backurl);

if ($urlnew!="") {
	js__location_replace($urlnew);
} else {
	js__history_back2();
} 

$pagelib->htmlFoot();

