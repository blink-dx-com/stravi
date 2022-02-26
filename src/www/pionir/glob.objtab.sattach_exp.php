<?
/**
 * export attachements of selection of objects
 * @namespace core::obj:SATTACH
 * @package glob.objtab.sattach_exp.php
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq UREQ:0002120: o.SATTACH : provide export of all attachments of selected objects 
 * @param 
 * 	  $tablename: [IMG], "EXP"
  	  $go	: 0 html form
			: 1 analyse
	  	     : 2 export
			: 4 clear temp-dir
	  $parx["defext"]
			'namepol' name policy : ['NAME'], 'ID'
			'infolev' : 1,2,3
 * @version $Header: trunk/src/www/pionir/glob.objtab.sattach_exp.php 59 2018-11-21 09:04:09Z $
 */
session_start(); 


require_once ('reqnormal.inc');
require_once('glob.objtab.page.inc');
require_once('object.subs.inc');
require_once('down_up_load.inc');
require_once("sql_query_dyn.inc");
require_once("f.workdir.inc");  
require_once("Tar.inc");  
require_once("f.compress.inc");  
require_once("visufuncs.inc");
require_once("func_form.inc");
require_once('f.progressBar.inc');
require_once('class.filex.inc');
require_once ("o.SATTACH.subs.inc");


// export ALL attachments of ONE object
class gObj_attachExp {

function __construct($tablemom, $go, $parx) {
	$this->tablemom=$tablemom;
	$this->go = $go;
	$this->parx = $parx;
	$this->satObj     = new cSattachSubs();
	$this->fileHelpLib = new fileC();
}

function setTempDir($tmpdir) {
	$this->tmpdir = $tmpdir;
}

function _convertToFileName($badname) {
	$goodname = $this->fileHelpLib->objname2filename( $badname );
	return $goodname;
}

/**
 * 
 * @param unknown_type $sqlo
 * @param unknown_type $rel_id
 * @return array($fileSizeTmp,$filename_nice)
 */
function _oneAttachment(&$sqlo, $rel_id) {
	global $error;
	$FUNCNAME= $this->__CLASS__.':_oneAttachment';
	
	$go = $this->go;
	$tablename  = $this->tablemom;
	$objid  =$this->objid;
	
	$attachFile = $this->satObj->getDocumentPath($tablename, $objid, $rel_id);
	
	if( !file_exists($attachFile) ) {
	  return;
	}
	
	$fileSizeTmp = filesize($attachFile);
	
	$ret = $sqlo->query("SELECT name, mime_type FROM sattach WHERE table_name='".$tablename."' AND OBJ_ID=".$objid." AND REL_ID=".$rel_id);
	$sqlo->ReadRow();
	$filename_nice = $sqlo->RowData[0];

	
	
	if ( $go==2 ) {
		
		$objDir =$this->objDir;
		$fileDestFull = $objDir."/".$filename_nice;
		if (!copy ($attachFile, $fileDestFull) ) {
			$error->set( $FUNCNAME, 1, 'Copy to temp-file failed. Attachment-Name:'.$filename_nice.' dest:'.$fileDestFull );
			return;
		} 
		
	}
	return array($fileSizeTmp,$filename_nice);
}

/**
 * create dir
 * @param unknown $objname
 * @param unknown $objid
 */
private function _create_dir($objname, $objid) {
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	$tmpdir   = $this->tmpdir;
	
	// NICE_DIR = Object-name ".ID_".Object-ID
	// example: co_H2pol_3900_20171213_1_ID_321211
	$niceDirName = $this->_convertToFileName($objname).'.ID_'.$objid;
	$this->objDir = $tmpdir."/".$niceDirName;
	
	if ( !mkdir ($this->objDir) ) {
		$error->set( $FUNCNAME, 1, 'Could not create temp-directory: '.$this->objDir );
		return;
	}
	
	
	return $niceDirName;
}

/**
 *  analyze one object
 *  @return: $objAttachArr: [] = array('subdir'=>$niceDirName, 'files'=>$objAttachArr)
 */
function analyze( &$sqlo, &$sqlo2, $objid ) {
	global $error;
	$FUNCNAME= $this->__CLASS__.':analyze';
	
	$this->objid = $objid;
	$niceDirName = NULL;
	$parx	  = $this->parx;
	$tablename= $this->tablemom;
	$go 	  = $this->go;
	
	$objname  = obj_nice_name ( $sqlo, $tablename, $objid ); 

	
	$first_attachment=1;
	$objAttachArr=NULL;
	$sqlo2->query("SELECT REL_ID FROM SATTACH WHERE TABLE_NAME='".$tablename."' AND OBJ_ID=".$objid. " ORDER BY REL_ID");
	while ($sqlo2->ReadRow() ) {
		
		if ( $first_attachment and $go==2 ) {
			$niceDirName = $this->_create_dir($objname, $objid);
			if ($error->Got(READONLY))  {
				$error->set( $FUNCNAME, 1, 'object: '.$objid.' : problem ocurred.' );
				return;
			}
		}
	
		$relid = $sqlo2->RowData[0];
		$fileInfo = $this->_oneAttachment($sqlo, $relid);
		if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 1, 'object: '.$objid.' : problem ocurred.' );
			return;
		}
 		if ($fileInfo!=NULL) $objAttachArr[]=$fileInfo;
 		$first_attachment=0;
	}

	return array('subdir'=>$niceDirName, 'files'=>$objAttachArr);
}

}

