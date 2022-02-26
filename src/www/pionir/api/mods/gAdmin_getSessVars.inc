<?php
/**
 * get session vars for root
 * @author Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq GLOBAL
 * @package gAdmin_getSessVars.inc
 */
class gAdmin_getSessVars_XRPC extends XRPC_BASE {
	
	
	/**
	 * return sessionvars
	 */
	function &run() {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$sqlo = $this->_sqlo;
	
		if ( !glob_isAdmin() ) {
			$error->set( $FUNCNAME, 1, 'Only allowed for admin.' );
			return;
		}
		
		$infoarr=NULL;
		$infoarr['s_sessVars'] = $_SESSION['s_sessVars'];
    	
		return $infoarr;
	}
}
