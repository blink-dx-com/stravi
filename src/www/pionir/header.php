<?php
/**
 * Header-frame of the App
 * EXTENSIONS: 
 * - $_SESSION['globals']["htmlFrameTop.homeBut"]
 * - $_SESSION['s_product']["product.head.extratext"]; --- optional text for e.g. DEV-SYSTEM
 * $Header: trunk/src/www/pionir/header.php 59 2018-11-21 09:04:09Z $
 * @package header.php
 * @author  Steffen Kube
 */
//extract($_REQUEST); 
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
$sql 	= logon2( $_SERVER['PHP_SELF'] );
if ($error->got()) $error->printLast();

$mainLib = new gFrHeaderGui();


$headcss= "

input[type=text], select {
  height: 25px;
  padding: 2px 5px;
  display: inline-block;
  border: 0px;
  box-sizing: border-box;
}

.xwrapper {
    border:0px solid;
    display:inline-block;
    position:relative;
    margin: 0;
    float: left ;
}

.x_sea_settings {
    vertical-align:middle; 
    stroke: #FFFFFF; 
    margin:0; 
    padding-left:10px; 
    float:left;
}

.xbutton {
    position:absolute;
    right:0;
    top:0px;
    border:0;
    margin:3px;
}
.xinput {
    padding-right:40px; /* button padding */
}

.xTD_ref{
    padding-left:10px;
    padding-right:10px;
}

.xTD_ref:hover {
    background-color: #B0B0B0;
}

";
$fhopt  = array(
    "noBody" => 1, 
    'css'    => $headcss,
    'jsFile' => array(    
        'res/js_goz/g.ajax_search.js?dummy=13',
    ), 
);
$title  = "System Header";

$pagelib = new gHtmlHead();
$pagelib->_PageHead ($title, $fhopt);


echo '<body class="yBodyHead">'."\n";


     
$home_proj_id = oDB_USER_sub2::userHomeProjGet( $sql );

$globSearchForm=1;

if ($_SESSION['userGlob']["g.headquicks"]<0) {
	$globSearchForm=0; // deny, if a special value is given ...
}
if ($globSearchForm) {
    echo "<form method=\"post\"  name=\"editform\"  action=\"javascript:open_unten( 2 )\" >\n";
}	  
echo '<table width="100%" style="padding-top: 2px;">'."\n";
echo "<tr>";
?>
<td NOWRAP valign=middle align=left>
<span style="padding-left: 15px;"></span>
<?php
echo '<a href="'.$_SESSION['globals']["htmlFrameTop.homeBut"].'" target="unten" >';
?>
<img src="images/but.home.w.png" TITLE="my theme park" width=30></a>
<a href="edit.tmpl.php?t=PROJ&id=<?echo $home_proj_id?>" target="unten">
<img src="images/ic.proj.w.png" TITLE="my folder"  width=30></a>
<?php  // <a href="home_first.php" target="_blank" ><img src="images/but.newwindow.w.png" TITLE="new window" width=30></a>
?>
<a href="p.php?mod=DEF/g.obj.favact&act=show" target="unten" ><img src="images/but.heart.svg" TITLE="my favorites" hspace=1></a>
<a href="javascript:open_help('help/robo/start.html')"><img src="images/ic.docu.w.png" TITLE="help" style="padding-left: 20px;" border=0 width=30 ></a>

<span style="padding-left: 75px;"></span>
<?php

$log    = & SQL_log::getHandle();
$errObj = $log->gotError();
$icon   = "images/but.sql_log.gif";
$URL    = "";




echo '</td>';
// <td bgcolor=#D0D0D0 width=25 valign=top><img src="images/blue.bow_rig.gif?d=1" width=25></td>

echo "<td width=100% nowrap>";

