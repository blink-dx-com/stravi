<?php
/**
 * show software features
 * @package sysinfo.php
 * @swreq UREQ:xxxxxxxxxxxx
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/www/pionir/sysinfo.php 59 2018-11-21 09:04:09Z $
 */
session_start(); 


require_once ('reqnormal.inc');
require_once("utilities.inc");
require_once('f.util_IniAna.inc');
require_once('f.configRead.inc');

class sysinfo_helper {

	function tabout($var, $text, $text2=NULL) {
		global $color;
		$color1 = "#EFEFEF";  // SUN violett 
		$color2 = "#EFEFFF";
		
	    if ($color == $color1)  $color = $color2;
	    else $color = $color1;  
	    echo "<tr bgcolor=\"".$color."\"><td>" .$var. "</td><td><B>" .$text. "</B></td>";
		if ($text2!="") echo "<td>" .$text2. "</td>";
		echo "</tr>\n";
	}
	
	function getGlob(&$sql, $varname) {
		$sql->query("select value from GLOBALS where name='".$varname."'");
		$sql->ReadRow();
		$retval = $sql->RowData[0];
		return ($retval);
	}
	
	/**
	 * read LAB config file
	 * @return string : version string
	 */
	function getLabVersionInfo() {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		if ($_SESSION['s_sessVars']['AppLabLibDir']==NULL) {
			return NULL; // no LAB code defined
		}
		
		$kernelFile = $_SESSION['s_sessVars']['AppLabLibDir'].'/zzz.version.dat';
		
		if (!file_exists($kernelFile)) {
			$error->set( $FUNCNAME, 1, 'version file not found: '.$kernelFile );
			return;
		}
	
		$confReadLib = new fConfigRead($kernelFile);
		$kernelVars  = $confReadLib->readVars();
		if ($kernelVars==NULL) {
			$error->set( $FUNCNAME, 2, 'no version-info found for '.$kernelFile );
			return;
		}
	
		return  $kernelVars['labVersion'];
	}

}

global $error, $varcol;

$error = & ErrorHandler::get();

$sqlo = logon2( $_SERVER['PHP_SELF'] );
$title="System information &gt; software";
$infoarr=array();
$infoarr["locrow"] = array (array("n.syslinks.php", "System info"), array("sysinfo.php","Software"),  );
$infoarr["title"] = $title;
$infoarr["form_type"]= "tool";

$pagelib = new gHtmlHead();
$pagelib->startPage($sqlo, $infoarr);

$mainlib = new sysinfo_helper();

$nls_params=array();

$db_type=$_SESSION['sec']['_dbtype'];

if ($db_type=='CDB_OCI8') {
    $sqlsel = 'PARAMETER,VALUE FROM NLS_DATABASE_PARAMETERS';
    $sqlo->Quesel($sqlsel);
    while ($sqlo->ReadRow()) {
    	if ($sqlo->RowData[0]=='NLS_CHARACTERSET' or $sqlo->RowData[0]=='NLS_NCHAR_CHARACTERSET') {
    		$nls_params[$sqlo->RowData[0]] = $sqlo->RowData[1];
    	}
    }
}

$infox=array();
$infox['labVersion']= $mainlib->getLabVersionInfo();



echo "&nbsp;<p>\n";
echo "<table cellspacing=1 cellpadding=1 border=0>";
$mainlib->tabout("Database-management-system:", $_SESSION['sec']['_dbtype'] ); // OCIServerVersion($sqlo->_db_linkid)
$mainlib->tabout("Database-service-name: ","".$_SESSION['sec']['db']);
$mainlib->tabout("Database-version: ","".$_SESSION['globals']['DBVersion']."");
$mainlib->tabout("Database-LAB-version: ", $mainlib->GetGlob($sqlo, "DBVersion_LAB") );
$mainlib->tabout("Database-serial-no: ","".$_SESSION['globals']['magasin_serial']."");
if ($db_type=='CDB_OCI8') {
    $mainlib->tabout("Database-NLS_CHARACTERSET: ",		$nls_params['NLS_CHARACTERSET'] );
    $mainlib->tabout("Database-NLS_NCHAR_CHARACTERSET: ",$nls_params['NLS_NCHAR_CHARACTERSET']);
}
$mainlib->tabout("Db.encoding: ", $_SESSION['globals']['Db.encoding'] );
$mainlib->tabout("Product-Name: ","".$_SESSION['s_product']["product.name"]."");
$mainlib->tabout("Application-type: ","".$_SESSION['s_product']["type"]."");
$mainlib->tabout("Application-Version","".$_SESSION['s_product']["project"]."_".$_SESSION['s_product']["version"]." ".$_SESSION['s_product']["date"]."");
$mainlib->tabout("Pionir-kernel-version: ",$_SESSION['s_product']["pionir_version"]);
$mainlib->tabout("Pionir-lab-version: ",   $infox['labVersion']);
$mainlib->tabout("Application-administrator: ","".$_SESSION['globals']["adminEmail"]."");
$mainlib->tabout("Application-server: ","".$_SERVER["HTTP_HOST"]."");
$mainlib->tabout("PHP-version: ","".phpversion()."");
$mainlib->tabout("Application-server-software: ","".$_SERVER["SERVER_SOFTWARE"]."");
$mainlib->tabout("Web-browser: ","".$_SERVER["HTTP_USER_AGENT"]."");

echo "</table>\n";


echo "&nbsp;<p>\n";
echo "File-size limits for uploading files<br><br>\n";
echo "<table cellspacing=2 border=0>";

$mainlib->tabout("Post limit: "  , (ini_get_bytes('post_max_size')/1E6)."&nbsp;MByte</B>","&nbsp;&nbsp;(php.ini:&nbsp;post_max_size)");
$mainlib->tabout("Upload limit:" , (ini_get_bytes('upload_max_filesize')/1E6)."&nbsp;MByte</B>","&nbsp;&nbsp;(php.ini:&nbsp;upload_max_filesize)");
$mainlib->tabout("ASCI table: "  ,($_SESSION['globals']["F.ASCI_TABLE.IMPORT.UPLOAD_MAX_SIZE"]/1E6)."&nbsp;MByte</B>","&nbsp;&nbsp;(F.ASCI_TABLE.IMPORT.UPLOAD_MAX_SIZE)");
$mainlib->tabout("Image of business object: ",($_SESSION['globals']["F.IMG.IMPORT.UPLOAD_MAX_SIZE"]/1E6)."&nbsp;MByte</B>","&nbsp;&nbsp;(F.IMG.IMPORT.UPLOAD_MAX_SIZE)");
$mainlib->tabout("Image of user:",($_SESSION['globals']["F.IMAGE_DB_USER.IMPORT.UPLOAD_MAX_SIZE"]/1E6)."&nbsp;MByte</B>","&nbsp;&nbsp;(F.IMAGE_DB_USER.IMPORT.UPLOAD_MAX_SIZE)");
$mainlib->tabout("PartisanXML: " ,($_SESSION['globals']["F.PARTISANXML.IMPORT.UPLOAD_MAX_SIZE"]/1E6)."&nbsp;MByte</B>","&nbsp;&nbsp;(F.PARTISANXML.IMPORT.UPLOAD_MAX_SIZE)");
echo "</table>\n"; 

$error->printAll();

$pagelib->htmlFoot('<hr>');
