<?php
/**
 * create a new Magasin-instance (table-space + user)
 * Header: www/pionir/rootsubs/db_imp/g.dbNew.php
 * @package g.dbNew.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param array $parx
 * @param array $go : 0,1
 */
extract($_REQUEST); 
session_start(); 


require_once ('func_head2.inc');
require_once ('g.dbAdminGui.inc');

class db_new_gui {

function __construct($parx, $go) {
	$this->parx = $parx;
	$this->go = $go;
	$this->AdminLib = new dbAdminSub();
	$this->doImpLib = new db_imp_act();
	$this->dbAdminGui = new dbAdminGui();
	$this->confLib = new gConfInstSub();
	$this->confLib->loadFile();
	
	$this->doImpLib->setModiFlag(0);
	$this->doImpLib->setshowCmd(1);
	
	$this->fieldx = $this->dbAdminGui->getFormArr();
	// $req: 1: need, 2: optional
	$this->fieldKeys = array(
		'newuser'=>1, 'newpw'=>1, 'dumpfile'=>1, 'fromuser'=>1,
		'dumpfileTabs'=>2, 'magasin_ser'=>1 );
}

function checkLogin() {
	if ( !$this->AdminLib->installCheckLogin() ) {
		$this->htmlFoot("Error", "Not logged in with install_password.");
	}
	if ( ! $this->AdminLib->checkLogin() ) {
		$this->htmlFoot("Error", "Not logged in as DB-admin.");
	}
}

function htmlFoot($text1, $text2=NULL) {
	htmlFoot($text1, $text2);
}

function GoInfo($go) {

	$goArray   = array( "0"=>"Give params", 1=>"Prepare", 2=>"Create" );
	$extratext = "[<a href=\"".$_SERVER['PHP_SELF']."\">Start again</a>]";
	
	$formPageLib = new FormPageC();
	$formPageLib->init( $goArray, $extratext );
	$formPageLib->goInfo( $go ); 
	echo "<br>";

}

function &login( ) {	
	return ( $this->doImpLib->login( $_SESSION["db_admin"] ) );
}

function formCrea() {
	
	
	$parx = $this->parx;

	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]         = "Creation parameters";
	$initarr["submittitle"] = "Next&gt;";
	$initarr["tabwidth"]    = "AUTO";

	$hiddenarr = NULL;

	$formobj = new formc($initarr, $hiddenarr, 0);
	
	$fieldKeys = $this->fieldKeys;
	$fieldxa=NULL;
	foreach( $fieldKeys as $key=>$dummy) {
		$field = $this->fieldx[$key];
		if ( $parx[$key]!=NULL ) $field['val'] = $parx[$key];
		$fieldxa[] = $field; 
	}
	reset ($fieldKeys);
	
	$formobj->fieldArrOut( $fieldxa );
	$formobj->close( TRUE );
}

function formPrep() {
	
	
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]         = "Prepare";
	$initarr["submittitle"] = "Do!";
	$initarr["tabwidth"]    = "AUTO";

	$hiddenarr = NULL;
	$fieldKeys = $this->fieldKeys;
	foreach( $fieldKeys as $key=>$dummy) {
		$hiddenarr['parx['.$key.']'] = $this->parx[$key]; 
	}
	reset ($fieldKeys);

	$formobj = new formc($initarr, $hiddenarr, 1);
	$formobj->close( TRUE );
}

function checkParams(&$sqlSys) {
	global $error;
	$FUNCNAME= 'checkParams';
	
	$roleCCT = $this->doImpLib->checkRole($sqlSys);

	$parx = $this->parx;
	$fieldKeys = $this->fieldKeys;
	$fieldxa=NULL;
	$errflag=0;
	$errtext= NULL;
	foreach( $fieldKeys as $key=>$reqVal) {
		$val = $parx[$key] ;
		if ( $val == '' and $reqVal<=1) {
			$errtext .= "value missing. ";
			$errflag=1;
			break;
		}
		// 'newuser', 'newpw', 'dumpfile', 'fromuser'
		switch ($key) {
			case 'newuser':
				$val= strtoupper($val);
				$this->parx['newuser'] = $val; // correct it ...
				$answer = $this->doImpLib->dbUserCheck( $sqlSys,  $val );
				if ($answer>0) {
					$errtext = "user $val already exists.";
					$errflag=2;
				}
				break;
			case 'dumpfile':
				if ( !file_exists($val) ) {
					$errtext = "Dumpfile does not exists.";
					$errflag=3;
				}
				break;
		}
		if ( $errflag>0 ) break; 
	}
	reset ($fieldKeys);
	
	if ($errflag) {
		$error->set( $FUNCNAME, 1, 'key: '.$key.' : '.$errtext );
		return;
	}
	
	$tablesSpaceDir = $this->doImpLib->tableSpaceDirGet($sqlSys);
	$this->sysInfo["tablesSpaceDir"] = array("val"=>$tablesSpaceDir, "inf"=>"");
	if ($tablesSpaceDir==NULL) {
		$error->set( $FUNCNAME, 4, 'tablesSpaceDir missing.' );
		return;
	} 
	
	$spacename = $this->doImpLib->SpaceUserCheck($sqlSys, $parx['newuser']  );
	$this->sysInfo["tableSpaceUser"] =  array("val"=>$spacename, "inf"=>"tablespace of user");
	if ($spacename!=NULL) {
		//$error->set( $FUNCNAME, 3, 'tableSpaceUser already exists' );
		//return;
		echo "Tablespace exists!<br>";
	} 

}

