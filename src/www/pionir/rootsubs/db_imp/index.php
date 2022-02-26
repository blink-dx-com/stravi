<?php
/**
 * - import an ORACLE database dump
 * - can be called without normal LOHIN!
 * - 1. login as "system"
 * - 2. delete old DB-user
 * 
 * @package db_imp/index.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param $act <pre>
 *  "installLogin" : login to this tool + $password
   	"login" and go==1: $parx: 
  			 	"user"
			 	"pw"
	"olduser":  $parx: 
				oldUser : database-user / tablespace
	"deluser": delete now
	
		
		$_SESSION["s_formState"]:
		   		"oldUser"
				"newuser"
    </pre>
 * @global : $_SESSION["db_admin"] = array(
		"user"=>   "system",
		"pw"=>     ....,
		"db"=>     $db,
		"dbtype"=> $_dbtype
		 );
 */

extract($_REQUEST); 
session_start(); 


require_once ('func_head2.inc');
require_once ('g.dbAdminGui.inc');

class db_imp_gui {

var $act;
var $sessParams;
var $goArray;
var $status;
var $sysInfo;

function __construct($act) {

	$this->AdminLib = new dbAdminSub();
	$this->doImpLib = new db_imp_act();
	$this->dbAdminGui = new dbAdminGui();
	
	$this->doImpLib->setModiFlag(1);
	$this->doImpLib->setshowCmd(1);

	
	$this->act = $act;
	$this->sessParams = $_SESSION["s_formState"]["dbImpDump"] ;
	$this->confLib = new gConfInstSub();
	$this->confLib->loadFile();
	
	$this->goArray   = array( // "t"=>title, "go"=> show in navi,
	
		"0"=>array("t"=>"Home", "go"=>1),
		"login"=>  array("t"=> "DB-Login",   "go"=>1),
		"olduser"=>array("t"=> "Remove oldUser" , "go"=>1,),
		"askuser"=>array("t"=> "Confirm oldUser", "go"=>0),
		"deluser"=>array("t"=> "Drop oldUser", "go"=>0),
		"userNewf"=>array("t"=> "Give new DB_USER", "go"=>1),
		"tabForm"=> array("t"=> "Create a new table space", "go"=>1),
		"tabCrea"=> array("t"=> "Create a table space", "go"=>0),
		"tabDrop"=> array("t"=> "Drop a table space", "go"=>0),
		"userCrea"=>array("t"=> "Create a new USER", "go"=>1),
		"dbimpInp"=>array("t"=>"import DUMP", "go"=>1),
		"dbimp"   =>array("t"=>"import DUMP", "go"=>0),
		"dbSequence"=>array("t"=>"Reset Sequences", "go"=>1),
		"appDirSetInp"   =>array("t"=>"AppDir creation/settings", "go"=>0),
		"appDirSet"   =>array("t"=>"AppDir creation/settings", "go"=>1),
		"dbexp"       =>array("t"=>"export DUMP", "go"=>0),
		"userRootPW"  =>array("t"=>"reset password for root", "go"=>0),
		"userTableViews"   =>array("t"=>"create SYS-views for user", "go"=>0)
				);
	
	// $this->sessParams["goStat"][$key]>0 
	$this->status = array(
			"login"    =>array("t"=>"Login", "act"=>array("login") ),
			"newUser"  =>array("t"=>"New user", "act"=>array("userNewf", "olduser") ),
			"tablespace"=>array("t"=>"Table space", "act"=>array("tabForm", "tabDrop")),	
			"userExist"=>array("t"=>"User exists", "act"=>array("userCrea", "userRootPW", 'userTableViews')), 
			"impDump"  =>array("t"=>"Import Dump", "act"=>array("dbimpInp", "dbSequence")),
			"appData"  =>array("t"=>"Set App data", "act"=>array("appDirSetInp")),
			"expDump"  =>array("t"=>"Export Dump", "act"=>array("dbexp")), 
			);
			
	$this->fieldx = $this->dbAdminGui->getFormArr();
}

function &login( &$db_admin ) {	
	return ( $this->doImpLib->login( $db_admin ) );
}

function htmlFootX() {
	global $error;
	if ( !$error->printAll() ) {
		echo "<font color=green><b>o.k.</b></font><br>";
	}
	echo "<hr>";
	htmlFoot();
}

// set parameter
function setParam($key, $val, $subkey=NULL ) {
	if ( $subkey!=NULL ) {
		$this->sessParams[$key][$subkey] = $val;
	} else {
		$this->sessParams[$key] = $val;
	}
}

function setGoStatus($key, 
	$val // 0,1
	) {
	$this->setParam( "goStat", $val, $key );
	$this->saveSessParams();
}

function saveSessParams() {
	$_SESSION["s_formState"]["dbImpDump"] = $this->sessParams;
}

function checkLogin() {
	return $this->AdminLib->checkLogin();
}

function installCheckLogin() {
	return $this->AdminLib->installCheckLogin();
}


function sysInfo(&$sqlSys) {
	
	$tablesSpaceDir = $this->doImpLib->tableSpaceDirGet($sqlSys);
	$this->sysInfo["tablesSpaceDir"] = array("val"=>$tablesSpaceDir, "inf"=>"");
	
	// user-table-spaces
	$tabSpaces="";
	$komma="";
	$sqls = $this->doImpLib->getSpaceUsersSQL();
	$sqlSys->query($sqls);
	while ( $sqlSys->ReadRow() ) {
		$tabSpace = basename($sqlSys->RowData[0]);
		if ( strtoupper($tabSpace) == strtoupper($this->sessParams["newuser"]."_TAB.DBF") ) {
			$this->sysInfo["tableSpaceUser"] =  array("val"=>$tabSpace, "inf"=>"tablespace of user");
		}
		$tabSpaces .= $komma . $tabSpace. '('.$sqlSys->RowData[1].')';
		$komma="<br>\n";
	}
	$this->sysInfo["tableSpaces"] = array("val"=>$tabSpaces, "inf"=>"users tablespaces");
	$this->sysInfo["database"]    = array("val"=>$_SESSION["db_admin"]["db"], "inf"=>"");
	
	$roleCCT = $this->doImpLib->checkRole($sqlSys);
	
	$tmpinf = $roleCCT != "" ? "o.k." : "missing: <a href=\"".$_SERVER['PHP_SELF']."?act=creaRole\">Create CCT role</a>";
	$this->sysInfo["role_cct"] = array("val"=>$roleCCT, "inf"=>$tmpinf);

	if ($this->sysInfo["role_cct"]!="") {
		// get users
		$users="";
		$komma="";
		$sqls = $this->doImpLib->dbUserGetAllSQL();
		$sqlSys->query($sqls);
		while ( $sqlSys->ReadRow() ) {
			$user = $sqlSys->RowData[0];
			if ( $user==strtoupper($this->sessParams["newuser"]) ) {
				$this->sysInfo["new_user_exists"] = array("val"=>$user );
			}
			$users .= $komma . $user;
			$komma=", ";
		}
		$this->sysInfo["db_users"] = array("val"=>$users, "inf"=>"users with role CCT_USER");
	} 
	
	
}

function sysInfoShow() {
	
	if ( !$this->sysInfo ) return;
	
	$tabobj = new visufuncs();
	$headOpt = array( "title" => "Sys Info" );
	$headx  = array ("Key", "Value", "Info");
	$tabobj->table_head($headx,   $headOpt);
	
	foreach( $this->sysInfo as $key=>$val) {
		$dataArr=array($key, $val["val"], $val["inf"]);
		$tabobj->table_row ($dataArr);
	}
	reset($this->sysInfo);
	$tabobj->table_close();
	
	echo "<br>\n";
	 		  
}

function _infoOut($text, $prio) {
	echo "<font color=gray>Info:</font> ".$text.'</br>'."\n";
}

function _getStatusInfo( &$sql, $key ) {
	$info=NULL;
	switch ($key) {
		case "newUser":
			$info = $this->sessParams["newuser"];
			break;
		case "tablespace":
			$info = $this->sysInfo["tableSpaceUser"]["val"];
			break;
		case "userExist":
			$info = $this->sysInfo["new_user_exists"]["val"];
			break;
	}
	return ($info);
}

function shNavi() {
	
	$tabobj = new visufuncs();
	$headOpt = array( "title" => "Navigation"
					  );
	$headx  = array ("Key", "Status", "Info", "Action");
	

	$tabobj->table_head($headx,   $headOpt);
	
	foreach( $this->status as $key=>$val) {
		
		$status = "";
		if ( $this->sessParams["goStat"][$key]>0 ) $status = "o.k.";
		
		$infoTxt = $this->_getStatusInfo($sql,$key );
		
		// ACTION-LINK
		$actText="";
		foreach( $val["act"] as $dummy=>$actkey) {
			$titact = $this->goArray[$actkey]["t"];
			$actText .= " [<a href=\"".$_SERVER['PHP_SELF']."?act=".$actkey."\">".$titact."</a>]";
		}
		
		$dataArr = array( 
				$val["t"],
				$status,
				$infoTxt,
				$actText,
						
				);
		$tabobj->table_row ($dataArr);
		
	}	
	reset ($this->status); 
	
	$tabobj->table_close();
	echo "<br>";
}

function goInfo($act) { 
	
	if (!$act) $act=0;
	$info = $this->goArray[$act];
	
	echo "<font style=\"color: #606060; font-weight:bold; font-size:18px;\">".$info["t"]."</font>";
	
	echo "&nbsp;&nbsp;&nbsp;[<a href=\"".$_SERVER['PHP_SELF']."\">Home</a>]";
	echo "<br><br>";
	
}

function checkParams($params) {
	global $error;
	$info = NULL;
	$komma="";
	foreach( $params as $dummy=>$key) {
		if ($this->sessParams[$key]=="") {
			$info  .= $komma. $key;
			$komma=", ";
		}
	}	
	reset ($params); 
	
	if ($info!="") {
		$error->set( "checkParams", 1, "Parameters ".$info." missing!" );
		return -1;
	} 
	return 0;
}

function installFormLogin() {
	// installer-login
	
	
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Login as Installer (config-file: \$confvars[\"installpass\"] )";
	$initarr["submittitle"] = "Submit";
	$initarr["tabwidth"]    = "AUTO";

	$hiddenarr = NULL;
	$hiddenarr["act"] = "installLogin";

	$formobj = new formc($initarr, $hiddenarr, 0);

	
	$fieldx = array ( 
		"title" => "password", 
		"name"  => "pw",
		"object"=> "password", 
		"notes" => "password for installer"
		 );
	$formobj->fieldOut( $fieldx );

	$formobj->close( TRUE );
	
}

function g_installLogin($parx) {
	// check for the "installpass", set in config.local.inc
	global $error;
	$FUNCNAME = "g_installLogin";
	
	if ( $parx["pw"]=="" ) {
		$error->set( $FUNCNAME, 1, "Give user and password." );
		return;
	} 
	
	$install_admin =  $this->confLib->login($parx["pw"]);
	return $install_admin;
}

function g_DBformLogin() {
	// DB: user 'system' - login form
		
	
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Login as DB-admin";
	$initarr["submittitle"] = "Submit";
	$initarr["tabwidth"]    = "AUTO";

	$hiddenarr = NULL;
	$hiddenarr["act"] = "login";

	$formobj = new formc($initarr, $hiddenarr, 0);

	$fieldx = array ( 
		"title" => "user", 
		"name"  => "user",
		"object"=> "info2",
		"val"   => "system", 
		"notes" => "system-user"
		 );
	$formobj->fieldOut( $fieldx );
	
	$fieldx = array ( 
		"title" => "password", 
		"name"  => "pw",
		"object"=> "password", 
		"notes" => "password for system-user"
		 );
	$formobj->fieldOut( $fieldx );
	
	$fieldx = array ( 
		"title" => "database", 
		"name"  => "db",
		"object"=> "text", 
		"notes" => "database name"
		 );
	$formobj->fieldOut( $fieldx );

	$formobj->close( TRUE );
	
}

function &doLogin($parx) {
	$answer = $this->doImpLib->doLogin($parx);
	$this->setGoStatus( "login", 1);
	return ($answer);
}

function &doUserLogin() {
	$newuser = $this->sysInfo["new_user_exists"]["val"];
	$newpw   = $this->sessParams["newpw"];
	if ($newuser=="" or $newpw=="" ) return;

	$loginarr = array( 'user'=>$newuser, 'pw'=>$newpw, 'db'=> $this->sysInfo["database"]['val'] );
	$sqlo =  $this->doImpLib->userlogin($loginarr);
	
	return ($sqlo);
}

function g_oldUser() {
	/*
	FUNCTION: give "db_user"
	*/
	
	
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Give oldUser to be deleted";
	$initarr["submittitle"] = "Submit";
	$initarr["tabwidth"]    = "AUTO";

	$hiddenarr = NULL;
	$hiddenarr["act"] = "askuser";

	$formobj = new formc($initarr, $hiddenarr, 0);

	$fieldx = array ( 
		"title" => "oldUser", 
		"name"  => "oldUser",
		"object"=> "text",
		"val"   =>  $_SESSION['sec']['dbuser'],
		"notes" => "old User"
		 );
	$formobj->fieldOut( $fieldx );

	$formobj->close( TRUE );
}

function g2_anaDbUser($parx) {

	if ($parx["oldUser"]=="") {
		$error->set( $FUNCNAME, 1, "Give oldUser !" );
		return;
	} 
	
	
	
	$this->setParam("oldUser", $parx["oldUser"] );
	$this->saveSessParams();
	
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "DELETE the user '".$parx["oldUser"]."' and table-data?";
	$initarr["submittitle"] = "Delete oldUser now";
	$initarr["tabwidth"]    = "AUTO";

	$hiddenarr = NULL;
	$hiddenarr["act"] = "deluser";

	$formobj = new formc($initarr, $hiddenarr, 0);
	$formobj->close( TRUE );
	
}

function g_user3del(&$sqlSys, $parx) {
	// delete old user 
	if ($this->checkParams(array("oldUser"))<0 ) {
		return;
	}
	
	$olduser = $this->sessParams["oldUser"];
	$this->doImpLib->do_userDrop( $sqlSys, $olduser );
	
	$this->setGoStatus("oldUser",1);
	echo "o.k.";
}

function g_newUser1($parx, $go) {

	
	
	if ($go>0) {
		$this->setParam("newuser", $parx["newuser"]);
		$this->setParam("newpw", $parx["newpw"]);
		$this->saveSessParams();
		echo "new user '".$parx["newuser"]."' o.k.";
		
		$this->setGoStatus("newUser",1);
		return;
	} 
	
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Give the new user";
	$initarr["submittitle"] = "Submit";
	$initarr["tabwidth"]    = "AUTO";

	$hiddenarr = NULL;
	$hiddenarr["act"] = "userNewf";

	$formobj = new formc($initarr, $hiddenarr, 0);
	$fieldxa=NULL;
	$fieldxa[] = $this->fieldx['newuser']; 
	$fieldxa[] = $this->fieldx['newpw']; 
	$formobj->fieldArrOut( $fieldxa );
	
	$formobj->close( TRUE );
	
}

function g_tabSpace1_sub() {

}

function g_tabSpace1() {
	// FUNCTION: create a tablespace
	// ACT: "tabForm"
	
	
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Create a tablespace";
	$initarr["submittitle"] = "Create";
	$initarr["tabwidth"]    = "AUTO";

	$hiddenarr = NULL;
	$hiddenarr["act"] = "tabCrea";

	$formobj = new formc($initarr, $hiddenarr, 0);
	
	if ($this->checkParams(array("newuser"))<0 ) {
		return;
	}
	$newuser = $this->sessParams["newuser"];
	$dbffile = $this->doImpLib->_getUserDbfName($newuser);

	$fieldx = array ( 
		"title" => "tablespace-file", 
		"object"=> "info2",
		"val"   => $dbffile
		);
	$formobj->fieldOut( $fieldx );
	
	$fieldx = array ( 
		"title" => "oracle data dir", 
		"name"  => "oraDataDir",
		"object"=> "text",
		"val"   => $this->sysInfo["tablesSpaceDir"]["val"], 
		"fsize" => 80,
		"notes" => "e.g. /data/oradata/magasin "
		 );
	$formobj->fieldOut( $fieldx );
	
	$formobj->close( TRUE );
}

function g_tabSpace2( &$sqlSys, $parx) {

	$newuser = $this->sessParams["newuser"];
	$newpw   = $this->sessParams["newpw"];
	
	if ($this->checkParams(array("newuser"))<0 ) {
		return;
	}
	$this->_infoOut('user: '. $newuser, 3 );

	$datadir  = $parx["oraDataDir"]; // /data/oradata/magasin
	
	$this->setParam( "oraDataDir", $parx["oraDataDir"] );
	
	$this->saveSessParams();
	
	$this->doImpLib->do_tabSpace($sqlSys, $newuser, $datadir);
	
	$this->setGoStatus("tablespace",1);
}

function g_tabDrop1(&$sqlSys, $parx) {
	
	
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Drop a tablespace";
	$initarr["submittitle"] = "Drop";
	$initarr["tabwidth"]    = "AUTO";

	$hiddenarr = NULL;
	$hiddenarr["act"] = "tabDrop2";

	$formobj = new formc($initarr, $hiddenarr, 0);
	
	$oldUser = $this->sessParams["oldUser"];
	
	$dbffile = $this->doImpLib->_getUserDbfName($oldUser);

	$fieldx = array ( 
		"title" => "old user", 
		"name"  => "oldUser",
		"object"=> "text",
		"val"   => $oldUser,
		'notes'=> $dbffile
		);
	$formobj->fieldOut( $fieldx );
	
	$formobj->close( TRUE );
}

function g_tabDrop2( &$sqlSys, $parx) {
	global $error;
	$oldUser = $parx["oldUser"]; 
	// $oldUser = $this->sessParams["oldUser"];
	/*
	if ($this->checkParams(array("oldUser"))<0 ) {
		return;
	}
	*/
	if ($oldUser==NULL) {
		$error->set($FUNCNAME , 1, 'no old user given.' );
		return -1;	
	} 
	$this->_infoOut('old user: '. $oldUser, 3 );
	
	$this->saveSessParams();
	
	$this->doImpLib->setModiFlag(0);
	$this->doImpLib->do_tabSpaceDrop($sqlSys, $oldUser);
	
	$this->setGoStatus("tablespace",1);
}

function g_userCrea(&$sqlSys) {
	global $error;
	$FUNCNAME="g_userCrea";
	
	$newuser = $this->sessParams["newuser"];
	$pw = $this->sessParams["newpw"];
	
	if ($this->checkParams(array("newuser", "newpw"))<0 ) {
		return;
	}
	
	$this->doImpLib->do_userCrea($sqlSys, $newuser, $pw);
	
	$this->setGoStatus("userExist",1);
	
	echo "o.k.<br>";

}

function g_userTableViews(&$sqlo) {
	global $error;
	$FUNCNAME="g_userTableViews";
	
	if ($this->checkParams(array("newuser"))<0 ) {
		return;
	}

	if ( $sqlo == NULL ) {
		$error->set($FUNCNAME , 1, "Could not login as user ".$newuser );
		return -1;	
	}
	
	$this->doImpLib->do_userTableViews($sqlo, $newuser);
	
	echo "o.k.<br>";
}

function g_userRootPW(&$sqlSys) {
	global $error;
	$FUNCNAME="g_userRootPW";
	
	$newuser = $this->sessParams["newuser"];
	if ($this->checkParams(array("newuser"))<0 ) {
		return;
	}
	
	$this->doImpLib->do_userRootPW($sqlSys, $newuser);
}

function g_dbimp1() {
	
	
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Import a dump-file";
	$initarr["submittitle"] = "Create";
	$initarr["tabwidth"]    = "AUTO";

	$hiddenarr = NULL;
	$hiddenarr["act"] = "dbimp";

	$formobj = new formc($initarr, $hiddenarr, 0);
	
	$fieldxa=NULL;
	$fieldxa[] = $this->fieldx['dumpfile']; 
	$fieldxa[] = $this->fieldx['fromuser']; 
	$formobj->fieldArrOut( $fieldxa );
	
	$formobj->close( TRUE );
	
}

function g_dbSequence(&$sqlSys, $parx) {
	
	
	if (!$parx['go']) {
		
		$initarr   = NULL;
		$initarr["action"]      = $_SERVER['PHP_SELF'];
		$initarr["title"]       = "Reset Sequences";
		$initarr["submittitle"] = "Doit";
		$initarr["tabwidth"]    = "AUTO";
		
		$hiddenarr = NULL;
		$hiddenarr["parx[go]"] = 1;
		$hiddenarr["act"] = 'dbSequence';
		
		$formobj = new formc($initarr, $hiddenarr, 0);
		
		$formobj->close( TRUE );
		return;
	}
	
	$newuser = $this->sessParams["newuser"];
	$this->doImpLib->do_sequenceReset($sqlSys, $newuser);
	
}

function g_appDirSet1() {
	
	
	$configDict=array();
	$configDict['appDataDir']  = $this->confLib->getVal('appDataDir');
	$configDict['appTempRoot'] = $this->confLib->getVal('appTempRoot');
	
	$tabobj = new visufuncs();
	$headOpt = array( "title" => "App Base directories",
			"headNoShow" => 1
	);
	$headx  = array ("Key", "val");
	$tabobj->table_head($headx,   $headOpt);
	
	foreach( $configDict as $key=>$val) {
		$dataArr = array($key, $val);
		$tabobj->table_row ($dataArr);
	}
	$tabobj->table_close();
	echo "<br>";
	
	
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Create/Set application dirs?";
	$initarr["submittitle"] = "Create";
	$initarr["tabwidth"]    = "AUTO";
	
	$hiddenarr = NULL;
	$hiddenarr["act"] = "appDirSet";
	
	$formobj = new formc($initarr, $hiddenarr, 0);
	
	$formobj->close( TRUE );
}

function g_appDirSet2(&$sqlSys) {
	global  $error;

	echo "import dump<br>";


	$newuser = $this->sessParams["newuser"];
	if ($newuser==NULL) {
		$error->set( $FUNCNAME, 1, "name of 'newuser' missing!" );
		return;
	}

	$configDict=array();
	$configDict['appDataDir']  = $this->confLib->getVal('appDataDir');
	$configDict['appTempRoot'] = $this->confLib->getVal('appTempRoot');
	$this->doImpLib->do_appDirsSet( $sqlSys, $newuser, $configDict );


}

function g_dbimp2($parx) {
	global $error;
	$FUNCNAME = "g_dbimp2";
	
	echo "import dump<br>";
	
	if ($this->checkParams(array("newuser"))<0 ) {
		return;
	}
	$fromuser = $parx["fromuser"];
	$file     = $parx["dumpfile"];
	$newuser = $this->sessParams["newuser"];
	
	htmlInfoBox( "Next steps after import", "", "open", "INFO" );
	?>
	<ul>
	<li>Login to the new DB</li>
	<li>Change variables in table GLOBALS</li>
	<li>Meta-data cache: refresh!</li>
	</ul>
	<?
	htmlInfoBox( "", "", "close" );
	echo "<br>";
	
	$this->doImpLib->do_impDump($newuser, $fromuser, $file);
	
}



function g_dbexp() {
	global $error;
	$FUNCNAME = "g_dbexp";
	
	echo "export dump<br>";

	$newuser = $this->sessParams["newuser"];
	$pw        = $this->sessParams["newpw"];
	$dbBackupDir = $this->confLib->getVal('dbBackupDir');
	if ( $dbBackupDir==NULL ) {
		$error->set( $FUNCNAME, 1, "'dbBackupDir' from config missing!" );
		return;
	}
	$this->doImpLib->do_expDump($newuser, $pw, $dbBackupDir);
}

function sessInfo() {
	// show $this->sessParams
	
	if (!sizeof($this->sessParams)) return;
	
	$tabobj = new visufuncs();
	$headOpt = array( "title" => "Session info",
					  "headNoShow" => 1
					  );
	$headx  = array ("Key", "val");
	$tabobj->table_head($headx,   $headOpt);
	
	foreach( $this->sessParams as $key=>$val) {
		$dataArr = array($key, $val);
		$tabobj->table_row ($dataArr);
	}	
	reset ($this->sessParams); 
	$tabobj->table_close();
	echo "<br>";
}

}


global $error, $varcol;

$error = & ErrorHandler::get();
if ($error->printLast()) htmlFoot();

$act 		= $_REQUEST["act"];
$parx 	= $_REQUEST["parx"];

$title_sh       	 = "Manage a database-dump";
$title       		 = "Database-dump: only support for ORACLE!";
$infoarr			 = NULL;
$infoarr['title']    = $title;
$infoarr['form_type']= 'tool'; // 'tool', 'list'
$infoarr['design']   = 'norm';
$infoarr['locrow']   = array( array('../rootFuncs.php', 'Administration tools') );


$sqlo_dummy=NULL; // here a dummy 

$pagelib = new gHtmlHead();
$pagelib->startPage($sqlo_dummy, $infoarr);
$pagelib->chkErrStop();

$linkarr = array(
	array('g.dbNew.php', 'create a new Magasin-instance'),
	);
$li1 = $linkarr[0];	
echo '&nbsp;[<a href="'.$li1[0].'">'.$li1[1].'</a>]'."<br><br>\n";
echo "<ul>\n";

$mainLib = new db_imp_gui($act);
if ( $error->printAll() ) {
	htmlFoot();
}

if (  $act=="installLogin" ) {
	$install_admin = $mainLib->g_installLogin($parx);
	if ($error->Got(READONLY))  {
		$error->printAll();
	} else {
		echo "register install_admin ... " ;
		$_SESSION["install_admin"] = $install_admin;
	}
}

if ( !$mainLib->installCheckLogin() ) {
	$mainLib->installFormLogin();
	$mainLib->htmlFootX();
}

$mainLib->goInfo($act);
if ( $mainLib->checkLogin() ) {
	if ($sqlSys==NULL) $sqlSys = $mainLib->login( $_SESSION["db_admin"] );
	$mainLib->sysInfo($sqlSys);
}
$mainLib->shNavi();
$mainLib->sessInfo();


// DB-system login

if (  $act=="login" ) {
	list($sqlSys,$db_admin)  = $mainLib->doLogin($parx);
	if ($error->Got(READONLY))  {
		$error->printAll();
	} else {
		echo "register db_admin ... " ;
		$_SESSION["db_admin"]=$db_admin;
	}
}

if ( !$mainLib->checkLogin() ) {
	$mainLib->g_DBformLogin();
	$mainLib->htmlFootX();
}

$sqlNorm = $mainLib->doUserLogin();

$mainLib->sysInfoShow();

if ( $act=="olduser" ) {
	$mainLib->g_oldUser();
	$mainLib->htmlFootX();
}

if ( $act=="askuser" ) {
	$mainLib->g2_anaDbUser($parx);
	$mainLib->htmlFootX();
}

if ( $act=="deluser" ) {
	$mainLib->g_user3del($sqlSys, $parx);
	$mainLib->htmlFootX();
}

if ( $act=="userNewf" ) {
	$mainLib->g_newUser1($parx, $go);
	$mainLib->htmlFootX();
}

if ( $act=="tabForm" ) {
	$mainLib->g_tabSpace1();
	$mainLib->htmlFootX();
}
if ( $act=="tabCrea" ) {
	$mainLib->g_tabSpace2($sqlSys, $parx);
	$mainLib->htmlFootX();
}

if ( $act=='tabDrop' ) {
	$mainLib->g_tabDrop1($sqlSys, $parx);
	$mainLib->htmlFootX();
}
if ( $act=='tabDrop2' ) {
	$mainLib->g_tabDrop2($sqlSys, $parx);
	$mainLib->htmlFootX();
}

if ( $act=="userCrea" ) {
	$mainLib->g_userCrea($sqlSys);
	$mainLib->htmlFootX();
}

if ( $act=="userRootPW" ) {
	$mainLib->g_userRootPW($sqlSys);
	$mainLib->htmlFootX();
}

if ( $act=="userTableViews" ) {
	$mainLib->g_userTableViews($sqlNorm);
	$mainLib->htmlFootX();
}


if ( $act=="dbimpInp" ) {
	$mainLib->g_dbimp1();
	$mainLib->htmlFootX();
}

if ( $act=="dbimp" ) {
	$mainLib->g_dbimp2($parx);
	$mainLib->htmlFootX();
}

if ( $act=="dbSequence" ) {
	$mainLib->g_dbSequence($sqlSys, $parx);
	$mainLib->htmlFootX();
}



if ( $act=="appDirSetInp" ) {
	$mainLib->g_appDirSet1();
	$mainLib->htmlFootX();
}
if ( $act=="appDirSet" ) {
	$mainLib->g_appDirSet2($sqlSys);
	$mainLib->htmlFootX();
}

if ( $act=="dbexp" ) {
	$mainLib->g_dbexp($parx);
	$mainLib->htmlFootX();
}
 
