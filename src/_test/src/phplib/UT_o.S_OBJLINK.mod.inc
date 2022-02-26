<?php
// example for testing: csv.substance.lib.xarray.inc

class UT_o_S_OBJLINK_mod_inc extends gUnitTestSub {
	
function __construct() {

}

/**
 * add child to mother
 */
function dotest( &$sqlo, $options ) {
	global $error;
	
	$CONCRETE_SUBST_ID = $this->_getDefObjID( "CONCRETE_SUBST", "default" );
	$IMG_ID            = $this->_getDefObjID( "IMG", "default" );
	$child2	 = $this->_getDefObjID( "CONCRETE_PROTO", "default" );
	
	$this->_infoNow( 'mother:',  "CONCRETE_SUBST, id:".$CONCRETE_SUBST_ID);
	// clear S_OBJLINK for mother object 
	$sqlo->deletex('S_OBJLINK',"MO_TABLE='CONCRETE_SUBST' and MO_ID=".$CONCRETE_SUBST_ID);

	$modlib = new oS_OBJLINK_mod();
	
	$modlib->setObj($sqlo, 'CONCRETE_SUBST', $CONCRETE_SUBST_ID);
	$retval = $modlib->addLink($sqlo, 'IMG', $IMG_ID, 'test');
	
	$this->_infoNow( 'added?', $retval );
	if (!$retval) return 0;
	
	$retval = $modlib->addLink($sqlo, 'IMG', $IMG_ID, 'test');
	$this->_infoNow( 'again: added?', $retval );
	if ($retval) return 0;
	
	$retval = $modlib->addLink($sqlo, 'CONCRETE_PROTO', $child2, 'test2');
	$this->_infoNow( 'other: added?', $retval );
	if (!$retval) return 0;
	
	
	return ($retval);
}

}