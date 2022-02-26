<?php
// example for testing
require_once('o.S_OBJLINK.mod.inc');

class UT_o_S_OBJLINK_subs_inc extends gUnitTestSub {
	
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

	$analyselib = new oS_OBJLINK_subs();
	$modlib     = new oS_OBJLINK_mod();
	
	$modlib->setObj($sqlo, 'CONCRETE_SUBST', $CONCRETE_SUBST_ID);
	$retval = $modlib->addLink($sqlo, 'IMG', $IMG_ID, 'test');
	
	$this->_infoNow( 'added?', $retval );
	
	$moArray = $analyselib->getMoObjbyKey($sqlo, 'IMG', $IMG_ID, 'test');
	$this->_infoNow( 'mother?', print_r($moArray,1) );
	
	$moArray = $analyselib->getMoObjbyKey($sqlo, 'IMG', NULL, 'test'); // allowed; but stupid
	$this->_infoNow( 'mother?', print_r($moArray,1)  );
	
	$result = $analyselib->getChiObjByTabOrKey($sqlo, 'CONCRETE_SUBST', $CONCRETE_SUBST_ID, 'IMG');
	$this->_infoNow( 'children?', print_r($result,1)  );
	
	$result = $analyselib->getChiObjByTabOrKey($sqlo, 'CONCRETE_SUBST', NULL, 'IMG');
	$this->_infoNow( 'children?', print_r($result,1)  );
	
	if (!$retval) return 0;
	
	
	return ($retval);
}

}