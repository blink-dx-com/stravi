<?php
/**
 * XML_RPC server interface
 * 
 * - register XML_RPC functions
 * - XML-debug levels: see below
 *
 * @package icono_svr.php
 * @link  file://CCT_QM_doc/89_1002_SDS_code.pdf#pck:icono_svr
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq UREQ:0000306: g > XML-RPC: HOME 
 * @global <pre>  
 * $globals["xml_rpc.debug_level"] XML-debug levels 
 * 		0 - no
 *		1 - log only function-name
 *		2 - log also INPUT-xml and output-time-tag
 *		3 - log also OUTPUT-string
 * $globals["xml_rpc.debug_dir"] the debug dir; e.g. /tmp
 * $globals["xml_rpc.encode"] ecoding; DEFAULT: ISO-8859-1
 *   - supported: UTF-8, ISO-8859-1
 *</pre>
 * @version $Header: trunk/src/www/pionir/xmlrpc/icono_svr.php 59 2018-11-21 09:04:09Z $
 */
require_once("db_access.inc");
require_once("globals.inc");
require_once("access_check.inc");
require_once("../../../config/config.local.inc");
require_once("xml_rpc_func.inc");

class fXmlrpcSvr {
	
/**
 * - init of fXmlrpcSvr
 * - important: $globals is still not REGISTERED !
 */
function __construct() {
	global $globals;
	
	
	
	// Output all the comunication in the file e.g. /tmp/debug.xmlrpc
	$this->output_debug = $globals["xml_rpc.debug_level"];  // Set it to >0 to get output
	$this->deb = array(); 
	$this->deb["hostip"]= $_SERVER['REMOTE_ADDR'];
	$this->methodName   = NULL;
	date_default_timezone_set('Europe/Berlin');
	
	$this->encoding = 'ISO-8859-1';
    if ( $globals["xml_rpc.encode"]=='UTF-8') {
    	$this->encoding = 'UTF-8';
    }
    
	
	if ($this->output_debug>0) {
		$filename  = 'xml_rpc_log.log';
		$dirname   = $globals["xml_rpc.debug_dir"];
		if ( !is_dir($dirname) )  {  
			$this->output_debug = 0;
		}
		$this->deb["full_name"] = $dirname ."/". $filename;
	}
}

function _getArg(&$request_xml, $startpos) {
	$maxIntLen = 20; //max len of integer-string
	$found=0;
	$posv2=0;
	$tmpstr=NULL;
	$posv1 = strpos( $request_xml, "<int>", $startpos) + strlen("<int>" );
	if ($posv1>0) $posv2 = strpos($request_xml,"</int>", $posv1);
	if ($posv2>0 and $posv2-$posv1<$maxIntLen ) {
		$found=1;
		$tmpstr = substr( $request_xml, $posv1, $posv2-$posv1 );
	}
	return array($found,$tmpstr);
}
	
/**
 * manage debugging of XMLRPC request: input parameters
 * - global INPUT:
 *   - $this->deb["full_name"] - logfile name
 *   - $this->output_debug : see $globals["xml_rpc.debug_level"]
 *   - $globals["xml_rpc.debug_keys"] : if this is set, only special ymlrpc-methods are logged ...
 *        array('whitefuncs'=>array( array of methodNames  ) log only these methods
 * - global OUTPUT: $this->methodName
 * @param  string $request_xml
 * @return int $loginfo
 * 		 1 : o.k. logged
 * 		10 : can not write to log-file
 * 		11 : method not in white list; no log
 */
function debugIn( &$request_xml ) {
	global $globals;
	
	$this->methodName=NULL;
	$my_file = fopen( $this->deb["full_name"], 'a' );
	if (!$my_file) return 10;
	
	
	// get method
	$pos1 = strpos($request_xml,"<methodName>")+ strlen("<methodName>");
	$pos2 = strpos($request_xml,"</methodName>");
	
	if ($pos2>$pos1) {
		$methodName = substr($request_xml,$pos1, $pos2-$pos1);
		$this->methodName = $methodName;
	}
	
	if (isset($globals['xml_rpc.debug_keys']['whitefuncs']) ) {
		// only log listed methods
		$whitelist_meths = &$globals['xml_rpc.debug_keys']['whitefuncs'];
		if (!in_array($this->methodName, $whitelist_meths)) {
			return 11;
		}
	}
	
	fwrite($my_file,"<OPENCOMM TYPE=\"REQ\" HOST=\"".$this->deb["hostip"]."\" date=\"".date("Y-m-d H:i:s")."\"");
	fwrite($my_file," method=\"".$this->methodName."\"");
	
	// search second argument-value (usually an Object-ID)
	$key="<value>";
	$keylen = strlen($key);
	$posv1  = strpos($request_xml, $key);
	$posv2     = 0;
	$strlenInt = strlen("<int>");
	$arguFound=array();
	do {
		if (!$posv1) break;
		// search second VALUE
		$posv1 = strpos($request_xml, $key, $posv1+1);
		if (!$posv1) break;
		
		// search "<int>"
		$posv1 = strpos($request_xml, "<int>", $posv1+1) + $strlenInt;
		$posv2 = strpos($request_xml,"</int>", $posv1);
		if ($posv2) $arguFound[0]=1;
		
	} while (0);
	
	
	if ( $arguFound[0]>0 ) {
		$tmpstr = substr( $request_xml,$posv1, $posv2-$posv1 );
		fwrite($my_file," argu1=\"".$tmpstr."\"");
		// second argument
		$answer = $this->_getArg($request_xml, $posv2 + $strlenInt );
		if ( $answer[0] ) {
			fwrite($my_file," argu2=\"".$answer[1]."\"");
		}
	}
	
	
	fwrite($my_file,"/>\n");
	
	if ( $this->output_debug>1 ) {
		fwrite($my_file,$request_xml);
	}
	/*
	if ( $this->output_debug>3 ) {
		require_once 'f.debug.inc';
		fwrite($my_file, "\n HEX_DEBUG: ");
		fwrite($my_file, fDebugC::str2Hexinfo2( $request_xml ) );
	}
	*/
	fclose($my_file);
	
	return 1;
}

/**
 * manage debug to logfile for XMLRPC-output
 * @param unknown $out
 */
function debugOut( &$out ) {
	// $my_file is closed ...
	$my_file = fopen( $this->deb["full_name"], 'a' );
	if (!$my_file) return;
	
	fwrite($my_file,"<OPENCOMM TYPE=\"OUT\" HOST=\"".$this->deb["hostip"]."\" method=\"".$this->methodName."\" date=\"".date("Y-m-d H:i:s")."\"/>\n");
	if ( $this->output_debug>2 ) fwrite($my_file,$out."\n");
	
	fclose($my_file);
}

}

