<?php
require_once("f.modulLog.inc");
require_once( "../api/lib/metacall.inc");


/** 
 * @access  private
 */
function _meta_include($mod) {
		global $error;
		$FUNCNAME = '_meta_include';
		
		$basekeys = array(
		    'DEF'=> $_SESSION['s_sessVars']["loginPATH"]   .'/api/mods',
			'LAB'=> $_SESSION['s_sessVars']["AppLabLibDir"].'/api/mods'
		);
		$BASELEN  = 3; // 3 letters !!!
		
		if ($mod==NULL) {
			$error->set( $FUNCNAME, 1, 'no Plugin name given.' );
			return;
		}
		if (strstr($mod,'..')!=NULL) {
			$error->set( $FUNCNAME, 2, 'Plugin-path invalid.' );
			return;
		}
		$basekeyX = substr($mod,0,$BASELEN);
		if ( $basekeys[$basekeyX]==NULL ) {
			$error->set( $FUNCNAME, 3, 'No support for Base:'.$basekeyX.'.' );
			return;
		}
		$pureModPath = substr($mod,$BASELEN+1); // without '/'
		$BASE_DIR = $basekeys[$basekeyX];
		$relFile  = $BASE_DIR.'/'.$pureModPath.'.inc';
		
		if (!file_exists($relFile)) {
			$error->set( $FUNCNAME, 4, 'Plugin "'.$relFile.'" not found.' );
			return;
		}
		require_once($relFile);
		
		$classname = $pureModPath.'_XRPC';
		return $classname;
	}


/**
 * manage XML-RPC meta-calls : the metacall interface
 * 
 * - call a module-method
 * - dynamic binding of a module
 * - module location:  "../mods/module_name.inc" > class module_name_XRPC
 * 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq   0001179: g > xml-rpc > metacall 
 * @param    array $f_param <pre>
 * [0] long   sess_id
 * [1] string moduleURL : NAMESPACE/MODULE
 *      + NAMESPACEs: DEF, LAB
 *      + moduleURL-example: 'DEF/oEXP_test'
 * [2] array  params
 * 	</pre>
 * @throws exception <pre>
 * 3 - invalid module-name
 * </pre>
 */	
function metacall_xml( $f_name, $f_param ){
	
	$FUNCNAME='metacall_xml';
	if ( sizeof($f_param)<2 ) {
		return array( 1, "Incorrect number of parameters: minimum: 2");
	}
	$errprio     = 3;
	$my_s_id     = $f_param[0];
	$module_name = $f_param[1];
	if ($f_param[2]!=NULL) $params 	 = &$f_param[2];
	else $params = NULL;
	
	global $error, $varcol;
	
    $xmlHelpLib = new xmlrpcHelper(); 
    $answer = $xmlHelpLib->startSession($my_s_id, 'metacall:'.$module_name); 
    if ($answer[0]>0) return $answer;    

	
	$error  = & ErrorHandler::get();
	$varcol = & Varcols::get();
	$sqlo   = logon2();
	
	$className = _meta_include($module_name);
	if ($error->Got(READONLY))  {
		$error->set( $FUNCNAME, 3, 'metacall: invalid module:"'.$module_name.'"' );
		$errtext = $error->getAsTextAndLog( $FUNCNAME, $errprio );
		$error->reset();
		return array( 3, $errtext );
	}
	
	
	$plugLib = new $className();
	$plugLib->__setInit($sqlo, $params);
	$answer = $plugLib->__checkParams();
	if ($answer<=0 or $error->Got(READONLY)) {
		$error->set( $FUNCNAME, 4, 'metacall: module:"'.$module_name.'" checkParams()' );
		$errtext = $error->getAsTextAndLog( $FUNCNAME, $errprio );
		return array( 4, $errtext );
	}
	
	$answer  = &$plugLib->run();
	if ($error->Got(READONLY))  {
		
		// @swreq: UREQ:0001171: g > XML_RPC > Fehlerkonzept 
		$errLast   = $error->getLast('',0,READONLY);
		$error_id  = $errLast->id;
	
		$erroridUse= 5;
		if ($error_id>10) $erroridUse=$error_id; // errors bigger 10 will be returned with original ID
		$error->set( $FUNCNAME, $erroridUse, 'module:"'.$module_name.'" run failed.' );
		$errtext = $error->getAsTextAndLog( $FUNCNAME.':'.$module_name, $errprio );
		return array( $erroridUse, $errtext );
	}
	
	return array( 0, $answer );
}