<?php
/**
 * - show main frame-set of the pionir application 
 * - if called: prevent, to show a frame-set inside an application
 * @package main.fr.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param  $browserInfo
     optional:    
   $tablename + $id      
   $sNoFrame   : 0|1 prevent frame-set?
   $forwardUrl : encoded forward-URL

 */
session_start(); 
require_once ('globals.inc');

class page_sub_C {
	/**
	 * show mobile frame-set
	 * @param $tmpHeadInfo
	 * @return -
	 */
    function showMobile($homesrc, $tmpHeadInfo) {
		
	    // OLD: $homesrc='home.mobile.php';
	    // OLD: <frame src="header.mobile.php" ...

		?>
		<frameset rows="42,*" border=1 frameborder=0 framespacing=0>    
	    <frame src="header.php?info_user=<?echo $_SESSION['sec']['appuser'].$tmpHeadInfo; ?>" name="oben" scrolling=no marginwidth=0 marginheight=0>
		<frame src="<?echo $homesrc?>" name="unten" marginwidth=0 marginheight=0>

		</frameset> 
		<?php
	}
	
	/**
	 * show normal frame-set
	 * @param $tmpHeadInfo
	 * @return -
	 */
	function showNormal($homesrc,$tmpHeadInfo) {
		
		
		$tmpFrameLwidth = "250";
		$tmpLeftMode 	= "";
		$tmprows  = 42; // 30
		$isNS4	  = 0;
		if ($_SESSION['s_product']["frameLeftWidth"]!="") $tmpFrameLwidth=$_SESSION['s_product']["frameLeftWidth"];
		if ($_SESSION['s_product']["frameLeftMode"]!="")  $tmpLeftMode = "?mode=".$_SESSION['s_product']["frameLeftMode"];
		
		
		
		if ( $_SESSION['userGlob']["g.headerUserIcon"]>0 ) { // need larger header-frame
			$tmprows = 50;
		}

		
		?>
	<frameset rows="<?echo $tmprows?>,*" border=1 frameborder=0 framespacing=0>
    
    <frame src="header.php?info_user=<?echo $_SESSION['sec']['appuser'].$tmpHeadInfo; ?>" name="oben" scrolling=no marginwidth=0 marginheight=0>
    <frameset cols="<?echo $tmpFrameLwidth?>,*" border=1 frameborder=1 framespacing=0>
		<frame src="frame.left.nav.php<?echo $tmpLeftMode?>" name="left" marginwidth=0 marginheight=0>
		<frame src="<?echo $homesrc?>" name="unten" marginwidth=0 marginheight=0>
		
	</frameset> 
	</frameset> 
		<?
	}
}


$browserInfo = $_REQUEST["browserInfo"];
$tablename  = $_REQUEST["tablename"];
$id = $_REQUEST["id"];
$sNoFrame  = $_REQUEST["sNoFrame"];
$forwardUrl = $_REQUEST["forwardUrl"];


$mainLib = new page_sub_C();
$titletmp = ""; 


if ( $_SESSION['userGlob']["g.headerloginfo"] ) { 
    $titletmp = " ".$_SESSION['sec']['appuser']."@".$_SESSION['sec']['dbid'];
}
if ($browserInfo !="") {
     $_SESSION['s_sessVars']["g.browserInfo"] = $browserInfo;
}
$db_tmp = $_SESSION['sec']['dbid'];


if ($sNoFrame>0) {
	echo "<!DOCTYPE HTML PUBLIC>\n";
	echo "<html>\n";
	echo "<head>\n";
} else {
	echo "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Frameset//EN\" \"http://www.w3.org/TR/html4/frameset.dtd\">\n";
	echo "<html>\n";
	echo "<head>\n";
}
echo " <title>".$_SESSION['s_product']["product.name"].$titletmp."</title>\n";

/*- call dummy page, due to DHTML at Netscape (it does not show LAYERS at 
      generation of a new window/frame)
  - TBD: remove this pre-call to home_first.php, beacause no support for old browsers anymore
  - home_first.php calls home.php then ...
*/
$homesrc="home_first.php?"; 

if ( $tablename!="" ) {
    $homesrc="edit.tmpl.php?t=".$tablename."&id=".$id;
}

if ( $forwardUrl!="" ) {
	$homesrc = urldecode($forwardUrl);
} 

// give $info_user and $info_db to "header.php" as link parameters to make the user_nick available in the apache log file 
$tmpHeadInfo = "&info_db=".$_SESSION['sec']['dbuser'].".". $db_tmp;
$homesrcEnc  = urlencode($homesrc);

if ($sNoFrame>0) {
	echo "<meta http-equiv=\"expires\" content=\"0\">\n";
	echo "</head>\n";
	echo "<body><a href=\"".$homesrc."\">... continue with this page ...</a>";
	?>
	<script><!--
    	location.href="<?echo $homesrc?>"; 
	//-->
	</script>	
	<?
	echo "</body>";
	return;
}

// reload the whole frame, if somebody tried to call the page inside a frame ...
?>
<meta http-equiv="expires" content="0">
<script><!--
   if (parent.frames.length != 0) {
      parent.location.replace("main.fr.php?forwardUrl=<?echo $homesrcEnc?>");
      exit;
   }
//--></script>	
</head>
<?

if ($_SESSION['s_sessVars']["g.surfMode"]=='mobile') {
    $mainLib->showMobile($homesrc, $tmpHeadInfo);
} else {
	$mainLib->showNormal($homesrc, $tmpHeadInfo);
}

?>
</body>
</html>
