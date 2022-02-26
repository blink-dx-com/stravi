<?php

/**
 * 
 * Lock Experiment and all related Components (CONCRETE_PROTOCOL, IMG) 
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
 *  )
 * 	</pre>
 * 
 * @swreq SREQ:0003467: g > XMLRPC > o.EXP > close()
 */
require_once ('access_lock.inc');
require_once ('glob.obj.touch.inc');
require_once ('o.CCT_ACCLOG.subs.inc');

class oEXP_close_XRPC extends XRPC_BASE {
	
	//constructor
	function __construct() {
	}
	
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
		global $error;
		$FUNCNAME= get_class($this).'->run()';

		$table = 'EXP';
		$sqlo = $this->_sqlo;
	  	$exp_id = $this->_params[0];
	  	
	  	//lock experiment and children
	  	$objLockLib = new gObjaccLock();
	  	$objLockLib->accessLockOne( $sqlo, $table, $exp_id);
	  	
	  	if ( $error->got(READONLY) ) {
	  		$error->set( $FUNCNAME, 2, "Error while locking Experiment: ".$exp_id." or one of the children (CONCRETE_PROTO, IMG).");
	  		return;
	  	}
	  	
	  	//touch experiment
	  	$actarr = array (
	  					'd'=> array('x'=>array('acc'=>NULL) ),
	  					'a'=> array('key'=>'acc', 'data'=>array('func'=>'lock') )
	  			);
	  	$objTouchLib = new globObjTouch();
	  	$objTouchLib->touch($sqlo, $table, $exp_id, $actarr);

	  	//set state to closed
	  	$argu=NULL; // argus for INSERT
	  	$accLogLib = new oAccLogC();
	  	$argu['action'] = oH_ALOG_ACT_subs::getH_ALOG_ACT_ID( $sqlo, $table, 'closed' );
	  	$accLogLib->addLogObj($sqlo, $table, $exp_id, $argu);
	  	
	  	if ( $error->got(READONLY) ) {
	  		$error->set( $FUNCNAME, 3, "Error while setting state of Experiment: ".$exp_id." to \"closed\".");
	  		return;
	  	}
	  	
		return;
	}
}