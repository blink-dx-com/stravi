<?php
/**
 * - manage file confInst.local.inc
 * 
 * $Header: trunk/src/www/pionir/rootsubs/db_imp/g.confInstFileSub.inc 59 2018-11-21 09:04:09Z $
 * @package g.confInstFileSub
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */

// pure actions
class gConfInstSub {

function loadFile() {
	global $error;
	$FUNCNAME = "loadFile";
	
	$this->conffile = 'confInst.local.inc';
	$filefull = "../../../../config/".$this->conffile;
	if (!file_exists($filefull ) )  {
		$error->set( $FUNCNAME, 1, "configfile '".$filefull."' not found!" );
		return;
	}
	require_once ($filefull);
	global $error;
	$FUNCNAME = "loadFile";
	
	$this->confvars = gConfigInstGet();
}

function login($pw) {
	global $error;
	$FUNCNAME = "login";
	$confvars = $this->confvars;
	
	if ($confvars["installpass"]=="") {
		$error->set( $FUNCNAME, 1, "Variable \$confvars[\"installpass\"] in ".$this->conffile." must be set.".
				"Please ask the admin." );
		return;
	}
	
	if ( $confvars["installpass"] != $pw ) {
		$error->set( $FUNCNAME, 1, "Password does not match." );
		return;
	}
	
	$install_admin = array( "login"=>1 );
	return ($install_admin);
}

function getVal($key) {
	return ($this->confvars[$key]);
}

}