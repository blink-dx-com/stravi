<?php
/**
 * export image-files from EXP or IMG list
 * - protects left space on $_SESSION['globals']['work_path']
 * - it checks the left space on 'work_path' at analysis for each image and before 
 *   the TAR-action
 * @package obj.img.list_export.php
 * @swreq FS:FS-EXP-IMG
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param   $tablemom: [IMG], "EXP"
  	  $go	: 0 html form
			: 1 analyse
	  	     : 2 export
			: 4 clear temp-dir
	$parx["defext"]
			'namepol' name policy : ['NAME'], 'ID'
			'infolev' : 0,1,2,3
 */
 
session_start(); 


require_once ('reqnormal.inc');
require_once ('glob.objtab.page.inc');
require_once('object.subs.inc');
require_once('down_up_load.inc');
require_once("sql_query_dyn.inc");
require_once("f.workdir.inc");  
require_once 'zip.inc'; 
require_once("f.compress.inc");  
require_once("glob.image.inc");  
require_once ("visufuncs.inc");
require_once ("func_form.inc");
require_once('f.progressBar.inc');

// analyze a single object (EXP or IMG)
class oExpImgExpSing {

function __construct($tablemom, $go, $parx) {
	$this->tablemom=$tablemom;
	$this->go = $go;
	$this->parx = $parx;
	
	$this->workDirlib = new workDir();
	
	$this->badchars = array( "\\","/", "*", "?", "\"", ":", ">", "<", "|" );
}

function setTempDir($tmpdir) {
	$this->tmpdir = $tmpdir;
}

// get export file name
function _getExportName($name, $ID) {

	if ( $this->parx['namepol'] =='NAME' ) {
		// else 'ID'
		$nameWarn = NULL;
		$filenameSingle1 = trim($name);  // generate a nice file-name ...
		$filenameSingle  = str_replace( $this->badchars, "_", $filenameSingle1);
		if ($filenameSingle!=$filenameSingle1) {
			$nameWarn = "some name-letters changed for file-name-compatibility";
		}
		return array($filenameSingle, $nameWarn);
	} else { 
		return array($ID, NULL);
	}
}

function _getImgExt($IMG_MIME, $IMG_NAME) {

	$imgext = "";
	do {
		$tmpext = $IMG_MIME;
		if ($tmpext!="") {
			if (strpos($tmpext,"image/")=== 0 ) {
				$imgext = substr($tmpext,6);
				break;
			}
		} 
		
		if ($IMG_NAME!="") {
			$imgext = "";
			do {
				$tmppos = strrpos($IMG_NAME,".");
				if (!$tmppos) break;
				
				$laststr = substr($IMG_NAME,$tmppos+1);
				if (strstr($laststr,"/")!=NULL) break;  // last part contains "/"
				if (strstr($laststr,"\\")!=NULL) break; // contains "\"
				$imgext = $laststr;
			} while (0);
			if ($imgext!="") break; 
		} 
		
	} while (0);
	
	return ($imgext);
}

// analyse image name
function _getImgBasename($IMG_NAME) {
	if ($IMG_NAME=="") return array(-1, 'no name'); 
	return array(0, '');
}

/**
 *  analyze one object
 *  copy image-files to WORK-dir
 *  @throws Exception::
 *    1: if workDir space exceeded space
 *  @return: array $answer
 *  'imgOriName'
	'nameWarn'
	'htmlname'
	'IMG_ID'
	'fileSingle'
	'size' : in bytes
 *  'ok' 
 *		1 : o.k.
 *		2 : warn  : file name was changed
 *		<0 : error
 * 		-1 : image does not exist on server
 * 		-2 : collecting file failed.
 */
function analyze( &$sqlo, $oneObjData ) {
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	$parx	  = $this->parx;
	$tablemom = $this->tablemom;
	$go 	  = $this->go;
	$tmpdir   = $this->tmpdir;

	$answer = NULL;
	$answer['size'] = 0;

	if ( $tablemom=="EXP" ) {

		$expid   = $oneObjData[0];
		$expname = $oneObjData[1];

		$sqlsx   = "SELECT x.IMG_ID, x.NAME, x.MIME_TYPE ".
				" FROM IMG x, IMG_OF_EXP e where e.EXP_ID=". $expid.
				" AND e.IMG_ID=x.IMG_ID";
		$sqlo->query($sqlsx);
		$sqlo->ReadRow();
		$IMG_ID 	= $sqlo->RowData[0];
		$IMG_NAME 	= $sqlo->RowData[1];
		$IMG_MIME   = $sqlo->RowData[2];
	} else {
		$IMG_ID 	= $oneObjData[0];
		$IMG_NAME 	= $oneObjData[1];
		$IMG_MIME   = $oneObjData[2];
	}
	$htmlname   = htmlspecialchars($IMG_NAME);

	$answer['htmlname'] = $htmlname;
	$answer['IMG_ID']   = $IMG_ID;

	$imgext = $this->_getImgExt($IMG_MIME, $IMG_NAME);
	
	if ( $tablemom=="EXP" ) {
		$srcName = $expname;
		$srcID	  = $expid ;
	} else {
		$srcName = $IMG_NAME;
		$imgres = $this->_getImgBasename($IMG_NAME) ;
		if ($imgres[0]<0) $srcName = $IMG_ID;
		$srcID     = $IMG_ID;
	}
	list($filenameSingle, $nameWarn) = $this->_getExportName($srcName, $srcID);
	
	if ( $imgext=="" AND $parx["defext"]!="") $imgext = $parx["defext"];
	if ( $imgext!="" ) $filenameSingle .= ".".$imgext;
	
	
	$filenameTmp    = $tmpdir."/".$filenameSingle;
	
	$answer['fileSingle'] = $filenameSingle;
	$answer['infoText']   = "server";
	$imageOriName = imgPathFull($IMG_ID);

	if ( !imgOnServerExists( $IMG_ID ) ) {
	
		$errmess = "image does not exist on server";
		
		if ($IMG_NAME!="") $serverName = netfile2serverpath( $IMG_NAME ); 
		if ( !file_exists($serverName) or !is_file($serverName) ) {
			$errmess .= " and not on Intranet.";
			$answer['errmsg'] = $errmess." ".$htmlname;
			$answer['imgOriName'] = $imageOriName;
			$answer['ok'] = -1; 
			return $answer;
			
		} else {
			$answer['infoText'] = "intranet";
			$errmess = "";
			$imageOriName = $serverName;
		}
		
	}
	
	$answer['imgOriName'] = $imageOriName;
	$answer['size']       = filesize($imageOriName);
	
	if ( $go==2 ) {
		
		$spaceExpect = $answer['size'];
		$info = $this->workDirlib->spaceAnalysis($spaceExpect);
		if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 1, 'Error on WorkDir space analysis.' );
			return;
		}
		
