<?php
/**
 * refreshes the cct-table/cct-column & varcol-cache in shared memory (unix) or session-variables (windows)
 * @package glob.cache.refresh.php
 * @author mac
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param    $back .. where to go back
         [$auto_back] .. boolean automatical go back using javascript (default: false)
 * @version0 2002-09-30
 */
session_start(); 

require_once ("reqnormal.inc");
require_once ('get_cache.inc');
require_once ('javascript.inc');
require_once ("visufuncs.inc");

class globCacheRefreshGui {

	function showInitWarnings() {
		global $error;
		
		echo 'Show warnings:<br>'."\n";
		$CacheLib = new gInitCache();
		$CacheLib->init_cache( $_SESSION['sec']['dbuser'], $_SESSION['sec']['passwd'], $_SESSION['sec']['db'], $_SESSION['sec']['_dbtype']);
		$warnarr = $CacheLib->getWarnArr();
		
		$tabobj = new visufuncs();
		$headOpt = array( "title" => "Warning array",);
		$headx  = array ("Structure", "Table", 'Column', 'Info', "Warn");
		$tabobj->table_head($headx,   $headOpt);
	
		reset ($warnarr); 
		foreach( $warnarr as $dataArr) {
			$tabobj->table_row ($dataArr);
		}
		reset ($warnarr); 
		
		$tabobj->table_close();
		$error->reset();
	}
}

$sqlo  = logon2( $_SERVER['PHP_SELF'] );

$auto_back = empty($_GET['auto_back']) ? 0 : 1;
$error     = & ErrorHandler::get();

if (empty($_GET['back'])) $back = 'home.php';
else {
  $back = $_GET['back'];
  unset($_GET['back']);
  unset($_GET['auto_back']);
  $back = js__history_back_get_url($back);
}

$title = 'Refreshing global cache ...';
$infoarr			 = NULL;
$infoarr["scriptID"] = "script.php";
$infoarr["title"]    = $title;
$infoarr["form_type"]= "tool";
$infoarr['help_url'] = 'g.cache-refresh.html';
$infoarr["locrow"] = array( array($back, "back???") );

$pagelib = new gHtmlHead();
$pagelib->startPage($sqlo, $infoarr);

$mainlib = new globCacheRefreshGui();

get_cache(1, !$auto_back); // force renew, but display only if not auto-back
if ($error->got(CCT_ERROR)) { // on error die
  $error->printLast();
  htmlFoot();
}
if ($error->got(CCT_WARNING) && !$auto_back) { // display all warnings when not automatically back-warding
	info_out('WARNING', 'There where the following warnings on initializing cache.');
	$error->printAll();	
	
	$mainlib->showInitWarnings();
}

if ($auto_back) {
  js__location_replace($back, 'done');
} else {
  echo '<blockquote><a href="'.$back.'">done</a></blockquote>';
}
htmlFoot();
