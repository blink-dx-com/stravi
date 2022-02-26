<?php 
require_once ("Tar.inc");   
require_once ('down_up_load.inc');
require_once ('class.filex.inc');

/**
 * g > collect attachments and export 
 * build TAR-ball
 * @package o.SATTACH.exp.inc 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq   SREQ:0003390: g > collect attachments and export 
 */
class oSATTACH_CollectC {
	
	var $objAttachArr; /**
	 array() = array( size, filename)
	*/
	var $go; /**
		0 : analyse
		2 : copy files for TAR operation
	*/
	
	/**
	 * construct
	 */
	function __construct() {
		
		$this->attachSubLib = new cSattachSubs();
	}
	
	
	function setTempDir($tmpdir) {
		$this->tmpdir = $tmpdir;
	}
	
	private function _convertToFileName($badname) {
		$goodname = $this->fileHelpLib->objname2filename( $badname );
		return $goodname;
	}
	
	/**
	 * 
	 * @param object $sqlo
	 * @param long   $rel_id
	 * @return array($fileSizeTmp,$filename_nice)
	 */
	private function _oneAttachment(&$sqlo, $tablename, $objid, $rel_id) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$go = $this->go;
		
		$objDir =$this->objDir;
		
		$attachFile = $this->satObj->getDocumentPath($tablename, $objid, $rel_id);
		
		if( !file_exists($attachFile) ) {
		  return;
		}
		
		$fileSizeTmp = filesize($attachFile);
		
		$ret = $sqlo->query("SELECT name, mime_type FROM sattach WHERE table_name='".$tablename."' AND OBJ_ID=".$objid." AND REL_ID=".$rel_id);
		$sqlo->ReadRow();
		$filename_nice = $sqlo->RowData[0];
	
		$fileDestFull = $objDir."/".$filename_nice;
		
		if ( $go==2 ) {
			if (!copy ($attachFile, $fileDestFull) ) {
				$error->set( $FUNCNAME, 1, 'Copy to temp-file failed. Attachment-Name:'.$filename_nice );
				return;
			} 
		}
		return array($fileSizeTmp,$filename_nice);
	}

	/**
	 * collect one attachment
	 * @param  $sqlo
	 * @param  $table
	 * @param  $objid
	 * @param  $attachRelID
	 */
	public function collectOne( &$sqlo, $table, $objid, $attachRelID) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$fileInfo = $this->_oneAttachment($sqlo, $table, $objid, $attachRelID);
		if ($error->Got(READONLY))  {
			$error->set( $FUNCNAME, 1, 'object: '.$objid.' : problem ocurred.' );
			return;
		}
 		if ($fileInfo!=NULL) $this->objAttachArr[]=$fileInfo;
		
	}
	
	private function _tarx($path, $files, $tarfile) {
		$tar = new Tar();
		$tar->create( $tarfile );       // create tar file
		$tar->cd($path . "/");              // change to path for taring
	
		// read file by file from dir and add to tar archive. catch readdir
		// returns '.', '..', xml file or zip file (?)
		
		foreach( $files as $dummy=>$fileInfoarr) {
			$filename = $fileInfoarr[1];
			$fileFull = $path . "/" . $filename;
			
			$tar->add($fileFull);
		}
		$tar->close(); 
	}
	
	
	
	function downLoadLink() {
		
		$tarfileSingle = $this->tarfileSingle;
	
		$tmpsizeMb = filesize($this->tarfile) * 0.000001;
		echo "<br><a href=\"f_workfile_down.php?name=".$tarfileSingle."&file=".
			 $this->workSubDir."/".$tarfileSingle."\">".
			 "<img src=\"images/ic.docdown.big.gif\" border=0> ".
			 "<B>Download TAR-file</B></a> (size: <b>$tmpsizeMb</b> Mb)<br>\n";
		//echo "<br><b>2.</b> <a href=\"".$_SERVER['PHP_SELF']."?go=3&tablename=".$this->tablemom."\"><B>Clean temporary files</B></a><br>\n";
	}
	
	function start( &$sqlo, $table, $tmpdir, $workSubDir, $go) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$this->satObj      = new cSattachSubs();
		$this->fileHelpLib = new fileC();
		
		$this->objAttachArr = NULL;
		
		$this->go    = $go;
		$this->tmpdir= $tmpdir;
		$this->workSubDir = $workSubDir;
		// $this->objid = $objid;
		$this->tablemom = $table;
		$tablename= $this->tablemom;
		$go 	  = $this->go;
		$tmpdir   = $this->tmpdir;
		

		$this->objDir = $tmpdir; // ."/".$niceDirName;
		
		
	}
	
	function filesNum() {
		return sizeof($this->objAttachArr);
	}
	
	// organize TAR
	function tarx(  ) {
		$tmpdir = $this->tmpdir;
		$files  = &$this->objAttachArr;
	
		$this->tarfileSingle = "files.tar";
		$tarfile = $tmpdir."/".$this->tarfileSingle;
		$this->tarfile = $tarfile;
	
		$this->_tarx( $tmpdir, $files, $tarfile );
	
		if ( ($fhd = fopen($tarfile , "r")) == FALSE) {
			htmlFoot("Error", "ERROR: could not read TAR-file ".$tarfile);
		}
		fclose($fhd);
		
		//$ret = $compressObj->zip($tarfile);
		//if ( $ret>0 ) $tarfileSingle = $tarfileSingle . ".gz";
		echo "TAR Ready ...<br>\n";
	}
	
		
}