// -----------------------------------------------------------------

class oSATTACH_objListExp {

function __construct($tablemom, $go, $parx, $cnt, &$flushLib) {
	$this->flushLib = $flushLib;
	$this->tablemom=$tablemom;
	$this->cnt = $cnt;
	$this->go = $go;
	if ( $parx['namepol'] == '') $parx['namepol'] = 'NAME';
	if ( !$parx['infolev']) $parx['infolev'] = 3;
	$this->parx = $parx;
	
	$this->onObjLib = new gObj_attachExp($tablemom, $go, $parx);
	
	$this->namepolArr=array('NAME'=>'Name of object', 'ID'=>'ID of object');
	$this->infoLevArr=array(1=>'quite', 2=>'show only errors', 3=>'show all');
}

function gox ($go) {
	
	$goArray   = array( "0"=>"Prepare", 1=>"Analyze", 2=>"Collect attachments", 3=>"Remove temp files" );
	$extratext = '[<a href="'.$_SERVER['PHP_SELF'].'?tablename='.$this->tablemom.'">Start again</a>]';
	
	$formPageLib = new FormPageC();
	$formPageLib->init( $goArray, $extratext );
	$formPageLib->goInfo( $go ); 
	echo "<br>";
}

function infox() {
	return; // not needed till now
	
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
	$hiddenarr["tablename"] = $tablemom;

	$formobj = new formc($initarr, $hiddenarr, 0);
	
	/*
	$fieldx = array ( "title" => "Name policy", 
			"name"  => "namepol",
			"object" => "select",
			'inits' => $this->namepolArr,
			"val"   => $parx['namepol'], 
			"notes" => "use NAME or ID as file name" );
	$formobj->fieldOut( $fieldx );
	*/
	
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
	<li> Produce TAR-file from attachments; </li>
	
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
	$hiddenarr["tablename"] = $tablemom;
	
	$formobj = new formc($initarr, $hiddenarr, 1);
	$formobj->addHiddenParx( $parx );
	$formobj->close( TRUE );
	
}

function _tabrow(&$dataArr) {

	if ( !$this->_tabOpen ) 
	{
		$this->tabobj = new visufuncs();
		$headOpt = array( "title" => "Info table");
		$headx  = array ("#", "Status","Object-ID", "Object-name", "Attachment", 'Size');
		$this->tabobj->table_head($headx,   $headOpt);
		$this->_tabOpen = 1;
	}
	
	 $this->tabobj->table_row ($dataArr);
}


/**
 * do the loop
 * @param array &files OUTPUT
 */
function doAll(&$sqlo, &$sqlo2, &$sqlo3, $sqlAfter, &$files ) {
	global $error;
	$FUNCNAME= $this->__CLASS__.':doAll';
	$tablemom = $this->tablemom;
	$parx = $this->parx;
	
	$objcount= 0;
	$sum     = 0;
	$goodCnt = 0;
	$warnCnt = 0;
	$ErrCnt  = 0;
	
	$prgopt['objname']='objects';
	$prgopt['maxnum'] = $this->cnt;
	$this->flushLib->shoPgroBar($prgopt);
	echo "<br>\n";
	
	$pkname = PrimNameGet2($tablemom);
	$namecol= importantNameGet2($tablemom);
	
	$sqlsLoop = "SELECT x.".$pkname.", x.".$namecol." FROM ".$sqlAfter;
	$allcnt=0;
	$this->_tabOpen = 0;
	$sqlo3->query($sqlsLoop);
	while ($sqlo3->ReadRow()) {
		$nameWarn = "";
		
		$oneObjData = $sqlo3->RowData;
		$objid   = $oneObjData[0];
		$objname = $oneObjData[1];
		
		$fileInfo = $this->onObjLib->analyze($sqlo, $sqlo2, $objid );
		if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 1, 'Problem ocurred.' );
			return;
		}
		
