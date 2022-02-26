<?php
/*MODULE:  f.pageSubs.inc
  DESCR:   functions for web-pages
  AUTHOR:  qbi
  VERSION: 0.1 - 20080203
*/

class fPageSubs {

function navigateNext( 
	$infoarr // array[] = array("url"=>, "txt"=> )
	) {
	
	
	if ( !sizeof($infoarr) ) return;
	
	if ( $_SESSION['s_sessVars']["loginURL"] == dirname($_SERVER['PHP_SELF']) ) {
		$pioDir = "";
	} else {
		$pioDir = $_SESSION['s_sessVars']["loginURL"]."/";
	}
	
	
	
	echo "<table cellpadding=3 cellspacing=1 border=0 width=300 background=\"".$pioDir."images/bg.listtab.gif\">";
	echo "<tr><td>";
	echo "<img src=\"".$pioDir."images/ic.navi.png\">&nbsp;&nbsp;&nbsp;<b>You can go now to:</b>";
	echo "</td><tr></table>\n";
	
	echo "<ul style=\"padding-left:25px; margin-left:9px; padding-top:10px; line-height:22px\" TYPE=SQUARE>\n";
	foreach( $infoarr as $dummy=>$textarr) {
		
		echo "<b><img src=\"".$pioDir."images/but.go13.gif\" hspace=5> <a href=\"".$textarr["url"]."\">".$textarr["txt"]."</a></b><br>\n";
	}
	reset ($infoarr); 
	
	echo "</ul>";
	
}

}