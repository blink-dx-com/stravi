<?php
########################################################################
# $Header: trunk/src/www/pionir/obj.db_user.prefs_save.php 59 2018-11-21 09:04:09Z $
########################################################################
/*MODULE: obj.db_user.prefs_save.php
  DESCR: save preferences
  AUTHOR: Qbi
  VERSION: 0.1 - 20030226   
*/
 
session_start(); 

                            
require_once ("reqnormal.inc");
require_once ('o.USER_PREF.manage.inc');
require_once ("role.inc");
require_once ("f.profilesub.inc"); 
require_once ( "javascript.inc" );

$sql = logon2( $_SERVER['PHP_SELF'] );

$backurl = "obj.db_user.settings.php";
$title= "Save preferences";
$infoarr			 = NULL;
$infoarr["scriptID"] = "";
$infoarr["title"]    = $title;
$infoarr["form_type"]= "obj"; // "tool", "list"
$infoarr["locrow"]   = array( array($backurl, "MyAccount") );


$pagelib = new gHtmlHead();
$headarr = $pagelib->startPage($sqlo, $infoarr);


$tmpallow = role_check_f ( $sql, "g.appPrefSave");
if ( $tmpallow == "deny" ) {                  
    $err_text = "due to your personal role-right 'g.appPrefSave'=DENY.";
    $deturl = "<center><br><br>[<a href=\"".$backurl."\">&lt;&lt; Back to user settings</a>]</center>";
    htmlFoot( "INFO", "Save of the user profile not allowed.", "<B>Reason: </B>".$err_text . $deturl);
} 

$profileObj = new profileSubC();
$profileObj->lastObjectsSave( $sql );

$prefLib = new oUSER_PREF_manage();
$prefLib->savePrefsNew( $sql );

echo ".. save last objects ...<br>";





echo "... done<br><br>";
js__location_replace($backurl, "back");
		
?>

</body>
</html>