		if (!copy ($imageOriName, $filenameTmp) ) {
			$answer['errmsg'] =  "collecting file failed. origin: ".$htmlname;
			$answer['ok'] = -2;
			return ($answer);
		} 
	}
	
	$retval = 1;
	if ( $nameWarn != NULL ) $retval = 2;
	$answer['ok'] = $retval;
	$answer['nameWarn'] = $nameWarn;
	
	return ($answer);
}

}

// -----------------------------------------------------------------

class oExpImgExport {
	
	var $tablemom;
	var $cnt;
	var $go;
	
	/**
	 * 
	 * @var array $infoarr
	 *  'sum' sum of files sizes in bytes
	 */
	var $infoarr;
	
	

function __construct($tablemom, $go, $parx, $cnt, &$flushLib) {
	$this->flushLib = $flushLib;
	$this->tablemom=$tablemom;
	$this->cnt = $cnt;
	$this->go = $go;
	if ( $parx['namepol'] == '') $parx['namepol'] = 'NAME';
	$this->parx = $parx;
	
	$this->onObjLib = new oExpImgExpSing($tablemom, $go, $parx);
	
	$this->namepolArr=array('NAME'=>'Name of object', 'ID'=>'ID of object');
	$this->infoLevArr=array(0=>'quite', 1=>'show only errors', 2=>'show also warnings', 3=>'show all');
}

function gox ($go) {
	
	$goArray   = array( "0"=>"Prepare", 1=>"Analyze", 2=>"Collect images", 3=>"Remove temp files" );
	$extratext = '[<a href="'.$_SERVER['PHP_SELF'].'?tablemom='.$this->tablemom.'">Start again</a>]';
	
	$formPageLib = new FormPageC();
	$formPageLib->init( $goArray, $extratext );
	$formPageLib->goInfo( $go ); 
	echo "<br>";
}

function infox() {
	$parx = $this->parx;
	$tabobj = new visufuncs();
	$dataArr= NULL;
	$dataArr[] = array( "Name policy:", "<b>".$parx["namepol"]."</b>");
	if ($parx["defext"]!="")  $dataArr[] = array( "Default image-extension:", "<b>".$parx["defext"]."</b>");
	$dataArr[] = array( "Info level:", "<b>".$parx["infolev"].':'.$this->infoLevArr[$parx["infolev"]]."</b>");
	
	$headOpt = array( "title" => "Parameters", "headNoShow" =>1);
	$headx   = array ("Key", "Val");
	$tabobj->table_out2($headx, $dataArr,  $headOpt);
}

// manage workdir
function workDir() {
	global $error;

	$workSubDir = "img.list_export";
	$this->workSubDir = $workSubDir;
	
	$workdirObj = new workDir();
	$this->tmpdir   = $workdirObj->getWorkDir ( $workSubDir );
	if ($error->Got(READONLY))  {
		$error->set('createWorkDir()', 2, "Creation of work-dir failed");
		$error->printAll();
		htmlFoot();    # TBD special htmlFoot()
	}
	
	$this->onObjLib->setTempDir($this->tmpdir);
	
	if ( $this->go == 3 ) {
		echo "remove temp files ...<br>";
		$workdirObj->removeWorkDir( );
	}

}

function formshow($parx) {
	
	$tablemom = $this->tablemom;
	$parx = $this->parx;
	
	
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Analyze images";
	$initarr["submittitle"] = "Next &gt;&gt;";
	$initarr["tabwidth"]    = "AUTO";

	$hiddenarr = NULL;
	$hiddenarr["tablemom"] = $tablemom;

	$formobj = new formc($initarr, $hiddenarr, 0);
	
	$fieldx = array ( "title" => "Name policy", 
			"name"  => "namepol",
			"object" => "select",
			'inits' => $this->namepolArr,
			"val"   => $parx['namepol'], 
			"notes" => "use NAME or ID as file name" );
	$formobj->fieldOut( $fieldx );
	
	$fieldx = array ( 
			"title" => "Default extension", 
			"name"  => "defext",
			"object" => "text",
			"val"   => "bmp", 
			"notes" => "add this file extension to UNKNOWN image types (e.g. bmp)" );
	$formobj->fieldOut( $fieldx );
	
	
	$fieldx = array ( "title" => "Infolevel", 
			"name"  => "infolev",
			"object" => "select",
			'inits' => $this->infoLevArr,
			"val"   => $parx['infolev'], 
			"notes" => "level of information" );
	$formobj->fieldOut( $fieldx );

	$formobj->close( TRUE );
}

function help() {
	echo "<br>";
	htmlInfoBox( "Short help", "", "open", "HELP" );
	?>
	<ul>
	<li> Produce ZIP-file from selected image-files; </li>
	<li> Search on server and intranet (if image is not on server). </li>
	<li> If the name of an image has non-file-compatible letters, 
		they will be replaced by "_". This will produce WARNINGS.</li>
	</ul>
	<?
	htmlInfoBox( "", "", "close" );
	echo "<br>";
	
}

function form2() {
	
	
	$tablemom = $this->tablemom;
	$parx = $this->parx;
	
	$initarr   = NULL;
	$initarr["action"]      = $_SERVER['PHP_SELF'];
	$initarr["title"]       = "Start export";
	$initarr["submittitle"] = "Start export NOW!";
	$initarr["tabwidth"]    = "AUTO";

	$hiddenarr = NULL;
	$hiddenarr["tablemom"] = $tablemom;

	$formobj = new formc($initarr, $hiddenarr, 1);
	$formobj->addHiddenParx( $parx );
	$formobj->close( TRUE );
	
}

function _tabrow(&$dataArr) {

	if ( !$this->_tabOpen ) 
	{
		$this->tabobj = new visufuncs();
		$headOpt = array( "title" => "Info table");
		$headx  = array ("#", "ID", "Status","Object-name", "File name", "Notes");
		$this->tabobj->table_head($headx,   $headOpt);
		$this->_tabOpen = 1;
	}
	
	 $this->tabobj->table_row ($dataArr);
}

function oneObj( &$sqlo, $oneObjData ) {
	return ( $this->onObjLib->analyze($sqlo, $oneObjData ) );
}

/**
 * do the loop
 * @param array &files OUTPUT
 */
function doAll(&$sqlo, &$sqlo1, $sqlAfter, &$files ) {
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	$tablemom = $this->tablemom;
	$parx = $this->parx;
	
	$imgcount= 0;
	$sum     = 0;
	$goodCnt = 0;
	$warnCnt = 0;
	$ErrCnt  = 0;
	
	$prgopt['objname']='objects';
	$prgopt['maxnum'] = $this->cnt;
	$this->flushLib->shoPgroBar($prgopt);
	echo "<br>\n";
	
	$sqlsLoop = "SELECT x.IMG_ID, x.NAME, x.MIME_TYPE FROM ".$sqlAfter;
	if ( $tablemom=="EXP" ) {
		$sqlsLoop = "SELECT x.EXP_ID, x.name FROM ".$sqlAfter;
	}
	
	$this->_tabOpen = 0;
	$sqlo->query($sqlsLoop);
	while ($sqlo->ReadRow()) {
		
		$nameWarn = "";

		$oneObjData = $sqlo->RowData;
		$objid   = $oneObjData[0];
		$objname = $oneObjData[1];
		
		$answer = $this->oneObj( $sqlo1, $oneObjData );
		if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 1, 'Error on object-ID:'.$objid );
			break;
		}
		
