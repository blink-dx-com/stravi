<?php
require_once ('f.workdir.inc');

/**
 * functions for upload of files (support also zip technology)
 * @package f.upload.inc
 * @swreq UREQ:GENERAL
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @example
 * 
 *  $script_name='XXX';
    $userfile      = $_FILES['userfile']['tmp_name'];
    $userfile_name = $_FILES['userfile']['name'];
    $userfile_size = $_FILES['userfile']['size'];
    
    $newName='file.dat';
    
    $uplib = new uploadC();
    $tmpfile =  $uplib->mvUpload2Tmp($script_name, $newName, $userfile, $userfile_name, $userfile_size);
	
	# after stiored in temp-dir ...
	$tmpfile = $uplib->getFileFromTmp($script_name, $newName);
 */
class uploadC { 

function __construct() {
	$this->workObj = new workDir();
}

function getWorkDir() {
	return ($this->workDir);
}

/**
 * is this a ZIP-file ?
 * - returns 0, if ZIP-file contains no data ...
 * @param string $filename  fullFileName
 * @return int 0,1
 */
function testZipFile( $filename ) {
	global $error;
    $FUNCNAME='testZipFile';

    $error = & ErrorHandler::get();
    if ( !file_exists($filename) ) {
		$error->set($FUNCNAME, 1, "zip-file '$filename' not found");
		return;
    } 

    $zipcmd   = "unzip -t \"".$filename."\"; echo $?";  // get answer from command
	// $dummybuf = NULL;
	$answer   =  exec ( $zipcmd );
	
	$isZip = 0;
	if ($answer==0) {
		$isZip = 1;
	} 
	return ($isZip);
}

/**
 * get ZIP-file info
 * - brings error on empty ARCHIVE
 * @param string $filename
 * @return array $result
 *   'numfiles' : num
 *   'sizeall'  : size of files in bytes
 */
function getZipfileInfo($filename) {
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	
	$error = & ErrorHandler::get();
	if ( !file_exists($filename) ) {
		$error->set($FUNCNAME, 1, "zip-file '$filename' not found");
		return;
	}
	
	$zipcmd   = "unzip -l \"".$filename."\"";  // get answer from command
	$answer = NULL;
	$answer = shell_exec ( $zipcmd );

	
	$NEWLINE = "\n";
	
	$result   = array();
	$numfiles = substr_count ( $answer , $NEWLINE ); // count lines
	$numfiles = $numfiles - 5; // minus num of offset lines
	
	if ($numfiles<0) {
		$error->set($FUNCNAME, 2, "zip-file is not valid (no files found) ".$answer);
		return;
	}
	
	
	/** search last line; e.g. 
	---------                     -------
         5187                     3 files

	 */
	$sizeall  = 0;
	$poslast  = strrpos($answer, $NEWLINE, -2); // ignore the LAST newline (on end of FULL string)
	$lastline = substr($answer, $poslast);
	$lastline = trim($lastline); // remove leading white spaces
	$linearr = explode(" ", $lastline);
	$sizeall = $linearr[0];
	
	if (!is_numeric($sizeall)) {
		$error->set($FUNCNAME, 3, "zip-file: can not analyse SIZE of extracted files.");
		return;
	}
	
	$result=array();
	$result['numfiles']=$numfiles;
	$result['sizeall'] =$sizeall;
	
	return $result;
}


/**
 * unzip file and extract file to $destdir
 * @param string $filename  fullFileName
 * @param $destdir
 * @param $opt : 'verbose' = ['full'], 'silent'
 */
function getZipFile( $filename, $destdir, $opt=NULL ) {
	$FUNCNAME='getZipFile';
    global $error;
    
    $error = & ErrorHandler::get();
    if ( !file_exists($filename) ) {
         $error->set($FUNCNAME, 1, "zip-file '$filename' not found");
         return;
    } 

	$isZip = $this->testZipFile( $filename );
	if (!$isZip) {
		$error->set($FUNCNAME, 2, "file '$filename' is not a ZIP-file.");
        return;
	}

    $zipcmd  = "unzip \"".$filename."\" -d \"".$destdir."\" "; 

	if ( $opt['verbose'] == 'silent' ) {
		$answer  =  exec ( $zipcmd );
	} else {
		$gotinfo = NULL;
    	$answer  =  system ( $zipcmd , $gotinfo);
	}
}

/**
 * move uploaded Temp-file to the Partisan-Work-dir
 * @param string $scriptid  identifies the PHP-script ( for generation of Partisan-Work-dir )
 * @param string $newName   new filename in the TEMP-dir (without DIRECTORY)
 * @param string $userfile full : file path
 * @param string $userfile_name : short original file name
 * @param int $userfile_size
 * @return string $fullFileName (in Partisan-Work-dir)
 */
function mvUpload2Tmp( $scriptid, $newName, $userfile, $userfile_name, $userfile_size=NULL ) {
    global  $error;   
	$FUNCNAME='mvUpload2Tmp';

    $filename = "";
    $FH = fopen($userfile, 'r');
    if ( !$FH ) {
        $tmptxt  = " file_size:$userfile_size file:$userfile <br>";
        $tmptxt .= " file_name:$userfile_name <br>";  
        $error->set($FUNCNAME, 1, "HTTP-Upload failed: ".$tmptxt );
        return;
		
    } else {

		fclose($FH);
		$this->workObj = new workDir();
		$this->workDir = $this->workObj->getWorkDir ( $scriptid );
		if ($error->Got(READONLY))  {
			$error->set($FUNCNAME, 2, "Generation of WORK-dir failed." );
			return;
		}
	
        $tmpfile= $this->workDir ."/". $newName;    
        if (!move_uploaded_file($userfile, $tmpfile))  {
            $error->set($FUNCNAME, 3, "Internal-Copy to temporary file failed!");
        }                
        $filename = $tmpfile;
    }
    return ( $filename );  
}

/**
 * get file-name from Partisan-Work-dir
 * @param string $scriptid  identifies the PHP-script ( for generation of Partisan-Work-dir )
 * @param string $newName   new filename in the TEMP-dir (without DIRECTORY)
 * @return string $fullFileName (in Partisan-Work-dir)
 */
function getFileFromTmp( $scriptid, $fileName ) {
	global  $error;   
	$FUNCNAME='getFileFromTmp';

	if($fileName==NULL) {
	    $error->set($FUNCNAME, 1, "Input: New-filename missing." );
	    return;
	}
	$noClean = 1;
	$workDir = $this->workObj->getWorkDir ( $scriptid, $noClean );
	if ($error->Got(READONLY))  {
		$error->set($FUNCNAME, 2, "Generation of WORK-dir failed." );
		return;
	}

	$tmpfile = $workDir ."/". $fileName; 
	return ($tmpfile);  
}

/**
 * set existing workdir, which contains data
 * @param string $scriptid
 * @return $path
 */
function set_existing_workdir($scriptid) {
    $workdirObj = new workDir();
    $workDirFull   = $workdirObj->getWorkDir ( $scriptid, 1 );
    return $workDirFull;
}

}

