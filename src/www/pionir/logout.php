<?php
/**
 * logout of system
 * @package logout.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param   $reason ... logout-reason
			 - 1 ... not logged in, session timed out
 */
session_start(); 

require_once ('reqnormal.inc');
require_once ('o.USER_PREF.manage.inc');
require_once ("class.history.inc");
require_once ("o.proj.profile.inc");
require_once ("f.logcookie.inc"); 
require_once ("f.profilesub.inc");
require_once 'o.DB_USER.subs.inc';



class gLogoutGui {
    
    function __construct($s_product) {
        $this->s_product = $s_product;
    }
	
	function htmlHead() {
	
	
		$htmlPageLib = new gHtmlHead();
		$htmlPageLib->_PageHeadStart ($this->s_product['product.name'].' Logout');
	
		?>
		<style type="text/css">

		.center {
			margin: auto;
	    	width: 400px;
	    	text-align: center;    
			}
			
		.imgback {
		    background-image:url(images/login_2018.jpg); 
		    background-repeat: no-repeat; 
			background-size: cover;
			opacity:0.3; 
			position:absolute; 
			top:0; 
			bottom:0; 
			right:0; 
			left:0;
			z-index:-1;
		}
		
		.submitButton {
	    	font-size: 16px; 
			cursor:pointer;
			width: 265px;
			padding:8px; /* padding:5px 25px; */
			background:#3366FF;
			color:#ffffff;
			border:1px solid #3030DD; 
			box-shadow: 0 0 4px rgba(0,0,0, .75);
			border-radius: 5px;
			
	    }
	    
	    </style>
	    <?php
		
	    $endopt=array("noBody"=>1);
		$htmlPageLib->_PageHeadEnd($endopt);
		?>
		<body style="color:#ffffff; 
			font-size:12px; font-family:Arial;">
		<div class="imgback"></div>
		<?php 
		
	}

	function showHeader2() {
		?>
		<table width="100%"cellspacing=0 cellpadding=0 border=0>
		<tr>
		<td nowrap align=center >
		<span style="font-size:1.2em;"><b><?php echo $this->s_product['product.name'];?> - Logout</span>
		</td>
		
		</tr>
		</table>
		<div class="center">
		<br><br><br>
		<?php 
		
	}

}

include ('../../config/config.product.inc'); // get $s_product

$error   = & ErrorHandler::get();

$reason = $_REQUEST["reason"];

$mainlib = new gLogoutGui($s_product);
$mainlib->htmlHead();
$mainlib->showHeader2();

$htmlExtraout = "";
if ( glob_loggedin() ) {
    
  $sql = logon2( $_SERVER['PHP_SELF'] );
  if ($error->printLast()) return;
   
  
  // save last touched project
  $hist_obj = new historyc();
  $lastproj = $hist_obj->last_bo_get( "PROJ" );
  $_SESSION['userGlob']["o.PROJ.last"]=$lastproj;
  $profileObj = new profileSubC();
  $profileObj->lastObjectsSave( $sql );
  
  $prefLib = new oUSER_PREF_manage();
  $prefLib->savePrefsNew( $sql );
  
  DB_userC::user_logout_save($sql, $_SESSION['sec']['db_user_id']);
  
  
} 

$tmp_db_index = ""; 
if (isset($_SESSION['s_sessVars']["g.db_index"])) {
     $tmp_db_index  = "?dbid=".$_SESSION['s_sessVars']["g.db_index"];
	 $save_db_index = $_SESSION['s_sessVars']["g.db_index"];
}

session_destroy();

if ($reason!="") {
	echo "<b>Info: You were logged out.</b><br>\nReason: ";
	if ($reason=="sessout") {
		echo "may be your session was timed out.";
	} 
	echo "\n";
} else {
	echo "<br><br>";
}
echo "<br><br><br>\n";

$logCookieObj = new logCookieC();
if ( $logCookieObj->cookieExists($save_db_index) ) {
    echo "<a href=\"index.php?logCookieDel=1&dbid=".$save_db_index."\"><img src=\"images/but13.del.gif\" border=0 TITLE=\"logout\"> Remove login information from your computer (cookie)</a><br>\n";
} else echo "<br>\n";

if ( $htmlExtraout !="" ) {
	echo $htmlExtraout;
}
?>
<BR>

<a href="index.php<?php echo $tmp_db_index?>" class=submitButton>&nbsp;&nbsp;&nbsp;Login again&nbsp;&nbsp;&nbsp;</a><P>


</div>
</body>
</html>