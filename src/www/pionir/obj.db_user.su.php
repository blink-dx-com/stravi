<?php
/**
 * - only "root": change identity as $user_id
   - can only relogin, of the $db_index is set !!!
 * @package obj.db_user.su.php
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @param   $user_id
		 $cctpwd
 * @version0 2001-05-21 
 */ 
session_start(); 


require_once ("db_access.inc");
require_once ("globals.inc");
require_once ("func_head.inc");
require_once ("logincheck.inc");

class oDb_userChange {
	
function __construct(&$sql, $user_id) {
	
	$this->user_id=$user_id;
	
	if ($user_id) {
		$sqls = "select nick from DB_USER  where DB_USER_ID=".$user_id;
		$sql->query("$sqls");
		if ( $sql->readRow() )  {
			$this->new_user = $sql->RowData[0]; 
		} else {
			htmlFoot("Error", "User ID: $user_id not found.");
		}
	}
}

function login( &$sql, $cctpwd ) {
	global $error;
	$FUNCNAME = "login";
	
	$new_user = $this->new_user;
	
	if ( !$this->user_id ) {
		 $error->set( $FUNCNAME, 1, "Please select a user." );
		 return;
	}
	
	if ( $new_user=="root" ) {
		 $error->set( $FUNCNAME, 1, "Relogin as user 'root' not allowed." );
		 return;
	}
	
	if ($cctpwd=="") {
		 $error->set( $FUNCNAME, 2, "Please give a password." );
		 return;
	}
	
	$loginLib = new fLoginC();
	$loginLib->rootLoginCheck( $sql, $cctpwd );
	if ($error->Got(READONLY))  {
    	$error->set( $FUNCNAME, 1, "Login-check failed." );
		return;
	}
	// take a from to put the in a POST-variable the password
	echo "<form name=\"editform\" method=\"POST\" action=\"main.php\" target=_top>\n";
	echo "<input type=\"hidden\" name=\"su_cctuser\" value=\"".$new_user."\" >\n";
	echo "<input type=\"hidden\" name=\"cctpwd\"     value=\"".$cctpwd."\"  >\n";
	if ( isset($_SESSION['s_sessVars']["g.db_index"]) ) 
			echo "<input type=\"hidden\" name=\"db_index\"  value=\"".$_SESSION['s_sessVars']["g.db_index"]."\"  >\n";
	echo "</form>\n";
	
	session_destroy();  // froget everything about "root"
	
	?>
	<script language="JavaScript">
	<!-- 
		document.editform.submit();   
	//-->
	</script>
	<?php 
	
	echo "</body>\n</html>\n";
	
    
}


function form2() {
	require_once ('func_form.inc');
	
	
	$user_id = $this->user_id; 
	$new_user= $this->new_user;
	
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Login as other user";
	$initarr["submittitle"] = "Login";
	$initarr["tabwidth"]    = "AUTO";
	$initarr["dblink"]	    = 1;
	
	$hiddenarr = NULL;

	$formobj = new formc($initarr, $hiddenarr, 0);

	$fieldx = array ( 
			"title" => "Login as user <img src=\"images/icon.DB_USER.gif\">", 
			"name"  => "user_id",
  			"namex" => TRUE,
			"object"=> "info2",
			"val"   => $user_id, 
			"notes" => NULL
			);
	
	if (!$user_id) {
		// select from table
		$fieldx["object"] = "dblink";
		$fieldx["inits"]  = array( "table"=>"DB_USER", "objname"=>"-- select --", "pos" =>"0");	
		
	} else {
		$fieldx["inits"] = "<font color=green><B>$new_user</B></font>";
	}
	
	$formobj->fieldOut( $fieldx );
	
	$fieldx = array ( 
		"title" => "Password for <b>root</b>", 
		"name"  => "cctpwd",
  		"namex" => TRUE,
		"object"=> "password",
		"val"   => NULL, 
		"notes" => NULL
		 );
	
	
	$formobj->fieldOut( $fieldx );
	
	$formobj->close( TRUE );
}

}



$error = & ErrorHandler::get();
$sql   = logon2( $_SERVER['PHP_SELF'] );

$user_id= $_REQUEST["user_id"];
$cctpwd= $_REQUEST["cctpwd"];
$go= $_REQUEST["go"];

$tablename = "DB_USER";
// $tmp_back_url="edit.tmpl.php?t=DB_USER&id=".$user_id;

$title = "Change identity";
$infoarr			 = NULL;
$infoarr["title"]    = $title;
$infoarr["form_type"]= "obj"; // "tool", "list"
$infoarr["obj_name"] = $tablename;
$infoarr["obj_id"]   = $_REQUEST["user_id"];

$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);

$mainlib = new oDb_userChange($sql, $user_id);
echo "<i>Info: Allow user 'root' to login as an other user, by using the password of user 'root'.</i>";
echo "<ul>\n";
echo "<br>\n";
if ($_SESSION['sec']['appuser'] != "root") {
	htmlFoot("Error","You must be user 'root' !" );
} 

if ( $_SESSION['userGlob']["g.debugLevel"]>1 ) {
    echo "<B>DEBUG:</B> Test, if script was submited: ".$go."<br>\n";
}
 

if ( $go ) {
	
	$mainlib->login( $sql, $cctpwd);
	if ( !$error->printAllEasy() )  {
    	htmlFoot();
		exit; // o.k. forward
	}
	echo "<br>";
}

$mainlib->form2();

echo "</ul>\n";
htmlFoot("<hr>");
