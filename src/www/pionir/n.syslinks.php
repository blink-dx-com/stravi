<?php
/**
 * System links for common users
 * @package n.syslinks.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
session_start(); 
require_once ("reqnormal.inc"); // includes all normal *.inc files
global $error;

$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot();

$title       = "System info portal";
$infoarr=array();
$infoarr["title"] = $title;
$infoarr["form_type"]= "tool";
$infoarr["locrow"] = array (array("home.php","home"), array("n.syslinks.php", "System info") );


$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);

?>
<ul>
<b>Table Overview</b>
<ul>
<a href="glob.tabsel.php?parx[action]=detail&parx[type]=BO"><img src="images/but.list2.gif" border=0>&nbsp; Table overview</a> <br>
</ul>
<br>


<b>System tables</b>
<ul>
<a href="view.tmpl.php?t=DB_USER"><img src="images/icon.DB_USER.gif" border=0>&nbsp; users</a> 
	&nbsp;&nbsp;&nbsp;[<a href="rootsubs/o.DB_USER.loggedin.php">who is logged in ?</a>]<br>
<a href="view.tmpl.php?t=USER_GROUP"><img src="images/icon.USER_GROUP.gif" border=0>&nbsp;  groups</a><br>
<a href="view.tmpl.php?t=ROLE"><img src="images/icon.USER_ROLES.gif" border=0>&nbsp;  roles</a><br>
</ul>
<br>

<b>Software</b><ul>
	<a href="sysinfo.php"><img src='images/i13_info.gif' border=0 hspace=7> Software versions/settings</a> <br>
	<a href="news.php"><img src='images/NEW.logo.gif' border=0 hspace=5> Software news log</a><br>
</ul>
</ul>
<?php
$pagelib->htmlFoot();
