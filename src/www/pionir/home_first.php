<?php
/**
 * decide, which page after start will be called
 * home_first.php calls home.php then ...
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
session_start(); 

require_once ('reqnormal.inc');
require_once("o.DB_USER.subs2.inc"); 

global $error;
$error = & ErrorHandler::get();

$sqlo  = logon2( $_SERVER['PHP_SELF'] );
if ($error->printLast()) htmlFoot();


$title		= 'home first';
$infoarr			 = NULL;
$infoarr['scriptID'] = '';
$infoarr['title']    = $title;
$infoarr['form_type']= 'tool';
$infoarr['design']   = 'norm';

$pagelib = new gHtmlHead();
$pagelib-> startPageLight( $sqlo, $infoarr);
                
    
$firstpage = "home.php"; 
$firstpage_tmp = $_SESSION['globals']["htmlFrameTop.homeBut"];
if ( $firstpage_tmp !="" ) $firstpage = $firstpage_tmp;

if ( $_SESSION['userGlob']["g.appStartPage"] == "project" ) { 
            
    $proj_id = oDB_USER_sub2::userHomeProjGet($sqlo);
    if ($proj_id) $firstpage = "edit.tmpl.php?tablename=PROJ&id=".$proj_id;
}

?>

<script>
<!--
 location.href='<?echo $firstpage?>';
//-->
</script>

<a href="<?echo $firstpage?>">automatic forward to 'home'</a>

<?php
$pagelib->htmlFoot();
