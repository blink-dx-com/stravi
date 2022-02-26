<?php
require_once ("f.directorySub.inc");
require_once ("visufuncs.inc");
require_once ('gui/f.plugin.inc');

/**
 * produce a lib to get all Tools (*.php and PLUGINS) in the system
 * - EXCLUDED: single-object-edit-extension *.xedit.php
 * @package g.tools.get.inc 
 * @swreq   SREQ:0003464: g > admin > get all tools in system (LIB) 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/phplib/root/g.tools.get.inc 59 2018-11-21 09:04:09Z $
 */
class gModulesAna {

	private $useDirs; // start of dir scan
	private $moduleArr;
	private $fileext; // e.g. .inc
	
	/**
	 * start analysis
	 * @param $baseDir - relative path to PARTISAN-base dir
	 */
	function __construct($baseDir) {
	    
	    $lab_sub_dir = $_SESSION['globals']['lab_path'];
	    
		$this->useDirs = array(
			"plugin" 		 => array('ty'=>'plugin'),
		    "www/".$lab_sub_dir."/plugin" => array('ty'=>'plugin'),
			"www/pionir"     => array('ty'=>'tool'),
		    "www/".$lab_sub_dir       => array('ty'=>'tool'),
		);
		
		$this->ignoreDirs = array(
			'www/pionir/rootsubs',
		    'www/'.$lab_sub_dir.'/root'
		);
		
		$this->bad_patterns = array(
			'.*\.xedit\.php'
		);
		
		
		$this->ignoreBases = array( '_subs' );
		
		$this->baseDir = $baseDir; // '../../'; // '../../../';
		$this->allarr  = array();
		$this->dirarr  = array();
	
		$this->dirAnaLib = new fDirextoryC();
		
	}
	
	public function getBaseDir() {
		return $this->baseDir;
	}
	
	/**
	 * check, if file is a module
	 * @param string $filex
	 * @return int $doit
	 *   0: no
	 *   1: yes
	 */
	private function _match_check($filex) {
		$doit=1;
		foreach($this->bad_patterns  as $pattern) {
			$pattern_php='/'.$pattern.'/';
			if ( preg_match($pattern_php, $filex) ) {
				$doit=0;
				break;
			}
		}
		return $doit;
	}
	
	/**
	 * analyse one dir
	 * @return $this->moduleArr
	 */
	private function _anaOneDir( $srcdir ) {
		global $error;
		$FUNCNAME= '_anaOneDir';
	
		$baseDir = $this->baseDir;
	
		$ext=$this->fileext; //  '.inc';
		$incarr = $this->dirAnaLib->scanDir( $baseDir.$srcdir, $ext, 0 );
		$error->reset();  // ????
		if (!sizeof($incarr)) return;
	
		foreach( $incarr as $dummy=>$filex) {
			
			if ($this->_match_check($filex)) {
				$this->moduleArr[]=$srcdir.'/'. $filex;
			}
		}
		reset ($incarr);
	
	}
	
	/**
	 * get only plugins
	 * @param $sqlo
	 * @param $srcdir
	 * @params array $options
	 * 	'fileext' : inc or php
	 */
	function scanStart( $srcdir, $options=NULL ) {
		// scan directory for files
		global $error;
		$FUNCNAME= 'scanStart';
		
		$this->fileext = $options['fileext'];
		
		$scanLib = new fDirexScanC($this->baseDir);
		$scanLib->setIgnoreDirs( $this->ignoreDirs, $this->ignoreBases );
		$scanLib->scanDirx($srcdir, 0);
		$dirarr = $scanLib->getDirArr();
		// glob_printr( $dirarr, "dirarr info" );
	
		// analyse files
		$cnt=0;
		foreach( $dirarr as $dirx) {
			$this->_anaOneDir( $dirx );
			if ($error->Got(READONLY))  {
	     		$error->set( $FUNCNAME, 1, 'dir: '.$dirx. ' problem' );
				return;
			}
			$cnt++;
			// if ( $cnt>4) return;
		}
		
	}
	
	
	
	/**
	 - scan recursive
	 - for each file do
	 @param $method
	   'plugin', only plugins
	   'ALL' : Tools and plugins
	 */
	function analyse( $method ) {
		$this->moduleArr = NULL;
		
		$scOpt = array('fileext'=>'.inc');
		
		foreach( $this->useDirs as $dirRoot=>$diropt) {
			if ($diropt['ty']=='plugin') {
				$this->scanStart($dirRoot, $scOpt);
			}
		}
		 
		
		if ($method=='plugin') return; // o.k.
		
		$scOpt = array('fileext'=>'.php');
		
		foreach( $this->useDirs as $dirRoot=>$diropt) {
			if ($diropt['ty']=='tool') {
				$this->scanStart($dirRoot, $scOpt);
			}
		}
		
		
	}
	
	public function getModArr() {
		return $this->moduleArr;
	}

}
