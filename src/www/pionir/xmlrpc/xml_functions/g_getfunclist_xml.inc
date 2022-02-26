<?php
/**
 * @namespace core::xmlrpc
 */


/**
 * get all functions of XMLRPC
 *
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @see     "usedby:external:ICONOCLUST"
 */
function g_getfunclist_xml( $f_name, $f_param ){
	
	$my_s_id     = $f_param[0];
	
	global $error, $varcol;
	
    $xmlHelpLib = new xmlrpcHelper(); 
    $answer = $xmlHelpLib->startSession($my_s_id, 'g_getfunclist'); 
    if ($answer[0]>0) return $answer;    

	

	
	$path_prefix="../../";
	//an array containing the directories containing the modules which have to be included   
	$modules_included=array("pionir/xmlrpc/xml_functions");
	
	//Check to see if there is a lab xml_rpc module. if found => included
	if(is_dir($path_prefix."lab/xml_functions"))
	$modules_included[sizeof($modules_included)]="lab/xml_functions";

	//Parse the modules list - include xml_dir_info.inc
	foreach( $modules_included as $dummy=>$cmod) {
		require($path_prefix.$cmod."/xml_dir_info.inc");
	}
	reset ($modules_included);
	
	$funcnames = NULL;
	foreach( $xml_included_modules as $dummy=>$x_in_mod) {
		$xmlfuPoi = &$xml_functions_provided[$x_in_mod];
		foreach( $xmlfuPoi as $dummy=>$x_in_func) {	 
			$funcnames[] = $x_in_func;
		}
		reset($xmlfuPoi);
	}	
	reset($xml_included_modules);
	//Include function files from all modules
	
	return array( 0, $funcnames );
}