		$imageOriName = $answer['imgOriName'];
		$nameWarn	  = $answer['nameWarn'];
		$htmlname	  = $answer['htmlname'];
		$IMG_ID  	  = $answer['IMG_ID'];
		$fileSingle   = $answer['fileSingle'];
		$fileSizeB    = $answer['size'];
		
		$dataArr = NULL;
		$dataArr[0]  = $imgcount+1;
		$dataArr[1]  = $objid;
		$dataArr[2]  = NULL;
		$dataArr[3]  = htmlspecialchars($objname);
		$dataArr[4]  = htmlspecialchars($fileSingle);
		$dataArr[5]  = NULL;
		
		$doShow = 0;
		
		if ( $answer['ok'] >0 ) {
			
			$tmpsizeMb = $fileSizeB * 0.000001;
			$dataArr[2]   =  "<font color=green><b>ok</b></font>";
			if ( $answer['ok'] ==2 ) 
			{
				$dataArr[2] = "<font color=#D04040><b>Warning</b></font>";
				$dataArr[5] = $nameWarn.'; ';
				$warnCnt++;
				if ($parx['infolev']>1) $doShow = 1;
			}
			$dataArr[5] .=  $answer['infoText'].", size:$tmpsizeMb Mb";
			
			$sum = $sum + $tmpsizeMb;
			$goodCnt++;
			$files[] = $fileSingle;
			if ($parx['infolev']>2) $doShow = 1;
			
		} else {
			$dataArr[2] = "<font color=red><b>Error</b></font>";
			$dataArr[5] = $answer['errmsg'];
			if ($parx['infolev']>0) $doShow = 1;
			$ErrCnt++;
		}
		
