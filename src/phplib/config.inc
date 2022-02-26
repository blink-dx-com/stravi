<?php
/**
 * config for the  for HUB-D variant
 * Initialize $s_product[] variables.
 *
 *
 * Configuration files are ALWAYS read in this order:
 * 1) "config.product.inc"
 * 2) "config.local.inc"
 * 
 * All config files are stored in directory 'partisan/config'.
 * This relative path is hard-coded here: 
 *
 * @module config/config.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */

require_once('utilities.inc');
require_once('f.util_IniAna.inc');
require_once('f.configRead.inc');

class gAppConfig {
	
    function _myError($message) {
    	trigger_error  ( $message, E_USER_ERROR );
    	exit(-1);
    }
    	
    function __construct() {
    	$this->appBaseDir = dirname(dirname(dirname($_SERVER['SCRIPT_FILENAME'])));
    }
    
    function getConfigDir() {
    	return ($this->appBaseDir.'/config');
    }
    
    /**
     * read kernel config file
     * @param array $s_product [IN|OUT]
     */
    function getKernelInfo(&$s_product) {
    	$FUNCNAME   = 'getKernelInfo';
    	$phplib_dir = $this->appBaseDir.'/phplib';
    	$kernelFile = $phplib_dir.'/zzz.kernel.dat';
    	
    	$confReadLib = new fConfigRead($kernelFile);
    	$kernelVars  = $confReadLib->readVars();
    	if ($kernelVars==NULL) {
    		 $this->_myError($FUNCNAME.': no kernel information!');
    	}
    	
    	$s_product['pionir_version'] = $kernelVars['kernelVersion'];
    }

}

$gAppConfLib = new gAppConfig();

$CONFIG_DIR = $gAppConfLib->getConfigDir() ;

## OLD: config.kernel.inc

$s_product=array();
## Read product info needed to confectionate the Partisan installation (skin):
$filename = "$CONFIG_DIR/config.product.inc";
if ( is_readable($filename) )
{ include($filename); }
else
{ echo "<pre>ERROR: '<tt>$filename</tt>' not found.\nPlease contact your Administrator.\nProgram exited.\n</pre></body></html>";
  exit;
}##End else
    
$gAppConfLib->getKernelInfo($s_product);


## Optionally read the local configuration file if it exists.
$filename = "$CONFIG_DIR/config.local.inc";
if ( is_readable($filename) )
{ include($filename); }


###########################################################

## Clipping of object specific 'upload_max_filesize' to the general
## limit set by the directive 'upload_max_filesize'
## in the PHP configuration file '/etc/httpd/php.ini'.

$general_upload_max_filesize = ini_get_bytes('upload_max_filesize');

foreach ( $globals as $key => $value )
{ if ( preg_match('/^F\..*\.UPLOAD_MAX_SIZE$/',$key) )
  { $globals[$key] = min($value,$general_upload_max_filesize); }
}##End foreach

###########################################################