<?php
/**
 * contains the allowed XML_RPC function list
 * @package xml_dir_info.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @app_type_info will be overwritten by type:2021_abbott
 */
    $xml_module_name="cct_main";

    $xml_included_modules[sizeof($xml_included_modules)+1]=$xml_module_name;
    $xml_modules_dir[$xml_module_name]="pionir/xmlrpc/xml_functions/";

    $xml_functions_provided[$xml_module_name] = array (
        // "connect",
	 	"connect2",
		"close",
		"f_rootinfo",
		"g_getfunclist",
    	"g_sessionalive",
    	"metacall",
		"get_version",
        );
