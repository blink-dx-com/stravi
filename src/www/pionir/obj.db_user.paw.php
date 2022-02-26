<?php
/**
 * GUI to change user password
 * 
 * - warn the user, if external authentication is active
 * - user can set password, if ONE OF THE FOLLOWING features occure:
 *   - is the same user
 *   - has role righr o.DB_USER=write
 *   - is ADMIN
 *   
 * - TBD: check for global LDAP-settings !!! , than it has no relevance to login
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param  <pre>
 *   $user_id
   	 $go : 0,1
	 $password1
	 $password1
	</pre>
 */

session_start(); 


require_once ('reqnormal.inc');
require_once ('func_form.inc');
require_once ("logincheck.inc");
require_once ('f.msgboxes.inc');
require_once ( "javascript.inc" );
require_once 'o.DB_USER.subs.inc';

class oDB_USER_paw {

    function __construct(&$sql, $user_id) {
    	$this->user_id=$user_id;
    }
    
    function check1( &$sql ) {
    	global $error;
    	$FUNCNAME= "check1";
    	
    	$tablename='DB_USER';
    	
    	$err_text    = "";
    	$user_id = $this->user_id;
    	$updateAllow = 0;   /*
    			= 1 : allowed
    			= 0 : not allowed for normal user
    			= -1: not possible, even for root
    			*/
    	
    	echo "<br><ul>\n";
    	
    	do {
    	
    		if ($_SESSION['sec']['db_user_id'] == $user_id) {
    			$updateAllow = 1;
    		}
    		$t_rights = tableAccessCheck( $sql, $tablename );
    		if ( $t_rights["write"] == 1 ) {
    		    $updateAllow = 1; // user has role for update ...
    		}
    		
    		
    		
    		//$contact_name="     ---       ";
    		$sqls = "select nick, contact_id, shown_email, notes from DB_USER  where DB_USER_ID=".$user_id;
    		$sql->query($sqls);
    		if ( $sql->readRow() )  {
    			$this->userFeat["nick"] 			= $sql->RowData[0];
    			$this->userFeat["contact_id_ini"]   = $sql->RowData[1];
    			$this->userFeat["shown_email_ini"]  = $sql->RowData[2];
    			$this->userFeat["ini_notes"]        = $sql->RowData[3];
    			
    			if ($this->userFeat["contact_id_ini"] ) {
    				$ret = $sql->query("select name from contact where CONTACT_ID=".$this->userFeat["contact_id_ini"] );
    				$sql->readRow();
    				$this->userFeat["contact_name"] = $sql->RowData[0]; 
    			}
    		} else {
    			$err_text = "user ID:$user_id not known.";
    			$updateAllow = -1;
    			break;
    		}
    		
    		$tmpallow = role_check_f ( $sql, "g.appPrefSave");
    		if ( $tmpallow == "deny" ) {                  
    			$updateAllow = 0;
    			$err_text = "due to your personal role-right 'g.appPrefSave'=DENY.";
    			break;
    		} 
    		
    		
    		
    	} while (0);
    	
    	if ( $_SESSION['sec']['appuser']=="root" AND $updateAllow>=0 ) $updateAllow = 1;
    	
    	$loginLib = new fLoginC();
    	$loginmeth = $loginLib->checkLoginMeth( $sql, $this->userFeat["nick"] );
    	if ($loginmeth!="NORM") {
    		echo "<br>";
    		htmlInfoBox( "Authentication warning", 
    				  "Changing the password has no relevance for login into this system. ".
    				  "The system is using '".$loginmeth."' for authentication.<br />".
    				  "You have to change the password on your authentication-server ".
    				  " or get an exception (ask your admin) to use this password.", "", "WARN" );
    		$updateAllow = -1;
    		$err_text = 'external authentication';
    		echo "<br>";
    	} 
    	
    	if ( $updateAllow<=0 ) {
    		$error->set( $FUNCNAME, 1, "Changing the password of user '".$this->userFeat["nick"].
    			"' not allowed for you. <B>Reason: </B>".$err_text);
    		return;
    	} 
    }
    
