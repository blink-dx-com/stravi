<?
/**
 * Collection of main utility functions.
 * @package module.inc
 * @swreq UREQ:xxxxxxxxxxxx
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param   string INPUT
 * @version $Header: trunk/src/phplib/utilities.inc 59 2018-11-21 09:04:09Z $
 */



/**
 * try to relogin
   bring the guest to logout, to give him a info-message
 * @param string $relpath e.g. "../pionir/"
 * @param string $reason  e.g. "sessout"
 */
function glob_relogin( 
	$relpath=NULL, 
	$reason=NULL   
	) {
	
	
	echo '<body>';
	echo '<html>';
	echo "... forward page. Reason: ".$reason;
	echo '<script language="JavaScript">'."\n";
	echo '<!--'."\n";
	
	// echo 'if ( parent.oben != null ) { '."\n"; // check if exists
	echo '  parent.location.href="'.$relpath.'logout.php?reason='.$reason.'";'."\n";
	//echo '}'."\n";
	
	echo ' //-->'."\n";
	echo '</script>'."\n";
	exit;
}

function is_partisan_open ()
{ ## Check, wether a user has already logged-in to Partisan
  ## within the current PHP session. Upon successful login into
  ## Partisan, several session variables are defined and set.

  if ( ! session_id() )                   { return 0; }
	
  if ( empty($_SESSION['sec']) )   { return 0; }
  if ( ! isset($_SESSION['sec']['dbuser']) )   { return 0; }
  if ( ! isset($_SESSION['sec']['appuser']) )   { return 0; }
  if ( ! isset($_SESSION['sec']['db_user_id']) ){ return 0; }
  if ( ! isset($_SESSION['sec']['passwd']) )	 { return 0; }
  if ( ! isset($_SESSION['sec']['db']) )		 { return 0; }

  if ( ! isset($_SESSION['globals']['DBVersion']) ) { return 0; }

  return 1;

}##End function

########################################################################