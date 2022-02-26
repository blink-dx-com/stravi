<?php

/**
 * sub routines to manage $_SESSION['globals']["work_path"] / YOUR_SESSION_DIR / SUB_DIRS
 * @package f.workdir.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/phplib/f.workdir.inc 59 2018-11-21 09:04:09Z $
 */
class workDir { 

var $subDirName;
var $spaceLeftLimit = 500000000; // needed space on work_path in bytes

/**
 * get main session-work dir, no test for existens 
 */
function getSessionMainDir() {  
    global  $error;
    $FUNCNAME = "getSessionMainDir";
	
    $initWorkPath = $_SESSION['globals']["work_path"];
    if ($initWorkPath=="") {
		$error->set($FUNCNAME, 1, "Work path is not set. Please contact your administrator.");
        return;
    }
	
    $session_path = $initWorkPath . "/pdir_". session_id();
    return ($session_path);
}

/**
 * get work dir
 * @param string $subDirName
 * @param string $noClean
 * @return $path
 */
function getWorkDir ( 
	$subDirName,   // name of sub-dir
	$noClean = NULL	 
    ) {
    // RETURN: $path
    
    global   $error;
	
	$error = & ErrorHandler::get();
    
    if ($subDirName=="") {
		$error->set("getWorkDir", -4, "Need a name for your sub-work-directory");
        return;
    }
    
   	$session_path = $this->getSessionMainDir();
   	if ($error->Got(READONLY)) {
		$error->set("getWorkDir", -1, "Could not get session-tmp-dir.");
        return;
    }
	
		
    if (!file_exists($session_path)) {
        if (!mkdir($session_path, 0700)) {
			$error->set("getWorkDir", -2, "Creating work path '$session_path' failed."
                                . " Please contact your administrator.");
            return; 
        }        
    } 
    
    if (strstr($subDirName, '/')) {
        // SLASHES are not allowed
        $subDirName = str_replace('/', '_', $subDirName);
    }
    
    $this->subDirName = $subDirName;
    
    $subWork_path = $session_path . "/" . $subDirName;
	
    if (!file_exists($subWork_path)) {
        if (!mkdir($subWork_path, 0700)) {
            $error->set("getWorkDir", -3,"Creating work path '$subDirName' at '$session_path' failed."
                                . " Please contact your administrator."); 
			return;
        }        
    } else { 
        // clean old files NOW ?
        if ( !$noClean ) $this->cleanSubDir();    
    } 
    
    if ( $_SESSION['userGlob']["g.debugLevel"] >= 4 ) 
        echo "DEBUG:[4..] getWorkDir():: PATHS: $session_path, $subWork_path <br>\n";
   
    
    return ( $subWork_path );   
}   

/**
 * - delete files/sub-directories in this directory
 * - if an object is a directory: call the function recursive and delete the directory afterwards
 * @param string $dirFull
 */
function _cleanSubDirFiles($dirFull) {
	
	
	if ( $_SESSION['userGlob']["g.debugLevel"] >= 2 ) 
        echo "DEBUG: _cleanSubDirFiles()::clean '$dirFull'<br>\n";
		
	if ($handle = opendir($dirFull)) {

        while (false !== ($file = readdir($handle))) {
            if ($file != "." && $file != "..") { 
				$fileFull = $dirFull."/".$file;
				if ( !is_dir($fileFull) ) {  
                	unlink ( $fileFull );	// only possible for files
				} else {
					$thisdir = $fileFull;	
					$this->_cleanSubDirFiles($thisdir); // remove files in directory
					rmdir($thisdir);	// remove the EMPTY directory
				}
                $cnt++;
            }
        }

        closedir($handle); 
    }
}

/**
 *  remove old files and directories in the session-dir
 */
function cleanSubDir() {       
	global $error;
	
    $main_dir = $this->getSessionMainDir();
    
	if ( $error->Got() )   return (-1); // clean error
    if ( $this->subDirName=="") {
         return (-1);
    }
    
    $subWork_path = $main_dir."/".$this->subDirName;
    // echo "DEBUG: cleanSubDir(): $subWork_path";  
    $this->_cleanSubDirFiles($subWork_path);
}

// PUBLIC
function removeWorkDir(  ) {
  	  global $error;
	  
      $main_dir = $this->getSessionMainDir(); 
	  if ( $error->Got() )   return (-1); // clean error
      if ( $this->subDirName=="" ) {
          return (-1);
      }
      
      // remove files of subDir
      $this->cleanSubDir();
      
      // remove subDir 
      $subWork_path = $main_dir."/".$this->subDirName;
      rmdir($subWork_path); 
}

function getAbsDir ($relpath) { 
	$startPath = $this->getSessionMainDir();
	$abspath = $startPath . '/' . $relpath;
	return ($abspath);
}

/**
 * analyse, if enough space is available
 * @param int $spaceExpect in bytes
 * @return array('free'=>, 'expectLeft'=>)
 */
public function spaceAnalysis($spaceExpect) {
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	$freeSpace       = disk_free_space ( $_SESSION['globals']['work_path'] );
	$expectSpaceLeft = $freeSpace - $spaceExpect;
	$answer = array('free'=>$freeSpace, 'expectLeft'=>$expectSpaceLeft);
	
	if ($expectSpaceLeft < $this->spaceLeftLimit) {
		$error->set( $FUNCNAME, 1, 
			'Not enough space left ('.$expectSpaceLeft.') for this amount of data on WORK-dir!'.
		    ' SpaceLeftLimit:'.$this->spaceLeftLimit.' Bytes' );
		return $answer;
	}
	
	return $answer;
	
}


}