    function formpass(&$sql) {
    	
    	
    	$user_id = $this->user_id;
    	
    	require_once ('func_form.inc');
    	
    	$initarr   = NULL;
    	$initarr["action"]      = $_SERVER['PHP_SELF'];
    	$initarr["title"]       = "Change password";
    	$initarr["submittitle"] = "Submit";
    	$initarr["tabwidth"]    = "AUTO";
    	
    	$hiddenarr = NULL;
    	$hiddenarr["user_id"]     = $user_id;
    	
    	$formobj = new formc($initarr, $hiddenarr, 0);
    	
    	$fieldx = array (
    	    "title" => "User",
    	    "object"=> "text",
    	    "val"   => $this->userFeat["nick"],
    	    "view"  =>1
    	);
    	$formobj->fieldOut( $fieldx );
    	
    	$fieldx = array (
    	    "title" => "Password",
    	    "name"  => "password1",
    	    "object"=> "password",
    	    "namex" => TRUE 
    	);
    	$formobj->fieldOut( $fieldx );
    	
    	$fieldx = array (
    	    "title" => "Retype Password",
    	    "name"  => "password2",
    	    "object"=> "password",
    	    "namex" => TRUE 
    	);
    	$formobj->fieldOut( $fieldx );
    	
    	$formobj->close( TRUE );
    	
    	
    }
    
 
    function updatePass( &$sqlo, $password1, $password2 ) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        if ( $password1 != $password2 ) {
            $error->set( $FUNCNAME, 2, "Mismatch between password and retyped password!");
            return;
        }
        $user_id = $this->user_id;
        DB_userC::password_set($sqlo, $user_id, $password1);
    }

}

$title = "Change password";
global $error, $varcol;

$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] ); // URL-link for the first db-login; e.g. for object ($_SERVER['PHP_SELF']."?id=".$id)
if ($error->printLast()) htmlFoot();
$varcol = & Varcols::get();

$user_id = $_REQUEST['user_id'];
$go = $_REQUEST['go'];
$password1 = $_REQUEST['password1'];
$password2 = $_REQUEST['password2'];

$tmp_back_url = "edit.tmpl.php?tablename=DB_USER&id=".$user_id;
$tmp_back_txt = "user";
if ($user_id == $_SESSION['sec']['db_user_id']) { // show if it is your own account ...
	$tmp_back_url  = "obj.db_user.settings.php";
	$tmp_back_txt =  "MyAccount";
}

$infoarr			 = NULL;
$infoarr["scriptID"] = "";
$infoarr["title"]    = $title;
$infoarr["form_type"]= "tool";
$infoarr["back_url"] = $tmp_back_url;
$infoarr["back_txt"] = "user";
$infoarr["icon"]     = "images/ic24.userprefs.png";
$infoarr["locrow"]   = array( array($tmp_back_url,$tmp_back_txt) );

$pagelib = new gHtmlHead();
$pagelib->_PageHead ( $infoarr["title"],  $infoarr );
$pagelib->_startBody( $sql, $infoarr );

if (!$user_id) {
    htmlFoot("ERROR", 'No User-ID given.');
}

$mainLib = new oDB_USER_paw($sql, $user_id);

$mainLib->check1( $sql );
if ( $error->printAllEasy() ) {
	htmlFoot("<hr>");
} 

if ( !$go ) {
  $mainLib->formpass($sql);
  htmlFoot("<hr>");
}


if ( $go>0) {
	$mainLib->updatePass( $sql, $password1, $password2 );
}

if ( $error->printAllEasy() ) {
	echo "<br><br>";
	$mainLib->formpass($sql);
	htmlFoot("<hr>");
} else {
    cMsgbox::showBox("ok", "Password changed."); 
    echo "<br>";
}


js__location_replace($tmp_back_url, "forward ...", NULL, 1000 );
  


htmlFoot();