		// array('subdir'=>$niceDirName, 'files'=>$objAttachArr)
		$filearr = &$fileInfo['files'];
		$subdir  = $fileInfo['subdir'];
		
		if (!sizeof($filearr)) {
			$objcount++;
			continue;
		}
	

		$filecnt=0;
		reset ($filearr);
		foreach( $filearr as $dummy=>$valarr) {
			
			$fileSizeByte  = $valarr[0];
			$filename      = $valarr[1];
			$fileWithDir = $subdir.'/'.$filename;
			$fileFull  = $this->tmpdir.'/'.$fileWithDir;
			
			$dataArr = NULL;
			$dataArr[0]  = $allcnt+1;
			$dataArr[1]  =  "<font color=green><b>ok</b></font>";
			$dataArr[2]  =  NULL;
			$dataArr[3]  =  NULL;
			if (!$filecnt) {
				 $dataArr[2]  = $objid;
				 $dataArr[3]  = htmlspecialchars($objname);
			}
			$dataArr[4]  = htmlspecialchars($filename);
			
			$tmpsizeMb  = $fileSizeByte * 0.000001;
			$dataArr[5] =  $tmpsizeMb ." Mb";
			
			$sum = $sum + $tmpsizeMb;
			$goodCnt++;
			$files[] = $fileWithDir;
			if ($parx['infolev']>2) $doShow = 1;
			
			if ( $doShow ) {

				$this->_tabrow($dataArr);
			}
			$this->flushLib->alivePoint($objcount);
			$filecnt++;
			$allcnt++;
			
		}
		reset ($filearr); 
		
		$objcount++;
		
	}	
		
	
	
	if ( $this->_tabOpen ) $this->tabobj->table_close();
	
	$this->flushLib->alivePoint($objcount, 1);
	
	echo "<br>";
	$tmparr = NULL;
	$tmparr[] = array ("Analysed objects:", "<b>$objcount</b>");
	$tmparr[] = array ( "<font color=green><b>Attachments:</b></font>", "<b>$allcnt</b>");
	//if ($warnCnt>0) $tmparr[] = array ( "<font color=#803000>Warnings:</font> ", "<b>$warnCnt</b>");
	//if ($ErrCnt>0) $tmparr[] = array ( "<font color=red>Error objects:</font> ", "<b>$ErrCnt</b>");
	$tmparr[] = array ( "Sum: ", "<b>$sum</b> Mb");
	$optx = array("title"  =>"Results");
	$dummy=NULL;
	$tabobj = new visufuncs();
	$tabobj->table_out( $dummy, $tmparr,  $optx);
	echo "<br>";
	
	return ($goodCnt);
}


function _tarx($path, $files, $tarfile) {
	$tar = new Tar();
	$tar->create( $tarfile );       // create tar file
	$tar->cd($path . "/");              // change to path for taring

	// read file by file from dir and add to tar archive. catch readdir
	// returns '.', '..', xml file or zip file (?)
	
	foreach( $files as $dummy=>$filename) {
		$tar->add($path . "/" . $filename);
	}
	$tar->close(); 
}

/**
 * ZIP it
 * @param string $path - the TEMP_DIR path
 * @param array $files
 * @param string $tarfile
 */
private function _zipall($path, $files, $tarfile) {
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;

	$zip = new ZipArchive();
	if ($zip->open($tarfile, ZipArchive::CREATE)!==TRUE) {
		$error->set( $FUNCNAME, 1, 'can not create ZIP-file:'.$tarfile );
		return;
	}
	
	
	$dir_cache=array();

	// read file by file from dir and add to tar archive. catch readdir
	// returns '.', '..', xml file or zip file (?)

	foreach( $files as $dummy=>$filename_with_dir) {
		
		if (strstr($filename_with_dir,'/')!=NULL) {
			$patharr = explode("/",$filename_with_dir);
			$this_dir = $patharr[0];
			if (!array_key_exists($this_dir, $dir_cache)) {
				//echo "- DEBUG: NEW DIR: $this_dir<br>";
				$zip ->addEmptyDir($this_dir);
				$dir_cache[$this_dir]=1;
			}
		}
		
		$ori_file = $path . "/" . $filename_with_dir;
		//echo "- DEBUG: FILE: $filename_with_dir; ORI:$ori_file|<br>";
		
		if (!$zip->addFile( $ori_file,  $filename_with_dir )) {
			echo "DDD: addFIle problem<br>\n";
		}
		
	}
	$zip->close();
}

