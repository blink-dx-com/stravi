<?php
/**
 * Delete all temporary cached image-thumbnails on $_SESSION['globals']["http_cache_path"]
 * @package obj.img.cache_act.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   none 
 */
session_start(); 


require_once ("reqnormal.inc"); // includes all normal *.inc files

function this_thumbdel() {

	
	$pathx      = $_SESSION['globals']["http_cache_path"];
	$pattern    = "img";
	$patternlen = strlen($pattern);
	echo "- Start deleting of thumbnails ...<br>";
	if ($handle = opendir($pathx)) {
		$cnt = 0;
		while (false !== ($file = readdir($handle))) {
			if ( substr($file,0,$patternlen) == $pattern ) {
				unlink ($pathx . "/" .$file);
				$cnt++;
			}
		}

		closedir($handle);
	}
	echo "- Ready<br>";
	echo "- <b>$cnt</B> thumbnails deleted.<br>";

}

// --------------------------------------------------- 
global $error, $varcol;

$error = & ErrorHandler::get();
$sql   = logon2( ); // URL-link for the first db-login; e.g. for object ($_SERVER['PHP_SELF']."?id=".$id)
if ($error->printLast()) htmlFoot();

$tablename			 = "IMG";

$title       		 = "Clear thumbnail-cache";
$infoarr			 = NULL;
$infoarr["title"] = $title;
$infoarr["form_type"]= "list";
$infoarr["obj_name"] = $tablename;


// $infoarr["version"]  = "1.0";	// version of script
// $infoarr["scriptID"] = "scriptID";	// ID of script: e.g. "o.EXP.resana"

$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);
echo "<ul>";

ob_end_flush ( );
while (@ob_end_flush()); // send all buffered output

if (!file_exists($_SESSION['globals']["http_cache_path"]) ) {
	htmlFoot("Error", "http_cache_path '".$_SESSION['globals']["http_cache_path"]."' does not exist.");
}

this_thumbdel();

htmlFoot();

