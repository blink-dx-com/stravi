<?php
/**
 * Logout
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @module gLogout.inc
 * @package gLogout
 * @param None
 * @return
 * 	array (
 * 		'ok'=>1
 *  )
 */
class gLogout_XRPC extends XRPC_BASE {
    
    /**
     * @param None
     * @return
     * 	array (
     * 		'ok'=>1
     *  )
     */
	function &run() {
	    session_destroy();
	    return array('ok'=>1);
	}
}
