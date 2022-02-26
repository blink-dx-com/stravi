<?php

require_once ("objlib/RInterface.inc");
require_once ('f.wiid_roid.inc');
require_once ('o.LINK.subs.inc');
require_once ('javascript.inc');
require_once ('o.S_VARIO.mod.inc');
require_once ('object.info.inc');
require_once 'o.SATTACH.subs.inc';

/**
 * RcodeExecuter - library; called by obj.link.c_Rcode.exe.php
 * - execute Rcode,   stored as attachment-file in object LINK ($id)
 * - input-data-file: $datafile
 * - produces various output files on USER_SESSIONDIR/R.pack
 * - if IMAGES are produced: directly show them
 * - if Cyto_Statistics.pdf is produced: directly forward to PDF download
 *
 * $Header: trunk/src/www/pionir/obj.link.c_Rcode.exe.inc 59 2018-11-21 09:04:09Z $
 * @package obj.link.c_Rcode.exe.inc
 * @author  Steffen Kube
 * @swreq UREQ:0010165 FS-LIM07-A01 Provide a basic R interface
 * @unittest EXISTS
 */
class oLINK_cRcodeExe {
	
	private $workdir;


/**
 * construct
 * @param unknown $sqlo
 * @param string $datafile: name data file for RInput (relative to USER_SESSIONDIR )
 * @param int $link_id : ID of Rcode document
 */
function __construct( &$sqlo, $datafile, $link_id ) {
	global  $error;
    $FUNCNAME='oLINK_cRcodeExe';
    
    $this->R_WORKDIR_NAME = "R.pack";
    
    $this->datafile = $datafile;
    $this->link_id = $link_id;
    
    $this->rcode = linkpath_get($link_id);
    
    $workdirObj    = new workDir();
    $this->datafileAbs   = $workdirObj->getAbsDir ($datafile);
    
    
    
    $this->workdir = NULL;
    
}

/**
 * get relative sub-dirname of R working environment
 * @return string
 */
public function get_rel_subdirname() {
	return $this->subDirName;
}

/**
 * update usage of tool in VARIO:R-Code-usage
 * @param unknown $sqlo
 * @param unknown $link_id
 */
private function updateUsageOfCode(&$sqlo, $link_id) {
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	$variolib = new oS_VARIO_mod();
	
	$option = array('noAccCheck'=>1);
	$variolib->setObject( $sqlo, 'LINK', $link_id, $option );
	$variolib->isTouched = 1; // do not log this action in the mod-log !!!
	$key='R-Code-usage';
	$val = date ("Y-m-d\TH:i:s", time()) .' '.$_SESSION['sec']['appuser'];
	$variolib->updateKeyVal($sqlo, $key, $val);
	
	if ($error->Got(READONLY))  {
		// ignore errors here !!!!
		$error->reset();
		return;
	}
}

/**
 * initial check of input parameters
 * @param unknown $sqlo
 */
public function checkR( &$sqlo ) {
	global $error, $varcol;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	$datafile = $this->datafile;
	$link_id  = $this->link_id;
	
	if ( !file_exists( $this->datafileAbs ) ) {
		$error->set( $FUNCNAME,1, 'input datafile "SESSION_DIR/'.$datafile.'" does not exist.');
		return;
	}
	
	if (!file_exists($this->rcode))
	{
		$error->set( $FUNCNAME,2, 'Rcode-file not found on DOC-ID:'.$link_id);
		return;
	}
	
	$classname='Rcode';
	$class_id_Rcode	= $varcol->class_name_to_id( 'LINK', $classname );
	if (!$class_id_Rcode) {
		$error->set( $FUNCNAME,3, 'Document class "'.$classname.'" missing in the system. Please inform your admin!');
		return;
	}
	
	$objInfoLib = new objInfoC();
	$doc_params = $objInfoLib->getParamsRaw ( $sqlo, 'LINK', $link_id);
	
	if ( !isset($doc_params["xobj"]['extra_class_id'])) {
		$error->set( $FUNCNAME, 4, 'Rcode document (ID:'.$link_id.') must be of class "'.$classname.'". Please modify the Rcode-document.');
		return;
	}
	
	if ( $class_id_Rcode != $doc_params["xobj"]['extra_class_id']) {
		$error->set( $FUNCNAME, 5, 'Rcode document (ID:'.$link_id.') must be of class "'.$classname.'". Please modify the Rcode-document.');
		return;
	}
	
	
	
	// update usage statistic
	$this->updateUsageOfCode($sqlo, $link_id);
}


/**
 * add all found attachments from document to Rcode-Temp-Dir
 * @param unknown $sqlo
 */
private function add_doc_attachments(&$sqlo) {
	global $error, $varcol;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	$workdir = $this->workdir;
	
	$attach_lib = new cSattachSubs();
	$attachments = $attach_lib->getRelArrByKey( $sqlo, 'LINK', $this->link_id, NULL );
	if (!sizeof($attachments)) return;
	
	foreach($attachments as $relid) {
		
		$docpath = $attach_lib->getDocumentPath('LINK', $this->link_id, $relid);
		if ( file_exists($docpath) ) {
			// add to temp path
			$attach_name = $attach_lib->getName($sqlo, 'LINK', $this->link_id, $relid);
			
			$one_filename      = $attach_name;
			$one_tmp_file_full = $workdir . $one_filename;
			if ( !copy( $docpath, $one_tmp_file_full ) ) {
				$error->set( $FUNCNAME,2, 'Copy of attachment "'.$one_filename.'" to the rCode-Temp-Dir failed.' );
				return;
			}
			
			if ($_SESSION["userGlob"]["g.debugLevel"]>1) {
				 debugOut('add attachment: '.$attach_name, $FUNCNAME);
			}
		}
	}
	
	
}

/**
 * - exec R code
 * - create work dir
 * @param unknown $sql
 * @return array info-array
 *    'R-workdir'=> $RObj->temp
 */
function execR( &$sql ) {
	global  $error;
    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
    
    // get work-path
    $workdirObj = new workDir();
    $workdir   = $workdirObj->getWorkDir ($this->R_WORKDIR_NAME);
    $workdir   .= "/";
    
    $this->subDirName = $workdirObj->subDirName;
    
    $this->workdir = $workdir;
    
    $inputFile = $workdir."RInput";
    
    if ( !copy( $this->datafileAbs, $inputFile ) ) {
        $error->set( $FUNCNAME,2, "copy of tempfile to RInput failed." );
        return;
    }
	
    if ( !file_exists($inputFile)) {
    	$error->set( $FUNCNAME,5, 'RInput file "'.$inputFile.'" not found.' );
        return;
    }
    
    $this->add_doc_attachments($sql);
    if ($error->Got(READONLY))  {
    	$error->set( $FUNCNAME,5, "Copy of Attachment-Files failed." );
    	return;
    }
    
    if ($_SESSION["userGlob"]["g.debugLevel"]>1) {
    	debugOut('input-data-file: '.$inputFile, $FUNCNAME);
    }
    
    $sqls   =  "select VALUE from GLOBALS where NAME='exe.R'";
    $sql    -> query($sqls);
    $sql    -> ReadRow();   
    $rtool  =  $sql->RowData[0];    
    
    // RInterface
    
    $RObj 	= new RInterface($rtool, '');
    
    if (!$RObj->prepare2($workdir)) {
        //$error->printLast();
    }
    
    if (!$RObj->prepareRScript($this->rcode, "RInterface.rcode")) { // old: "objtools/RInterface.rcode"
        $error->set( $FUNCNAME,3, "R prepare failed.");
        //return;
    }   
    if (!$RObj->runRScript()) {
        $error->set( $FUNCNAME,4, "running R failed.");
        //return;
    }
    
    return array( 'R-workdir'=> $RObj->temp);
}	

/**
 * build and show results of R execution
 * @param array $options 
 *   'images_no_show' => 0,1
 */
public function buildFileTable ($options=array() ) {
	
	
	$workdir = $this->workdir;
	$images_show = 1;
	if ( $options['images_no_show']>0 ) $images_show = 0;
	
	//
	// directly show images ?
	//
	if ( $images_show ) {
		
		$aImages = array('.gif', '.jpg', '.png');
		$images = NULL;
		if ($handle = opendir($workdir)) {
			while (false !== ($file = readdir($handle))) {
				#$fileArr[] = $file;
				foreach ($aImages as $value) {
					if (stristr($file, $value) !== FALSE) $images[] = $file;
				}
			}
			if(!empty($images)) {
				foreach($images as $value) {
					$srcFile = $workdir.$value;
					$imageSize = getimagesize($srcFile);
					$xDim = $imageSize[0];
					$yDim = $imageSize[1];
					echo '<img src="f.image_show.php?filename='.$srcFile.'&dim[0]='.$xDim.'&dim[1]='.$yDim.'" /><br />';
				}
				// return;
			}
		}
	}
	
	$outfile = "Cyto_Statistics.pdf";
    $fileRelPath = $this->R_WORKDIR_NAME.'/'.$outfile;
    
    $headx  = array ('File', 'Size', 'Option');
    $headOpt = array ("title"=>'generated files');
    require_once ("visufuncs.inc");
    $this->tabobj3 = new visufuncs();
    $this->tabobj3->table_head($headx, $headOpt);
    $url = NULL;
    // show link to pdf
    echo '<br />';
	if ($handle = opendir($workdir)) {
		$defaultfiles = array('.','..');
	    	while (false !== ($file = readdir($handle))) {
	    		if (!in_array($file, $defaultfiles)){
					$filesize = filesize($workdir . $file);
					if ($filesize > 1023){
						if ($filesize < 1048576){
							$filesize = ceil($filesize / 1024) . " K";
						}else{
							$filesize = ceil($filesize / 1048576) . " M";
						}
					} else{
						$filesize .= " "; 
					}
		        	$fileRelPath2 = $this->R_WORKDIR_NAME.'/'.$file;
		        	$dataArr=array($file, $filesize . 'Byte', '<a href="f_workfile_down.php?file='.$fileRelPath2.'">download</a>');
					$this->tabobj3->table_row($dataArr);
					$lisoffiles .= " " .$workdir . $file;
	    		}
	    		if ($file == $outfile ){
   					$url = 'f_workfile_down.php?file='.$fileRelPath2;
	    		}
	    }
	}
	
	$zipcmd  = "zip -j ".$workdir."data". $lisoffiles;
	$answer  =  exec ( $zipcmd );
	$zipFile = $this->R_WORKDIR_NAME.'/data.zip';
	// echo '<br />';
	if ( file_exists($workdir."data.zip") ) {
		$dataArr=array('ALL as ZIP', '', '<a href="f_workfile_down.php?file='.$zipFile.'">Download ALL</a>');
		$this->tabobj3->table_row($dataArr);
	}
	$this->tabobj3->table_close();
	
    echo "\n";
    if ($url != NULL and filesize($workdir . "RError") == 0) {
    	js__location_replace($url, 'RCode result file "'.$fileRelPath.'"' );
    }
}

}
