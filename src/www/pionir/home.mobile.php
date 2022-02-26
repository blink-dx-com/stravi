<?php
/**
 * MOBILE home page
 * @package home.mobile.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param viewmode
 *   'desktop' : back to desktop
 */

session_start(); 


require_once ('reqnormal.inc');
require_once ("f.textOut.inc");
require_once ("o.DB_USER.subs2.inc");
require_once ("gui/go.qSearchGui.inc");

class layer_mobile {

function layer_show( &$sqlo ) {
	
	echo '<br /><div style="padding-left:9px; margin-left:9px;">';
	
	$home_proj_id = oDB_USER_sub2::userHomeProjGet( $sqlo );
	
	echo '<table border=0 cellpadding=10><tr valign=top><td width=70%>'."\n";

	$txtopt=NULL;
	$textoutObj = new textOutC($txtopt);
	$flist=array();
	$flist[] = array("ty"=>"head", "txt"=>'My personal area', "lay"=>"1" );
	
	$flist[] = array("ty"=>"lnk", "txt"=>'theme park', "iicon"=>"ic13.theme_park.gif",
	   "href"=>'home.php', 'li'=>'br');
	$flist[] = array("ty"=>"lnk", "txt"=>'My Search Center', "iicon"=>"ic.myqueryLogo.40.png", "icon_opt"=>'height=15',
	   "href"=>'obj.link.c_query_mylist.php', 'li'=>'br');
	if ($home_proj_id) $flist[] = array("ty"=>"lnk", "txt"=>'Home project',  "href"=>'edit.tmpl.php?t=PROJ&id='.$home_proj_id,
		"iicon"=>'icon.PROJ.gif', 'li'=>'br');
	else  $flist[] = array("ty"=>"lnk", "txt"=>'Home project (not exists)',  "href"=>'', "iicon"=>'icon.PROJ.gif', 'li'=>'br' );
	$flist[] = array("ty"=>"lnk", "txt"=>'MyAccount (settings)',  "href"=>'obj.db_user.settings.php', "iicon"=>'icon.DB_USER.gif', 'li'=>'br');
	$flist[] = array("ty"=>"lnk", "txt"=>'Show Desktop version', "iicon"=>"but.newwindow.gif",
			"href"=> $_SERVER['PHP_SELF'].'?viewmode=desktop', 'li'=>'br');
	
	$flist[] = array("ty"=>"headclose"); 
	$textoutObj->linksOut($flist);
	
    echo '</table>';
	
	echo "</div>\n";
	
}

}


 

//  * @package sample
// $Header: trunk/src/www/pionir/home.mobile.php 59 2018-11-21 09:04:09Z $

// --------------------------------------------------- 
global $error;

$error = & ErrorHandler::get();
$sqlo  = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot();

$title		= 'Mobile home';

$infoarr			 = NULL;
$infoarr['scriptID'] = '';
$infoarr['title']    = $title;
$infoarr['form_type']= 'tool';

$pagelib = new gHtmlHead();

$headOpt=array();
$pagelib->_PageHead($infoarr['title'], $headOpt);

if ($_REQUEST['viewmode']=='desktop') {
	
	$_SESSION['s_sessVars']["g.surfMode"]='desktop';
	echo '<script language="JavaScript">'."\n";
	echo '<!--'."\n";
	
	echo 'if ( parent.oben != null ) { '."\n"; // check if exists
	echo '  parent.location.href="main.fr.php";'."\n";
	echo '}'."\n";
	
	echo ' //-->'."\n";
	echo '</script>'."\n";
	return;
}

$mainlib = new layer_mobile();
$mainlib->layer_show($sqlo);

$pagelib->htmlFoot();


