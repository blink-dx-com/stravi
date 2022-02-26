<?php
/**
 * mobile Header-page of the App
 * @package header.mobile.php
 * @author  Steffen Kube
 */ 
session_start(); 


require_once ("reqnormal.inc");
require_once("o.DB_USER.subs2.inc");
require_once("main.nav.inc");
require_once("f.data_dir.inc");
require_once("gui/go.qSearchGui.inc");

class gFrHeaderGui {
	
function _showForm(&$sqlo) {
	$formLib = new go_qSearchGui();
	$formLib->showTableSearchField($sqlo);
}


}

$error 	= & ErrorHandler::get();
$mainLib = new gFrHeaderGui();

$backcolor = "#18436E";
$headcss= "
.aBright a:link    { color:#C0F0FF; }
.aBright a:visited { color:#C0F0FF; }
.aBright a:active  { color:#C0F0FF; }";
$fhopt  = array("noBody" => 1, 'css'=>$headcss);
$title  = "Partisan Header";

$pagelib = new gHtmlHead();
$pagelib->_PageHead ($title, $fhopt);


?>
<script language="JavaScript">
<!--
    function open_help( url )   {				
		
		InfoWin = window.open( url, "help","scrollbars=yes,width=950,height=500,resizable=yes"); 
		InfoWin.focus();				
    }
    function open_unten( mode ) {
        id  = document.editform.idx.value; 
        tab = document.editform.tablename.options[document.editform.tablename.options.selectedIndex].value;
        parent.unten.location.href="glob.obj.qsearch.php?idx=" + escape(id) + "&go=" + mode + "&tablename="+tab;
    }
           
//-->
</script> 
<body class="yBodyHead">
<?php


$sql 	= logon2( $_SERVER['PHP_SELF'] );
if ($error->got()) $error->printLast();
     
$home_proj_id = oDB_USER_sub2::userHomeProjGet( $sql );


echo "<form method=\"post\"  name=\"editform\"  action=\"javascript:open_unten( 2 )\" >\n";
 
echo "<table width=\"100%\" cellpadding=0 cellspacing=0 border=0>\n";
echo "<tr>";

// bgcolor=#D0D0D0
?>
<td NOWRAP valign=middle align=left >
<B><a href="edit.tmpl.php?tablename=PROJ&idname=PROJ_ID&id=<?echo $home_proj_id?>" target="unten">
<img src="images/ic.proj.w.png" width=30 TITLE="home project" border=0 hspace=0></a></B>
<?php
echo '<a href="home.mobile.php" target="unten" >';
?>
<img src="images/but.home.w.png" width=30 TITLE="theme park" border=0 hspace=0></a>

<?

$log    = & SQL_log::getHandle();
$errObj = $log->gotError();
$URL    = "";

echo '</td>';
echo '<td align=right valign=top><img src=0.gif width=25 height=1></td>'."\n";
// <td bgcolor=#D0D0D0 width=25 valign=top><img src="images/blue.bow_rig.gif?d=1" width=25></td>

echo "<td width=100% nowrap>";


echo "&nbsp;&nbsp;";
$mainLib->_showForm($sql);
echo " <input type=text name=idx value=\"\" size=20> ";
echo "<input type=button name='quick' value='Search' onclick=\"open_unten( 2 )\">\n";

echo "</td>";
echo '<td align=right valign=top ><img src=0.gif width=25 height=1></td>'."\n";
echo "<td align=right valign=middle NOWRAP >&nbsp;"; // bgcolor=#D0D0D0
echo "<img src=\"images/ic.user.w.png\" border=0 TITLE=\"user\"> ";
echo "<a href=\"obj.db_user.settings.php\" target=unten>".$_SESSION['sec']['appuser']."</a>";

  
?>
&nbsp; &nbsp;
<a href="logout.php" target="_top" ><B>logout</B></a>&nbsp;</td>
</tr></table>
<?
echo "</form>";
?>
</body>
</html>
