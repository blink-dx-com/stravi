<?php
/**
 * - PHP Script for testing the default "globals" of the application
   - Set debug to 1 if you want debugging
   - Count errors
   
   - INFO: for new checks, please use the lib $sysChMeta->checkByName($sql, $key);
   
   - can be extended for a lab: $_SESSION['globals']["lab_path"]."/glob.syscheckLab.inc"
   - configure, which tests should be performed: config/settings.syscheck.json
   
 * @package glob.syscheck.php
 * @swreq UREQ:0001115: g > Application Systemcheck
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param 
    [$debug]  0 - short info
              1 - more info (show edit links)
              2 - more info,  number of files in paths, file size (takes time...)
              3 - extra tests
    $action = http_cache_clean
 */ 
session_start(); 


require_once ('reqnormal.inc');
require_once ("logincheck.inc");
require_once ("subs/glob.syscheck.inc");
require_once ('f.msgboxes.inc');

// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------
// ---------------------------------------------------------------------------------

$error = & ErrorHandler::get();
$sql  = logon2( $_SERVER['PHP_SELF'] );

$debug = $_REQUEST["debug"];
$action = $_REQUEST["action"];

$title = "System check";
$infoarr=array();
$infoarr["help_url"] = "g.System_check.html";
$infoarr["title"] = $title;
$infoarr["form_type"]= "tool";
$infoarr["locrow"]= array( array("rootsubs/rootFuncs.php", "Administration" ) );
$infoarr["icon"]     = "images/f.glob.syscheck.gif";
$infoarr["css"] 	 = ".xshead  { background-color:#000030; text-align:center; font-weight:bold; color: #FFFFFF; }";

$pagelib = new gHtmlHead();
$pagelib->startPage($sql, $infoarr);

if (empty($debug)) $debug = 0;
$tabcol_num = 2; 

// update globals !!!
$_SESSION['globals']= NULL;
require("config.inc");
$loginLib = new fLoginC();
$loginLib->loginGlobals( $sql );
$_SESSION['globals'] = $globals; // update globals variables ....

$justGlob = NULL;

$gquery="select name, value from globals order by name";
$sql->query($gquery);
while ($sql->ReadRow()) {
    $justGlob[$sql->RowData[0]]= $sql->RowData[1]; // current globals
}


$sysMessObj    = new sysMessC($debug);
$sysCheckObj   = new sysCheckC($sysMessObj, $justGlob);

$sysCheckObj->showHead($debug);

$sysMessObjLab = NULL;

if ( $_SESSION['globals']["lab_path"]!="" ) {
	$labsubs = "../".$_SESSION['globals']["lab_path"]."/glob.syscheckLab.inc";
	if ( file_exists($labsubs) ) {
		require_once ($labsubs);
		$sysMessObjLab = new sysMessLabC($sysCheckObj);
	}
}

$sysChMeta = new sysCheckByName($sysCheckObj, $debug);

if ($debug>0) { 
    $tabcol_num =$tabcol_num+3;
}
echo "<br>\n";
if ($debug<=0) echo "<ul>\n"; 
echo "<br>";




//Alter relative path -- please change it if you change the location of script
//$http_cache_path="../".$http_cache_path;

// Test values -- Taken from database now
//   $img_path="/home/data/images.robur";
//   $work_path="/tmp";
//   $img_convert="/usr/bin/convert";
//   $img_path_win="//conan/home/data/images.robur";
//   $idl_path_rpc="/home/steffen/Programme/CCT_tools/Temp";
//   $img_identify="/usr/bin/identify";
//   $http_cache_path="Temp.ganja";


// Test for Image path
clearstatcache();

if ($debug==3) {
	$extraTests = new syscheck_extra($sysChMeta);
	$extraTests->doall($sql);
	echo "<hr>";
	htmlFoot();
}

$optxy = array("headline"=>1);
$sysMessObj->messageout( "", "Security", "", "", "", "", "",$optxy );


$sysChMeta->checkByName($sql, "DbLoginMeth");
$sysChMeta->checkByName($sql, "DbLoginDeny");
$sysChMeta->checkByName($sql, "DbUserMessage");
$sysChMeta->checkByName($sql, "DbLoginIPOnly");
$sysChMeta->checkByName($sql, "security_level");
$sysChMeta->checkByName($sql, "security_write");

$optxy = array("headline"=>1);
$sysMessObj->messageout( "", "Paths", "", "", "", "", "",$optxy );