function _shConfig() {
	$parx = $this->parx;
	$dbid   =  strtolower($parx['newuser']);
	$admParams = $this->doImpLib->getAdminParams();
	$dbname = $admParams['db'];
	$alias  = $dbid.'@'.$dbname;
	$answer = $this->doImpLib->do_shConfig( $dbid, $dbname, $parx['newuser'], $parx['newpw'], $alias );

	htmlInfoBox( "config.local.inc", "", "open", "INFO" );
	echo '<pre>';
	echo htmlspecialchars($answer);
	echo '</pre>';
	htmlInfoBox( "", "", "close" );
}

function _setMagSerial( &$sqlSys ) {
	global $error;
	$FUNCNAME= '_setMagSerial';
	$serial = $this->parx['magasin_ser'];
	if (!is_numeric($serial)) {
		$error->set( $FUNCNAME, 1, 'magasin_serial "'.$serial.'"must be a number.' );
		return;
	}
	$vars = array('magasin_serial'=>$serial );
	$this->doImpLib->do_saveGlobals($sqlSys, $this->parx['newuser'], $vars);
}

function _appDirsSet( &$sqlSys ) {
	global $error;
	$FUNCNAME= '_appDirsSet';
	
	$newuser  = $this->parx['newuser'];
	
	$configDict=array();
	$configDict['appDataDir']  = $this->confLib->getVal('appDataDir');
	$configDict['appTempRoot'] = $this->confLib->getVal('appTempRoot');
	$this->doImpLib->do_appDirsSet( $sqlSys, $newuser, $configDict );
	
}

function doit(&$sqlSys, $go) {
	global $error;
	$FUNCNAME= 'doit';
	$parx = $this->parx;
	
	if ($go==2)  $this->doImpLib->setModiFlag(1);
	
	if ( $this->sysInfo["tableSpaceUser"]["val"] == NULL ) {
		$datadir = $this->sysInfo["tablesSpaceDir"]['val'];
		$this->doImpLib->do_tabSpace($sqlSys, $parx['newuser'], $datadir);
	}
	if ($error->Got(READONLY))  return;
	$this->doImpLib->do_userCrea($sqlSys, $parx['newuser'], $parx['newpw']);
	if ($error->Got(READONLY))  return;

	$impTimeStart = time();

	$optimp = array( 'seltabs' => $parx['dumpfileTabs'] );
	$this->doImpLib->do_impDump($parx['newuser'], $parx['fromuser'], $parx['dumpfile'], $optimp );
	
	$impTimeEnd = time();
	$impTimeDiff = $impTimeEnd - $impTimeStart;
	$this->sysInfo["impTimeDiffSec"] = array("val"=>$impTimeDiff, "inf"=>"sec");
	$this->sysInfo["impTimeDiffMin"] = intval($impTimeDiff / 60.0);
	
	if ($error->Got(READONLY)) {
		return;
	}
	
	$this->doImpLib->do_userRootPW( $sqlSys, $parx['newuser'] );
	if ($error->Got(READONLY))  return;
	$this->_setMagSerial( $sqlSys );
	if ($error->Got(READONLY))  return;
	$this->_appDirsSet( $sqlSys );
	if ($error->Got(READONLY))  return;
	
}

function help() {
	echo "<br>";
	htmlInfoBox( "Short help", "", "open", "HELP" );
	?>
	This tool creates a new MAGASIN instance.<br>
	<ul>
		<li>Create a new user</li>
		<li>Create a new tablespace</li>
		<li>Import database-DUMP</li>
		<li>Reset 'root' password</li>
		<li>mkdir application dirs</li>
		<li>Change variables in table GLOBALS:<ul>
			<li>img_path</li>
			<li>data_path</li>
			<li>http_cache_path</li>
			<li>magasin_serial</li>
			<li>... installation WIID entry</li>
		</ul>
	</ul>
	<br><br>
	Not yet implemented ...
	<ul>
	<li>Change config.locals.inc</li>
	<li>Meta-data cache: refresh!</li>
	</ul>
	<?
	htmlInfoBox( "", "", "close" );
}

}

global $error, $varcol;

$error = & ErrorHandler::get();
if ($error->printLast()) htmlFoot();
$parx 	= $_REQUEST["parx"];

$title       		 = "Create a new Magasin-instance (space+user)";
$infoarr			 = NULL;
$infoarr['title']    = $title;
$infoarr['form_type']= 'tool'; // 'tool', 'list'
$infoarr['design']   = 'norm';
$infoarr['locrow']   = array( 
    array('../rootFuncs.php', 'Administration tools') ,
    array('index.php', 'Import home') 
);
$sqlo_dummy=NULL; // here a dummy
$pagelib = new gHtmlHead();
$pagelib->startPage($sqlo_dummy, $infoarr);
$pagelib->chkErrStop();


echo "<ul>\n";

$mainLib = new db_new_gui($parx, $go);
$mainLib->GoInfo($go);
$mainLib->checkLogin();

$sqlSys = $mainLib->login( );

if (!$go) {
	$mainLib->formCrea();
	$mainLib->help();
	htmlFoot('<hr>');
}

glob_printr( $mainLib->parx, "parx info" );
$mainLib->_shConfig();

if ( $go<2) {
	$mainLib->formPrep();
	// htmlFoot('<hr>');
}

$mainLib->checkParams($sqlSys);
if ( $error->printAll() ) {
	htmlFoot();
}

$mainLib->doit($sqlSys, $go);
glob_printr( $mainLib->sysInfo, "sysInfo info" );

$error->printAll();
htmlFoot('<hr>');

