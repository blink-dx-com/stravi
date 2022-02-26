<?
/**
 * show logins of users
 * @package o.DB_USER.loggedin.php
 * @author  qbi
 * @param int $go=0/1
 */
extract($_REQUEST); 
session_start(); 

  
require_once ("reqnormal.inc");
require_once ('date_funcs.inc');

function oDB_USER_showloggedin( &$sql ) {
    
    $timeNow  = time();
    $timeBack = $timeNow - 10 * 3600;
    
    
    $sqlcmd = 'SELECT db_user_id, nick, '.$sql->Sql2DateString('login_last').' login_last, '.$sql->Sql2DateString('logout_last').' logout_last'.
        ' FROM db_user WHERE ((login_last > logout_last) OR (logout_last IS NULL)) ' .
        '  AND (login_last > '.$sql->Timestamp2Sql($timeBack).')'.
        ' ORDER BY login_last DESC'; // select logged in users since midnight of the current day
    $sql->query($sqlcmd);
    $cnt=0;
    echo "<table cellpadding=0 cellspacing=0 border=0>\n";
    while ( $sql->ReadRow() ) {
        $user_id  = $sql->RowData[0];
        $nick	  = $sql->RowData[1];
        $logged_in= $sql->RowData[2];
        $lener    = strlen ($logged_in);
        // $logged_in=substr($logged_in, 11, $lener-8);
        echo "<tr><td>&nbsp;&nbsp;- ".$nick."</td><td>&nbsp;$logged_in</td>\n";
        $cnt++;
    }
    echo "</table>\n";
    
    echo $cnt. ' users logged in.<br />';
    
}


$sql = logon2( $_SERVER['PHP_SELF'] );

$title = "Users which are logged in";
$infoarr			 = NULL;
$infoarr["scriptID"] = "script.php";
$infoarr["title"]    = $title;
$infoarr["form_type"]= "tool"; // "tool", "list"
$infoarr["locrow"]   = array( array("rootFuncs.php", "Administration") );

$pagelib = new gHtmlHead();
$headarr = $pagelib->startPage($sql, $infoarr);

echo "<P>";
echo "<B>List of users which are logged in:</B><P>";

if ( !$_SESSION['s_suflag'] && ( $_SESSION['sec']['appuser']!="root" ) ) {
	echo "Sorry, you must be root or have su-flag.";
	return 0;
}


$retval = oDB_USER_showloggedin( $sql );
echo "<br>...done";

htmlFoot();