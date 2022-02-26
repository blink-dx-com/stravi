<?php
/*MODULE: frame.left.lsearch.inc
  DESCR:  function for table search
  AUTHOR: qbi
  VERSION: 0.1 - 20020904
*/

require_once("sql_query_dyn.inc");
require_once("frame.left.func.inc");
require_once("view.tmpl.inc");
require_once("subs/f.tsearch.inc");

function this_lsearch( &$sql, $tablename ) {
	 global    $varcol;

/*?>
<html>
<head>
<style type="text/css">
        .xbr  { color: #EFEFEF; }
</style>
</head>
<body text="#000000"  bgcolor=#FFFFFF>
<?
*/
  $varcol = & Varcols::get();
  $viSubObj = new viewSubC($tablename);
	// frameLc::header1Out();

    if ( $tablename=="" ) {
        echo "<br><br>\n<font color=#FF8080>Error:</font>";
        echo "Need a table<br>\n";
        return;
    }
  // TBD: js__open_info(): put this to main frame
  echo '<script type="text/javascript"><!--',"\n";
  echo 'function open_info( url ) {',"\n";
  echo ' InfoWin = window.open(url, "help", "scrollbars=yes,width=650,height=500,resizable=yes");',"\n";
  echo ' InfoWin.focus();',"\n";
  echo '}',"\n";
  echo '//--></script>',"\n";

	
	$tablenice = tablename_nice2($tablename);
	$icon      = file_exists( "images/icon.".$tablename.".gif") ?
	               "images/icon.".$tablename.".gif":
	               "images/icon.UNKNOWN.gif";

	echo '<table border=0 cellspacing=0 cellpadding=0 bgcolor=#6699DD width=100%>';
	echo '<tr><td NOWRAP width=90%>';
    echo "&nbsp;<a href=\"view.tmpl.php?t=".$tablename."\" target=unten><img src=\"".$icon."\" border=0><font color=#003366> ";
    echo "<B>".$tablenice."</B></font></a></td>";
    echo "<td align=right><a href=\"".$_SERVER['PHP_SELF']."?amode=\"><img src=\"res/img/x.svg\" border=0 hspace=5></a><br>";
    echo '</td></tr></table>'."\n";

	echo '<table border=0 cellspacing=0 cellpadding=0 bgcolor=#FFFFFF width=100%>';
	echo "<tr><td>&nbsp;</td>"; // leading space
	echo "<td><font color=#000000>&nbsp;";

	$classes	    = $varcol->get_class_names($tablename);
	$colNames  		= columns_get_pos($tablename);
	$access_id_has  = cct_access_has2($tablename);
	$class_tab_has  = sizeof($classes) ? 1 : 0;
	$classname 		= $_SESSION['s_tabSearchCond'][$tablename]["c"];
	$exp_raw_desc_id= 0;

	list($selectCols, $useJoin) = $viSubObj->colinfoget($sql, $tablename, $colNames, $access_id_has, $class_tab_has, $classname, $exp_raw_desc_id, 1);
	ksort ($selectCols); /* sort columns by position flag */

	$classInfo = NULL;
	$classInfo["name"] 	   = $classname;
	$classInfo["classhas"] = $class_tab_has;
	if ($class_tab_has)    $classInfo["parashow"] = 1 ; // TBD: make more intelligent

	$topt = NULL;
	$topt["mode"] = "left";
	
	$searchForm = new tSearch_form($tablename);
	$searchForm->tform_show( $sql, $selectCols, $classInfo, $access_id_has, $topt);
	echo "<br>&nbsp;\n";
	echo "</td></tr></table>\n";
	//echo "<hr size=1 noshade>";
	//echo "</body></html>\n";
}

