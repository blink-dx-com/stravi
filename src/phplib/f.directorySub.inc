<?php
/**
 * provide functions to search files in a directory recursivly
 * @package f.directorySub.inc
 * @swreq 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/phplib/f.directorySub.inc 59 2018-11-21 09:04:09Z $
 */


/**
 *  directory sub functions
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 *
 */
class fDirextoryC {

    /**
     * scan directory, get all files (only this dir, not recursivly)
     * TBD: analyze, if file is a directory ! sort them out !
     * @param string $srcdir must contain SLASHES 
     * @param string $file_ext if NULL: does not test for extension ...
     * @param int $type_dir : by default only get files
     *      0: ALL
     *      1: search ONLY directories
     *      2: search ONLY file
     * @param string $regexp_pattern : e.g. "ResultImage.*\.jpg"
     * @return array sorted array( filenames )
     */
    function scanDir( $srcdir,  $file_ext = NULL, $type_dir = 0, $regexp_pattern=''  ) {
    	
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
    	
        $filearr  = array();
        //$retval  = 0;   
        $filecnt = 0;
        
        if ($file_ext!='') {
            $file_ext_esc = str_replace('.', '\.', $file_ext);
            $regexp_pattern='/'.$file_ext_esc.'/';
        } else {
            $regexp_pattern='/'.$regexp_pattern.'/';
        }
        
        
    	
    	if ( $_SESSION["userGlob"]["g.debugLevel"]>1 ) {
    		echo $FUNCNAME.":DEBUG-mode: srcdir:$srcdir regexp_pattern:'$regexp_pattern'<br>\n";
    	}
    	
    	if (!file_exists($srcdir)) {
    	    $error->set($FUNCNAME, 3, "Dir '$srcdir' not found.");
            return;
    	}
    
        if ( ($dh = opendir ( $srcdir )) !== false ) {
            
            while (false !== ($file = readdir ($dh))) {
                
    			$takeit = 0;
                //$flen   = strlen ($file);
               
                if ($file=="." OR $file=="..") continue;
                if ($regexp_pattern=="")    $takeit = 1;
    			else {  
    			    if ( preg_match($regexp_pattern, $file) ) $takeit = 1;
    			}
    			if ($type_dir==1) {
    			    //  only dirs
    				$takeit = 0;
    				if (is_dir ( $srcdir."/".$file )) $takeit = 1;
    			}
    			if ($type_dir==2) {
    			    //  only files
    			    if (is_dir ( $srcdir."/".$file )) $takeit = 0;
    			}
    			
    			if ($takeit) $filearr[]=$file;
                $filecnt++;
            }
            
            if ( sizeof($filearr) ) {
                sort ($filearr);
                reset ($filearr);
            }
            closedir($dh); 
        } else {
            $error->set($FUNCNAME, 1, "Can't open path '$srcdir'.");
            return;
        }
        /*
        if (!sizeof($filearr) AND $filecnt) {
        	$error->set('scanDir()', 2, "No files found with extension '$file_ext', but found $filecnt other file");
        } 
        */
        
        return ($filearr);
    }


    /**
     * join two paths DEFAULT: "/" 
     * if first element contains "\" => use WINDOWS-Sep
     * @param string $path1
     * @param string $path2
     * @return string
     */
    public static function join_path($patharr) {
        $dirsep = DIRECTORY_SEPARATOR;
        if (strstr($patharr[0],"\\") !=NULL )  $dirsep = "\\"; // then its WINDOWS
        
        $newPathArr=array(); 
        // remove leading + trailing DIRSEPs
        foreach($patharr as $path) {
            $tmp_str = rtrim($path, $dirsep);
            $do_join=1;
            if ($tmp_str=='.') $do_join=0;  // nix, ignore "."
            if ($tmp_str=='')  $do_join=0;  // nix, ignore "."
               
                
            if ($do_join)  $newPathArr[] = $tmp_str;
            
        }

        
        $fullPath = join($dirsep, $newPathArr );
        return $fullPath;
    }
    