// organize TAR
function tarx( &$files ) {
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;

	$tmpdir = $this->tmpdir;

	$this->tarfileSingle = "attachments.zip"; // .tar
	$tarfile = $tmpdir."/".$this->tarfileSingle;
	$this->tarfile = $tarfile;

	//$this->_tarx( $tmpdir, $files, $tarfile );
	$this->_zipall( $tmpdir, $files, $tarfile );
	if ($error->Got(READONLY))  {
		$error->set( $FUNCNAME, 1, 'Errors on ZIP.' );
		return;
	}

	if ( ($fhd = fopen($tarfile , "r")) == FALSE) {
		$error->set( $FUNCNAME, 1, "could not read ZIP-file ".$tarfile );
		return;
	}
	fclose($fhd);
	
	//$ret = $compressObj->zip($tarfile);
	//if ( $ret>0 ) $tarfileSingle = $tarfileSingle . ".gz";
	echo "ZIP Ready ...<br>\n";
}

function downLoadLink() {
	
	$tarfileSingle = $this->tarfileSingle;

	$tmpsizeMb = filesize($this->tarfile) * 0.000001;
	echo "<br><b>1.</b> <a href=\"f_workfile_down.php?name=".$tarfileSingle."&file=".
		 $this->workSubDir."/".$tarfileSingle."\">".
		 "<img src=\"images/ic.docdown.big.gif\" border=0> ".
		 "<B>Download ZIP-file</B></a> (size: <b>$tmpsizeMb</b> Mb)<br>\n";
	echo "<br><b>2.</b> <a href=\"".$_SERVER['PHP_SELF']."?go=3&tablename=".$this->tablemom."\"><B>Clean temporary files</B></a><br>\n";
}

}


/* Open connection to DBMS */
$varcol     = & Varcols::get();
$error      = & ErrorHandler::get();
$sqlo  = logon2( $_SERVER['PHP_SELF'] );
$sqlo2 = logon2( $_SERVER['PHP_SELF'] );
$sqlo3 = logon2( $_SERVER['PHP_SELF'] );
$infox 		  = NULL;
$inf_classarr = NULL;

$parx = $_REQUEST['parx'];
$tablename 	= $_REQUEST['tablename'];
$go 		= $_REQUEST['go'];

$compressObj = new compressC();
$flushLib 		= new fProgressBar( ); 

$title = "Export attachments for a list of objects";
$infoarr			 = NULL;
$infoarr["scriptID"] = "";
$infoarr["title"]    = $title;
$infoarr['obj_name'] = $tablename;
$infoarr["form_type"]= "list";
$infoarr["obj_cnt"]  = 1;
$infoarr["title_sh"] = 'Export attachements';
$infoarr["css"]  =  $flushLib->getCss() ;
$infoarr["javascript"]  =  $flushLib->getJS() ;
$infoarr["help_url"]  = 'o.SATTACH.html';

$pagelib = new gObjTabPage($sqlo, $tablename );
$pagelib->showHead($sqlo,$infoarr);
$pagelib->initCheck( $sqlo );
$sqlAfter  = $pagelib->getSqlAfter();
echo "<ul>";


$headarr = $pagelib->headarr;
$mainlib = new oSATTACH_objListExp($tablename, $go, $parx, $headarr['obj_cnt'], $flushLib);
$mainlib->gox($go);
$mainlib->workDir();
if ( $go==3 ) {
	htmlFoot("<hr>");
}
	
if ( !$go ) {
	$mainlib->help();
	$mainlib->formshow($parx, $tablename);
	htmlFoot();
}

if ( $go==1 ) {
	$mainlib->form2();
	echo "<br>";
}

$mainlib->infox();
echo "<br>";

$files = NULL;
$goodCnt = $mainlib->doAll($sqlo, $sqlo2, $sqlo3, $sqlAfter, $files );
$pagelib->chkErrStop();

if ( !$goodCnt ) {
	htmlFoot("Info", "No objects collected!");
}

if ($go==1) {
	htmlFoot("<br><hr>");
}

$mainlib->tarx($files);
$pagelib->chkErrStop();

$mainlib->downLoadLink();

htmlFoot("<br><hr>");