$sysChMeta->checkByName($sql, 'img_path');
$sysChMeta->checkByName($sql, 'data_path');
$sysChMeta->checkByName($sql, "work_path");
$sysChMeta->checkByName($sql, "lab_path");
$sysChMeta->checkByName($sql, "img_convert");
$sysChMeta->checkByName($sql, "img_identify");


$sysCheckObj->test_http_cache_path();
$sysCheckObj->test_path_sys_net2srv();

$sysChMeta->checkByName($sql, "app.upload_zip");
$sysChMeta->checkByName($sql, 'app.modulLog.dat');


$optxy = array("headline"=>1);
$sysMessObj->messageout( "", "Misc variables", "", "", "", "", "",$optxy );

$sysChMeta->checkByName($sql, 'magasin_serial');
$sysChMeta->checkByName($sql, "Db.encoding");
$sysChMeta->checkByName($sql, "o.PROJ.userhome");

$optxy = array("headline"=>2);
$sysMessObj->messageout( "", "", "", "", "", "", "",$optxy );

$optxy = array("headline"=>1);
$sysMessObj->messageout( "", "Optional navigation settings", "", "", "", "", "",$optxy );   

$sysCheckObj->test_editScript($sql);

$error_flag = this_funcCreaOpt($sql, $sysMessObj);
if ($error_flag) $sysCheckObj->errorInc();

$sysCheckObj->test_tableTrigger($sql);

//

$optxy = array("headline"=>1);
$sysMessObj->messageout( "", "Logs + Debugging", "", "", "", "", "",$optxy ); 

$sysCheckObj->logsAccLogEna  ( $sql );
$sysChMeta->checkByName($sql,'app.advmod');

$theopt = array("autoCreate"=>1);
$sysCheckObj->fileTest  ( $sql, "app.logfile", "application log file", 0, "stores severe errors", $theopt );
$sysCheckObj->simpleTest( $sql, "globLogFlag", "Global SQL-log", "log SQL-queries for all users: only queries with high exec time");

$sysChMeta->checkByName($sql,'xml_rpc.debug_dir');



if ( $sysMessObjLab!=NULL ) {
	$optxy = array("headline"=>1);
	$sysMessObj->messageout( "", "LAB specific test", "", "", "", "", "",$optxy );
	$sysMessObjLab->labTests($sql);
}

$optxy = array("headline"=>1);
$sysMessObj->messageout( "", "Optional packages", "", "", "", "", "",$optxy );      

$sysChMeta->checkByName($sql, "pol.objRelease"); 
$sysChMeta->checkByName($sql, "xDocVersControl");
$sysChMeta->checkByName($sql, "exe.graphviz");
$sysChMeta->checkByName($sql, "NTDPAL_EXE");
$sysChMeta->checkByName($sql, "exe.R");
$sysChMeta->checkByName($sql, "htmlFrameTop.homeBut");


// ================================================================================================
// other tests
// ================================================================================================


$optxy = array("headline"=>1);
$sysMessObj->messageout( "", "Advanced Tests", "", "", "", "", "", $optxy ); 

$sysChMeta->checkByName($sql, "xRoleRights");
$sysChMeta->checkByName($sql, "xUserHaveRoles");
	
$sysCheckObj->test_install_wiid ( $sql );

$sysCheckObj->tableClose();

//Test to see if all is ok
if( $sysCheckObj->errcnt>0 ) {
	cMsgbox::showBox("error", "Errors detected!"); 
} else {
	cMsgbox::showBox("ok", "Main tests completed succesfully !"); 
}

if ($debug>0) {
    echo "<br>";
    echo "<b>Table of Timing of some Tests (in seconds):</b><br>\n";
    $time_arr = $sysChMeta->get_timearr();
    echo '<table>';
    foreach ($time_arr as $key=>$row) {
        $tdiff = $row['end']- $row['start'];
        $tdiff = round($tdiff,2);
        echo "<tr><td>".$key."</td><td>".$tdiff."</td></tr>";    
    }
    
    if ( $sysMessObjLab!=NULL ) {
        $time_arr = $sysMessObjLab->get_timearr();
        foreach ($time_arr as $key=>$row) {
            $tdiff = $row['end']- $row['start'];
            $tdiff = round($tdiff,2);
            echo "<tr><td>".$key."</td><td>".$tdiff."</td></tr>";
        }
    }
    
    echo '</table>';
}

if ($debug<=0) echo "<br><br></ul>";



echo "<hr>";
htmlFoot();

// ****************************** ende ******************************
