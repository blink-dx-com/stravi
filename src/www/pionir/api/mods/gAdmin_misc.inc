<?php
/**
 * get session vars for root
 * @author Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq GLOBAL
 * @package gAdmin_getSessVars.inc
 */
class gAdmin_misc_XRPC extends XRPC_BASE {
	
	
	/**
	 * return some session vars
	 */
	function &run() {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$sqlo = $this->_sqlo;
		$action = $this->_params['action'];
	
		if ( !glob_isAdmin() ) {
			$error->set( $FUNCNAME, 1, 'Only allowed for admin.' );
			return;
		}
		
		if($action==NULL) return;
		
		$infoarr=NULL;
		switch ($action) {
		    case 'get_globals':
		        $infoarr = $_SESSION['globals'];
		        break;
		}
    	
		return $infoarr;
	}
}
