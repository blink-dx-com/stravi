<?php
/**
 * @namespace core::xmlrpc
 */


/**
 * test if session is alive (user is logged in)
 *
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param   array $f_param <pre>
 * 	 [0] sess_id
 * 	</pre>
 * @return array   0 : ok, >0 not alive
 * @see     "usedby:external:ICONOCLUST"
 * 
 */
function g_sessionalive_xml(
	$f_name, 
	$f_param
	)
{
    if (sizeof($f_param) < 1) return array(1, "Incorrect number of parameters.");
	$my_s_id  = $f_param[0];
	
	$retval = 0;
    $xmlHelpLib = new xmlrpcHelper(); 
    $answer = $xmlHelpLib->startSession($my_s_id, 'g_sessionalive'); 
    if ($answer[0]>0) return $answer;    

    
    if ( !glob_loggedin() ) {
    	$retval = 1;
    	$rettxt = 'session invalid.';
    } else {
    	$retval = 0;
    	$rettxt = 'o.k.';
    }
	
	
	return array($retval, $rettxt);
}
