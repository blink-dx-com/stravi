<?php
/**
 * lib to manage KEYS, saved in a document
 * $Header: trunk/src/phplib/o.LINK.keydoc.inc 59 2018-11-21 09:04:09Z $
 * @package o.LINK.keydoc.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
 
class oLINK_keydoc {

function open( &$sqlo, $docid ) {
	$this->notes = glob_elementDataGet( $sqlo, 'LINK', 'LINK_ID', $docid, 'NOTES');
	
}

function hasKeys() {
	if ($this->notes!=NULL) return 1;
	else return 0;
}

function getKeys() {
	$keyarr = explode(',', $this->notes);
	if ($keyarr==NULL) return;
	
	$keyarrPoi = &$keyarr;
	foreach( $keyarrPoi as $cnt=>$val) {
		$keyarr[$cnt] = trim($val);
	}
	reset ($keyarrPoi); 
	return ($keyarr);
}

}