		if ( $doShow ) $this->_tabrow($dataArr);
		$this->flushLib->alivePoint($imgcount);
		$imgcount++;
	}
	
	if ( $this->_tabOpen ) $this->tabobj->table_close();
	
	$this->flushLib->alivePoint($imgcount, 1);
	
	echo "<br>";
	
	$this->infoarr['sum'] = $sum;
	
	$tmparr = NULL;
	$tmparr[] = array ("Analysed images:", "<b>$imgcount</b>");
	$tmparr[] = array ( "<font color=green><b>Good images:</b></font>", "<b>$goodCnt</b>");
	if ($warnCnt>0) $tmparr[] = array ( "<font color=#803000>Warnings:</font> ", "<b>$warnCnt</b>");
	if ($ErrCnt>0) $tmparr[] = array ( "<font color=red>Error images:</font> ", "<b>$ErrCnt</b>");
	$tmparr[] = array ( "Sum: ", "<b>$sum</b> Mb");
	$optx = array("title"  =>"Results");
	$dummy=NULL;
	$tabobj = new visufuncs();
	$tabobj->table_out( $dummy, $tmparr,  $optx);
	echo "<br>";
	
	return ($goodCnt);
}

/**
 * analyse, if enough space is available
 * 
 */
function spaceAnalysis() {
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	$diskspaceExpect = $this->infoarr['sum'] *1000000 * 2; // calculate space for images + TAR-file
	
	$worklib = new workDir();
	$info = $worklib->spaceAnalysis($diskspaceExpect);
	
	echo "Info: free space on WORK-dirk: ".($info['free'] * 0.000001)." MBytes<br>";
}