//Global definitions
global $error, $varcol; // must be defined here !
$error = & ErrorHandler::get(); // must define $error here, otherwise is unknown to functions()

$mainlib = new fXmlrpcSvr();

//Path prefix is a relative path identifying the partisan root directory ( /opt/partisan/www )
$path_prefix="../../";

//an array containing the directories containing the modules which have to be included   
$modules_included=array("pionir/xmlrpc/xml_functions");

//Check to see if there is a lab xml_rpc module. if found => included
if(is_dir($path_prefix."lab/xml_functions"))
	$modules_included[sizeof($modules_included)]="lab/xml_functions";

$xml_included_modules=array();
$xml_modules_dir=array();
$xml_functions_provided=array();

//Parse the modules list - include xml_dir_info.inc
foreach($modules_included as $cmod)
	require_once($path_prefix.$cmod."/xml_dir_info.inc");
	
	
	
//Include function files from all modules
foreach($xml_included_modules as $x_in_mod) {
	foreach ($xml_functions_provided[$x_in_mod] as $x_in_func) {	 
		require_once($path_prefix.$xml_modules_dir[$x_in_mod].$x_in_func."_xml.inc");
	}
}


// Start the processing of the xml request       
$request_xml = file_get_contents('php://input'); //PHP7.3 OLD:  $HTTP_RAW_POST_DATA;
      
if( $mainlib->output_debug ){
   $mainlib->debugIn($request_xml);
}

if(!$request_xml) {
     echo "<h5>No XML input found!</h5>";
     echo "<h5> Aborting ! </h5>";
} else {
    $my_xml=xmlrpc_server_create();

    //Register Functions
	$cnt=0;
    foreach($xml_included_modules as $x_in_mod)
    foreach ($xml_functions_provided[$x_in_mod] as $x_in_func) {
        xmlrpc_server_register_method ( $my_xml, $x_in_func,$x_in_func."_xml");
	}

    $out = xmlrpc_server_call_method ( $my_xml, $request_xml, "", array(
    	"output_type" => "xml", 
    	"version"  => "auto", 
    	"verbosity"=> "no_white_space",
    	"encoding" => $mainlib->encoding
         )
       );

    if( $mainlib->output_debug>1 ){
        $mainlib->debugOut( $out );
    };
    
	// 
    echo $out;
};