if ($globSearchForm) { 
    //echo "&nbsp;&nbsp;";
    echo '<div style="margin:0;">'."\n";
    echo ' <div class="xwrapper">';
    $mainLib->_showForm($sql);

    echo '<input id="txt_search_list" name=idx list="search_list" type="text" size="30" autoComplete="off" onkeyup="showResult(this.value)" size=20 class=xinput placeholder=" Name or ID">';
    echo '<datalist id="search_list"></datalist>'."\n";
    // echo '<input type=text name=idx value="" size=20 class=xinput placeholder=" Name or ID" />';
    echo '<input type="image" src="images/ic.33.search.png"  width="20" height="20" name="quick" title="Search" onclick="open_unten( 2 )" class=xbutton>';
    echo '</div>'."\n";
    
    //echo " <input type=text name=idx value=\"\" size=20> ";
    //echo "<input type=button name='quick' value='Search' onclick=\"open_unten( 2 )\">\n";
    echo '<div class="x_sea_settings">';
    
    //  need file_get_contents() to inherit the standard colors to the SVG
	echo '<a href="javascript:open_unten( 1 )" title="Advanced search">'.
	   	file_get_contents('res/img/settings.svg')  .  '</a>'."\n";
	echo ' </div>'."\n";
	echo '</div>'."\n";
	
}  else echo "&nbsp;";

echo '<span id="headerDebug" style="color:#EFEFEF"></span>';
echo "</td>\n";

echo '<td NOWRAP valign=middle>';
if (isset($_SESSION["userGlob"]["g.sql_logging"])
    && $_SESSION["userGlob"]["g.sql_logging"] == 1) { // logging switched on
        if ($errObj === false) {
            $log_info = $log->_getLogFileName();
            $filex=$log_info['file'];
            $SQL_URL = 'f_workfile_down.php?file='.$filex;
        }
        ?>
	<B><a href="<?php echo $SQL_URL?>" target="_sql_log">
	<img src="<?php echo $icon?>" TITLE="SQL logging window" border=0 hspace=5></a></B>
	<?php
}##End if
if (!empty($_SESSION['userGlob']["g.debugLevel"])) { 
  echo "<span style='color:yellow;'>Debug: ".$_SESSION['userGlob']["g.debugLevel"]."</span>&nbsp;&nbsp;";
}

echo $_SESSION['s_product']["product.head.extratext"]; // optional text for e.g. DEV-SYSTEM

echo '</td>'."\n";

$pageShowUser = 0;
if ( $_SESSION['userGlob']["g.headerUserIcon"]>0 ) { // need larger header-frame
	$pfilename = datadirC::datadir_filename( "DB_USER", $_SESSION['sec']['db_user_id'] ,"jpg" );
    if ( file_exists($pfilename) ) $pageShowUser=1;
}

$user_nick = $_SESSION['sec']['appuser'];
$dbid      = $_SESSION['sec']['dbid'];
$user_link =  '<a href="obj.db_user.settings.php" target="unten" title="User settings for '.$user_nick.'@'.$dbid.'">';

if ($pageShowUser) {
    echo "<td >".$user_link;
	echo "<img src=\"glob.obj.img_show.php?tablename=DB_USER&primid=".$_SESSION['sec']['db_user_id']."&extension=jpg\" height=50 hspace=\"5\">\n";
	echo "</td><td valign=middle NOWRAP >";
}  else {
    echo "<td class=xTD_ref  align=right valign=middle NOWRAP>&nbsp;".$user_link;
	echo '<img src="images/ic.user.w.png" TITLE="user settings" width=20>&nbsp;';
}


echo $user_nick;
if (!empty($_SESSION['userGlob']["g.headerloginfo"])) echo "@".$dbid;
echo "</a>";

?>
</td><td class=xTD_ref valign=middle> 
<a href="logout.php" target="_top" ><img src="images/ic.logout.png" width=20 height=20 title="logout"></a></td>

<?


//echo '<td  NOWRAP>';
//echo '<a href="'.$_SESSION['s_product']["company.URL"].'" target="_new"><img src="images/0.gif" width=5 height=1 border=0>';
//echo '<img SRC="images/'.$_SESSION['s_product']["product.icon"].'" border=0 title="System Icon"></a>';
//echo '<img src="images/0.gif" width=5 border=0></td>';

echo '<td NOWRAP>';
echo '<a href="'.$_SESSION['s_product']["company.URL"].'" target="_new">';
echo '<img SRC="images/'.$_SESSION['s_product']["company.icon"].'" TITLE="Company" style="margin-left:15px; margin-right:5px;"></a> &nbsp;</td>';
echo '</tr></table>';

if ($globSearchForm) { 
    echo "</form>\n";
}

?>                         
</body>
</html>