// function _tarx($path, $files, $tarfile) {
// 	$tar = new Tar();
// 	$tar->create( $tarfile );       // create tar file
// 	$tar->cd($path . "/");              // change to path for taring

// 	// read file by file from dir and add to tar archive. catch readdir
// 	// returns '.', '..', xml file or zip file (?)
	
// 	$prgopt=NULL;
// 	$prgopt['maxnum'] = count($files);
// 	$this->flushLib->setNewLimits($prgopt);
	
// 	$imgcount=0;
// 	foreach( $files as $dummy=>$filename) {
// 		$tar->add($path . "/" . $filename);
// 		$this->flushLib->alivePoint($imgcount);
// 		$imgcount++;
// 	}
// 	$tar->close(); 
// 	$this->flushLib->alivePoint($imgcount,1);
// }

function _zip($path, $files, $tarfile) {

	$ziplib = new myZip();
	$ziplib->zipall($path, $files, $tarfile);
	
}

// organize TAR
function tarx( &$files ) {
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	$diskspaceExpect = $this->infoarr['sum'] *1000000; // calculate space for TAR-file
	$worklib = new workDir();
	$info = $worklib->spaceAnalysis($diskspaceExpect);
	
	if ($error->Got(READONLY))  {
		$error->set( $FUNCNAME, 1, 'Error on WorkDir space analysis.' );
		return;
	}
	
	$tmpdir = $this->tmpdir;

	$this->tarfileSingle = "imgfiles.zip";
	$tarfile = $tmpdir."/".$this->tarfileSingle;
	$this->tarfile = $tarfile;

	// $this->_tarx( $tmpdir, $files, $tarfile );
	$this->_zip( $tmpdir, $files, $tarfile );

	if ( ($fhd = fopen($tarfile , "r")) == FALSE) {
		htmlFoot("Error", "ERROR: could not read ZIP-file ".$tarfile);
	}
	fclose($fhd);
	
	//$ret = $compressObj->zip($tarfile);
	//if ( $ret>0 ) $tarfileSingle = $tarfileSingle . ".gz";
	echo "Download Ready ...<br>\n";
}

