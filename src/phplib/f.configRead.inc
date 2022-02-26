<?php
/**
 * - read a config-file (LINUX-style)
 * - !!! do NOT use the ERROR-handler using object $error, 
 *   because it is called on SYSTEM-init
 * $Header: trunk/src/phplib/f.configRead.inc 59 2018-11-21 09:04:09Z $
 * @package f.configRead.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
 
class fConfigRead {
	
function __construct($filename=NULL) {
	if ($filename!=NULL) {
		$this->setFile($filename);
	} 
}

function _myError($message) {
	trigger_error  ( 'ERROR: '.$message, E_USER_ERROR );
	exit(-1);
}

function setFile($filename) {
	$this->filename = $filename;
}

/**
 * read config-file and create config-array
 * @global $this->filename
 * @return array $varArr
 */
function readVars() {
	$LINE_LENGTH = 32000;
	$filename    = $this->filename;
	
	if ($filename==NULL) {
		$this->_myError("No config-file given.");
	}
	$FH = fopen($filename, 'r');
	if ( !$FH ) {
		$this->_myError("Can't open config-file '$filename'");
	}
	
	$varArr=NULL;
	$lineno=1;
	while( !feof ( $FH ) ) { 
	    $line = fgets($FH, $LINE_LENGTH);
	    $lineSlim = trim($line);
	    do {
	    	if ($lineSlim==NULL or substr($lineSlim,0,1)=='#') break;
	    	$tmpEqualPos = strpos($lineSlim, '=');
	    	if (!$tmpEqualPos) {
	    		$this->_myError('Config-Error in file: "'.$filename.'" LineNo: '.$lineno);
	    	}
	    	$key = trim(substr($lineSlim,0,$tmpEqualPos));
	    	$val = trim(substr($lineSlim,$tmpEqualPos+1));
	    	$varArr[$key]=$val;
	    } while (0);
	    $lineno++;
	    
	}
	fclose ( $FH);
	
	return ($varArr);
}
	
}
