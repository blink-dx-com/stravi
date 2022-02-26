<?php

/**
 * Module logging
 * - used by func_head2.inc
 * - globals:
 *  $_SESSION['globals']['app.modulLog']: 0 or 1
 *  $_SESSION['globals']["app.log.dir"] : e.g. /var/log/partisan
 * @package f.modulLog.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq SREQ:0003368: g > Modul-Usage-Statistics 
 * @unittest exists
 * @version  $Header: trunk/src/phplib/f.modulLog.inc 59 2018-11-21 09:04:09Z $
 */
class fModulLogC {
	
	/**
	 * flag, if logging is active
	 * @var $_logPossible
	 *   >0  : active
	 *   <=0 : inactive
	 */
	private $_logPossible;
	
	/**
	 * constructor
	 */
	public function __construct() {
		
		$this->_logPossible = 0;
	
		if( $_SESSION['globals']['app.modulLog'] <=0 ) {
			$this->_logPossible = -1;
			return; // not active
		}
		
		$datapath = $_SESSION['globals']["app.log.dir"];
		if ($datapath==NULL) return -4; // bizarre!
		
		$modLogFile = $datapath . '/app.modulLog.dat'; 
		if (!file_exists($modLogFile)) {
			
			if (!file_exists($datapath)) {
				$this->_logPossible = -2;
				return; // no base dir
			}
			
			if (!is_writable($datapath)) {
				$this->_logPossible = -3;
				return;
			}
		}
		if (!is_writable($modLogFile)) {
			$this->_logPossible = -6;
			return;
		}
		
		$this->modLogFile = $modLogFile;
		$this->_logPossible = 1;

	}
	
	/**
	 * check session cache, if module already logged at this user-session
	 * - analyse $_SESSION['s_sessVars']['g.modulesUse']
	 * @return $doTouch 0/1
	 */
	private function checkCache($phpfile) {
		$MAX_CACHE=40; // max number of cached MODULE-names
		
		if (!$_SESSION['sec']['db_user_id']) return; // not logged in
		
		// process analysator
		$_SESSION['s_sessVars']['g.moduleNowF'] = $phpfile;
		$_SESSION['s_sessVars']['g.moduleNowT'] = date ("Y-m-d\TH:i:s", time());
		$_SESSION['s_sessVars']['g.moduleNowP'] = getmypid ();
		
		$tmparr = &$_SESSION['s_sessVars']['g.modulesUse'];
		if (is_array($tmparr)) {
			if (in_array($phpfile, $tmparr)) {
				return 0;
			}
			
			// as long the array is too big
			while (sizeof($tmparr)>=$MAX_CACHE) {
				// remove first entry
				
				$key = key($tmparr);
				unset($tmparr[$key]); 
			}
		}
		$_SESSION['s_sessVars']['g.modulesUse'][] = $phpfile; // insert new entry
		
		return 1;
		
	}
	
	
	/**
	 * - cache last used PHP-script-calls
	 * - [OPTIONAL] log PHP-script-calls in $this->modLogFile
	 * input: $this->_logPossible
	 * file-format:
	 * 0	1	 2	    3   4
	 * TIME USER MODULE NUM IP
	 *	
	 * @param $sqlo : can be NULL (place holder)
	 * @param $modname : can be NULL, than calc name automatically
	 *   - normal file-path
	 *   - 'PLUGIN:'xxx
	 *   - 'XMLRPC:'xxx
	 *   - 'JSONRPC:'xxx
	 * @param boolean $doNotLogFile
	 *  if 1: do not write logfile
	 * @param string $moduleExtension: extension after module, separated by "?"
	 * @return $wasLogged
	 *   10 - yes
	 *   6  - problem on writing data
	 *   5  - no: touch-cache: already logged
	 *   0  - no action needed
	 *   1  - log on file not allowed; by flag
	 */
	public function logModul(&$sqlo, $modname=NULL, $doNotLogFile=0, $moduleExtension=NULL) {	
		
		if (!$_SESSION['sec']['db_user_id']) return; // not logged in
		$now = time();
		
		if ($modname==NULL) {
			$phpfile = $_SERVER['PHP_SELF'];
			if (substr($phpfile, -6)=='/p.php') {
		  		// get module-name
		  		$phpfile = 'PLUGIN:'.$_REQUEST['mod']; // append module-name
		  	}
		} else $phpfile = $modname;
		if ($moduleExtension!=NULL) {
			$phpfile .= '?'.$moduleExtension;
		}
	  	
	  	// check Session_cache
	  	$doTouch = $this->checkCache($phpfile);
	  	if (!$doTouch) return 5;
	  	
	  	// from here on log info to log-file ...
	  	
	  	if ($this->_logPossible<=0) return 0; // no logging allowed
	  	if ( $doNotLogFile>0 ) return 1;
	  	
	  	$modLogFile = $this->modLogFile;

	  	$rawdata2 = date ("Y-m-d\TH:i:s", $now)."\t".$_SESSION['sec']['appuser']."\t".$phpfile."\t\t".$_SERVER['REMOTE_ADDR'] ."\n";
	  	
	  	$fp = @fopen($modLogFile, 'a'); // append text; hide errors, because not severe
		if (!$fp) {
			return 6; // problem on writing file
		}
		
		$retVal = fputs( $fp, $rawdata2 ); /* write data */
		fclose( $fp );
		
		return 10;
	  	
	}
	
	public function logIsPossible() {
		return $this->_logPossible;
	}
	
}
