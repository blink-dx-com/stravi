<?php
/**
 * news of application code
 *  loaf file: subs/f.news_dat.inc
 * @package news.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param $option  ['dev'] give developer news
 */
session_start(); 

require_once ('reqnormal.inc');
require_once('gui/f.news.inc');

/**
 * 
 * @var array $newsArr
 *   'k': 'New', 'Fix', 'Improved', 'Removed'
 */


// -------------------

$option=$_REQUEST['option'];

$sqlo  = logon2( $_SERVER['PHP_SELF'] );
$title = 'Kernel news of Pionir';
$infoarr=array();
$infoarr["title"] = $title;
$infoarr["form_type"]= "tool";

$infoarr["locrow"]= array( array('home.php', 'home' ) );

$pagelib = new gHtmlHead();
$pagelib->startPage($sqlo, $infoarr);

$MainNewsLib = new gNewsGui();

if (empty($option)) $option = 'user';


if ($_SESSION['sec']['appuser']=="root") echo " &nbsp; [<a href=\"".
    $_SERVER['PHP_SELF']."?option=dev\">Show Developper news</a>]";
echo "<br><br><br>\n";
echo '<ul>';

include('subs/f.news_dat.inc'); // gets $newsArr

$MainNewsLib->showAll($newsArr);

echo '</ul>';

$pagelib->htmlFoot();
