<?php
/**
 * @namespace core::xmlrpc::meta
 */

/**
 * insert/update GLOBALS key/value pair
 *
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param   array $f_param <pre>
 * 	 [0] NAME
 *   [1] VAL
 * 	</pre>
 * @return array
 * @throws exception
 * @see     "usedby:external:???"
 * 
 */
class oGLOBALS_update_XRPC extends XRPC_BASE {
	
function __construct() {
}

function __checkParams() {
	global $error;
	$FUNCNAME= '__checkParams';
	
	$sqlo = $this->_sqlo;
	$tablename='GLOBALS';
	$t_rights = tableAccessCheck( $sqlo, $tablename );
	if ( $t_rights["write"] != 1 ) {
		$answer = getRawTableAccMsg( $tablename, 'write' );
		$error->set( $FUNCNAME, 1, $answer );
		return -1;
	}
	
	$KEY=$this->_params[0];
	$VAL=$this->_params[1];
	if ($KEY==NULL) {
		$error->set( $FUNCNAME, 2, 'Key not set.' );
		return -2;
	}
	
	$allowKeys=array('DbUserMessage','DbLoginDeny'); // allowed keys
	if (!in_array($KEY, $allowKeys)) {
		$error->set( $FUNCNAME, 3, 'Key "'.$KEY.'" not allowed.' );
		return -3;
	}
	

	return 1;
}

// update table
function &run() {
	$KEY=$this->_params[0];
	$VAL=$this->_params[1];
	$sqlo = $this->_sqlo;
	$tablename='GLOBALS';
	
	$isSet = 0; // row exists ?
	$sqlo->query("select value, notes from globals where NAME=".$sqlo->addQuotes($KEY)."");
	if ($sqlo->ReadRow() ) {
		$isSet   = 1;
	}
	if ($VAL==NULL) $VAL='';
	
	if ($isSet) {
		require_once ("glob.obj.update.inc");
		$UpdateLib = new globObjUpdate();
		$args = array("vals"=>array('VALUE'=>$VAL));
		$UpdateLib->update_meta( $sqlo, $tablename, $KEY, $args );

	} else {
		require_once ("insertx.inc");
		$inserLib  = new  insertC();
		$args = array("vals"=>array('NAME'=>$KEY, 'VALUE'=>$VAL));
		$inserLib->new_meta($sqlo, $tablename, $args);	
	}
	
}

}