<?php
/**
 * @namespace core::xmlrpc
 */

/**
 * example for XMLRPC
 *
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param    array $f_param <pre>
 * 	 [0] sess_id
 * 	 [1] exp_id
 * 	</pre>
 * @return array
 * @throws exception
 * @see     "usedby:external:ICONOCLUST" or "usedby:external:CABtool"
 * 
 */
function DEFAULT_xml( $f_name, $f_param ){
	
	if ( sizeof($f_param)<2 ) {
		return array( 1, "Incorrect number of parameters: minimum: 2");
	}
	
	$my_s_id     = $f_param[0];
	$my_cbatch_id= $f_param[1];
	
	global $error, $varcol;
	
    $xmlHelpLib = new xmlrpcHelper(); 
    $answer = $xmlHelpLib->startSession($my_s_id, '_default'); 
    if ($answer[0]>0) return $answer;    

	
	if ( !glob_loggedin() ) return array( 2, "Invalid session ID !");
	
	$error  = & ErrorHandler::get();
	$varcol = & Varcols::get();
	$sqlo   = logon2();
	
	$tablename='CONCRETE_PROTO';
	$t_rights = tableAccessCheck( $sqlo, $tablename );
	if ( $t_rights["write"] != 1 ) {
		return array( 3, 'no role right "write" on table '.$tablename );
	}
	
	
	$barcodeArr = dosomething();
	if ($error->Got(READONLY))  { 
		$errprio=2;
		$FUNCNAME='DEFAULT_xml';
		$errtext = $error->getAsTextAndLog( $FUNCNAME, $errprio );
		return array( 3, $errtext );
	}
	
	return array( 0, "o.k." );
}