    /**
     * string sanitizer for a filename
     * remove special chars which are not allowed in windows or unix paths
     * @param string $text
     */
    public static function str2path($text) {
        
        $pattern='/[^a-zA-Z0-9 -\._]/';
        $path_name = preg_replace($pattern, '_', $text);
        return $path_name;
    }
}


/**
 * advanced class to scan a directory recursivly
 *
 */
class fDirexScanC {
    
    private $dirarr; // array of realtive pasths


function __construct($baseDir) {
	$this->baseDir = $baseDir;
	$this->dirAnaLib = new fDirextoryC();
	$this->dirarr  = NULL;

	$this->ignoreDirs = NULL;
	$this->ignoreBases= NULL;
	$this->sub_stop_pattern =NULL;
}

/**
 * set special dirs
 * @param array $ignoreDirs : array of ignored dirs; e.g. ( opt/part/info )
 * @param array $ignoreBases : array of ignored dir-BASE: e.g. "CVS"
 */
function setIgnoreDirs($ignoreDirs, $ignoreBases) {
	$this->ignoreDirs = $ignoreDirs;
	$this->ignoreBases= $ignoreBases;
}

/**
 * if this pattern is found: STOP this tree and save directory
 */
function set_sub_stop_pattern($sub_stop_pattern) {
    $this->sub_stop_pattern = $sub_stop_pattern;
    $tmp = pathinfo($sub_stop_pattern);
    $this->sub_stop_file_ext = $tmp['extension'];

}

/**
 * get list of RELATIVE directories
 * @return NULL
 */
function getDirArr() {
	return ($this->dirarr);
}

/**
 * get list of ABSOLUTE directories
 * @return NULL
 */
function getAbsDirArr() {
    if (empty($this->dirarr)) return array();
    $full_dirs=array();
    foreach($this->dirarr as $rel_dir) {
        $full_dirs[] = fDirextoryC::join_path(array($this->baseDir, $rel_dir));
    }
    return $full_dirs;
}

/**
 * produce $this->dirarr
 * RECURSIVE function !
 * @param string $srcdir
 * @param int $level
 */
function _scan_dir_rec($srcdir, $level) {
    global $error;
    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
    
    $level = $level + 1;
    /*
     if ($level>7) {
     htmlFoot('ERROR', '_scanDir: <b>emergency exit!!!</b>');
     }
     */
    
    if ($srcdir != '.')     $srcDirFull = $this->baseDir. '/' . $srcdir;
    else     $srcDirFull = $this->baseDir;
    
    if ($this->ignoreDirs!=NULL) {
        if ( in_array($srcdir, $this->ignoreDirs) )  return;
    }
    if ( $this->ignoreBases!=NULL ) {
        $bname = basename($srcdir);
        if ( in_array( $bname, $this->ignoreBases) ) return;
    }
    
    $this->dirarr[] = $srcdir;
    
    if ( $this->sub_stop_pattern !=NULL ) {
        // read all files
        $type_file=0;
        
        $filearr  = $this->dirAnaLib->scanDir( $srcDirFull, $this->sub_stop_file_ext, $type_file );
        if (in_array($this->sub_stop_pattern, $filearr)) {
            return; // stop scanning in this branch
        }
    }
    
    
    $type_dir = 1;
    $dirarr  = $this->dirAnaLib->scanDir( $srcDirFull, '', $type_dir );
    
    if ( sizeof($dirarr) ) {
        foreach( $dirarr as $dirx) {
            
            if ($srcdir != '.')     $dirfull = $srcdir.'/' . $dirx;
            else                    $dirfull = $dirx;
 
            
            $this->_scan_dir_rec($dirfull, $level);
            if ($error->Got(READONLY))  {
                $error->set( $FUNCNAME, 1, 'Error on Dirfull: '.$dirfull);
                return;
                //OLD: $error->reset();
            }
            
        }
       
    }
}



/**
 * output: $this->dirarr
 * call after beeing ready: getDirArr()

 * @param  $srcdir
 * @param  $level
 * @return -
 */
function scanDirx($srcdir, $level=0 ) {
    $this->_scan_dir_rec($srcdir, 0);
}

}