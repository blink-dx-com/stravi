<?php
/**
 * define session variables
  - see also CVS/_dev/session_vars.txt
 * @package main.sessvars.inc
 * @swreq   SREQ:0001070: g > XMLRPC > s_sessVars : harmonize
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */


/*
 * set mainSess
 * @param $php_self_str     = $_SERVER['PHP_SELF'];
   @param $script_file_str  = $_SERVER['SCRIPT_FILENAME'];
 * @var array $s_sessVars system session variables
 * @link file://CCT_QM_doc/89_1002_SDS_code.pdf#var:s_sessVars
 * @swreq SREQ:0001070: g > XMLRPC > s_sessVars : harmonize 
 */
function makeSessVars($php_self_str, $script_file_str) {
	global $s_sessVars;
	
	$s_sessVars=array();          /* session variables $s_sessVars[VAR_NAME]=VALUE */
	$s_sessVars["start"]    = ""; /* ["start"] login window for first time */
	$s_sessVars['g.headquicks']=1;
	
	## URL of the pionir subdirectory without servername (eg. '/partisan/www/pionir'):
	
	
	$s_sessVars["loginURL"] = dirname($php_self_str);
	
	## Operating system absolute pathname of the pionir subdirectory (eg. '/opt/partisan/www/pionir'):
	$s_sessVars["loginPATH"] = dirname(__FILE__);  //OLD: $script_file_str
	
	## URL of the Partisan 'document root' directory without servername (eg. '/partisan/www'):
	
	$tmpstr = dirname( $s_sessVars["loginURL"] );  
	if ( substr($tmpstr,strlen($tmpstr)-1,1) == "/" ) $tmpstr .= "."; // add ".", if last char is "/"
	$s_sessVars['DocRootURL'] = $tmpstr;      
	
	## Operating system absolute pathname of the Partisan WebSpace root directory (eg. '/opt/partisan/www'):
	$s_sessVars['DocRootDir'] = dirname( $s_sessVars["loginPATH"] ); 
	
	## Operating system absolute pathname of the Partisan Application root directory (eg. '/opt/partisan'):
	$s_sessVars['AppRootDir'] = dirname( $s_sessVars['DocRootDir']  ); 
	
	## Operating system absolute pathname of the Partisan Application root directory (eg. '/opt/partisan'):
	$s_sessVars['AppLibDir'] = $s_sessVars['AppRootDir'] . DIRECTORY_SEPARATOR . "phplib"; 

	
}
