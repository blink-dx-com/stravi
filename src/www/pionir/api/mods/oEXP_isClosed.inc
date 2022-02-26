<?php
/**
 * 
 */

/**
 * Checks if an experiment is wether closed/released or not. 
 * @author  Michael Brendel (michael.brendel@clondiag.com)
 * @param
 * array $f_param:
 *  <pre>
 * 	array (
 * 		[0] sess_id,
 * 	 	[1] exp_id
 *  )
 * 	</pre>
 * @return
 * 	<pre>
 * 	array (
 * 		[0] errorcode,	(0 if no error occured)
 * 		[bool]	- 0, if not closed and not released
 * 				- 1, if closed or released
 *  )
 * 	</pre>
 * 
 * @swreq SREQ:0003467: g > XMLRPC > o.EXP > close()
 */

require_once ('o.CCT_ACCLOG.subs.inc');

class oEXP_isClosed_XRPC extends XRPC_BASE {
	

	
	function __checkParams() {
		global $error;
		$FUNCNAME= '__checkParams';
		
		$sqlo = $this->_sqlo;
		
		$tablename='EXP';
		$objectId = $this->_params[0];
		
		$this->__objectCheck($sqlo, $tablename, $objectId);
		if ( $error->got(READONLY) ) {
			$error->set( $FUNCNAME, 1, "No object with ID: ".$objectId." found in table ".$tablename.".");
			return;
		}	
		return 1;
	}
	
	function &run() {
		//global $error;
		//$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$table='EXP';
		$sqlo = $this->_sqlo;
	  	$exp_id = $this->_params[0];
	  	
	  	$accLogLib = new oAccLogC();
	  	$accLogLib->setObject($sqlo, $table, $exp_id);
	  	$statusID = $accLogLib->getLastLog($sqlo);
	  	if (!$statusID) return 0;
	  	$stateClosed_ID = oH_ALOG_ACT_subs::getH_ALOG_ACT_ID( $sqlo, $table, 'closed' );
	  	if ($statusID==$stateClosed_ID or $accLogLib->currentIsReleased($sqlo)) { // is closed or released ?
	  		return 1;
	  	} else {
	  		return 0;
	  	}
	}
}