function downLoadLink() {
	
	$tarfileSingle = $this->tarfileSingle;

	$tmpsizeMb = filesize($this->tarfile) * 0.000001;
	echo "<br><b>1.</b> <a href=\"f_workfile_down.php?name=".$tarfileSingle."&file=".
		 $this->workSubDir."/".$tarfileSingle."\">".
		 "<img src=\"images/ic.docdown.big.gif\" border=0> ".
		 "<B>Download-file</B></a> (size: <b>$tmpsizeMb</b> Mb)<br>\n";
	echo "<br><b>2.</b> <a href=\"".$_SERVER['PHP_SELF']."?go=3&tablemom=".$this->tablemom."\"><B>Clean temporary files</B></a><br>\n";
}

}


/* Open connection to DBMS */
$varcol   = & Varcols::get();
$error    = & ErrorHandler::get();
$sqlo     =  logon2( $_SERVER['PHP_SELF'] );
$sqlo1    =  logon2(  );


$tablemom = $_REQUEST["tablemom"];
$go= $_REQUEST["go"];
$parx= $_REQUEST["parx"];

if ($tablemom=="") $tablemom	  = "IMG";

//$compressObj = new compressC();
$flushLib 		= new fProgressBar( ); 

if ( $tablemom=="EXP" ) {
	
	$title = "Export image-files of selected experiments";
	$infoarr			 = NULL;
	$infoarr["scriptID"] = "";
	$infoarr["title"]    = $title;
	$infoarr['obj_name'] = 'EXP';

} else {
	$title = "Export image-files for a list of images";
	$infoarr			 = NULL;
	$infoarr["scriptID"] = "";
	$infoarr["title"]    = $title;
	$infoarr['obj_name'] = 'IMG';
}

	$infoarr["form_type"]= "list";
$infoarr["obj_cnt"]  = 1;
$infoarr["title_sh"] = 'Export image-files';
$infoarr["css"]  =  $flushLib->getCss() ;
$infoarr["javascript"]  =  $flushLib->getJS() ;

htmlFoot("ERROR", 'Currently not supported. Ask Admin.');

$pagelib = new gObjTabPage($sqlo, $tablemom );
$pagelib->showHead($sqlo,$infoarr);
$pagelib->initCheck( $sqlo );
$sqlAfter  = $pagelib->getSqlAfter();
echo "<ul>";

$headarr = $pagelib->headarr;
$mainlib = new oExpImgExport($tablemom, $go, $parx, $headarr['obj_cnt'], $flushLib);
$mainlib->gox($go);
$mainlib->workDir();
if ( $go==3 ) {
	htmlFoot("<hr>");
}
	
if ( !$go ) {
	$mainlib->help();
	$mainlib->formshow($parx, $tablemom);
	htmlFoot();
}


$mainlib->infox();
echo "<br>";

$files   = NULL;
$goodCnt = $mainlib->doAll($sqlo, $sqlo1, $sqlAfter, $files );
$pagelib->chkErrStop();

if ($go==1) {

	$mainlib->spaceAnalysis();
	$pagelib->chkErrStop();
	
	$mainlib->form2();
	echo "<br>";
}

if ( !$goodCnt ) {
	htmlFoot("Info", "No images collected!");
}

if ($go==1) {
	htmlFoot("<br><hr>");
}

$mainlib->tarx($files);
$pagelib->chkErrStop();
$mainlib->downLoadLink();

htmlFoot("<br><hr>");
