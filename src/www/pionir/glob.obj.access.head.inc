<?php
/**
 * header for TOOLS: object access > xxx
 * $Header: trunk/src/www/pionir/glob.obj.access.head.inc 59 2018-11-21 09:04:09Z $
 * @package glob.obj.access.head.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
require_once ("f.rider.inc");

class gObjAccessHead {

function __construct( $xmode, $table, $objid) {
	
	
	$this->xmode=$xmode;
	$baseurl = 'glob.obj.access.php?t='.$table.'&id='.$objid.'&xmode=';
	$this->xmodes=array(
		'perm' => array('permissions', $baseurl. 'perm'),
		'meta' => array('meta features', 'glob.obj.feat.php?id='.$objid.'&t='.$table),
		'audit'=> array('audit trail', $baseurl. 'audit'),
		'mod'  => array('modification log', 'obj.cct_acc_up.showobj.php?id='.$objid.'&t='.$table )
	);
	//$this->relLink = '<font color=gray>related:</font> <a href="glob.obj.feat.php?t='.$table.'&id='.$objid.'">properties</a>';
}

function setMode( $xmode ) {
	$this->xmode=$xmode;
}

function showNavTab() {
	$ridlib = new fRiderC();
	$RidTableName=NULL;
	
	$ridlib->riderShow( $this->xmodes, $this->xmode, $RidTableName );
}

}
 