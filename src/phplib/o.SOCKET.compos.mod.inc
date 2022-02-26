<?php
require_once ('o.ABS_SOCKET.subs.inc');

/**
 * MODIFY methods for SOCKET:Blink specific
 * @package o.SOCKET.compos.mod.inc
 * @swreq UREQ:
 * @author  Steffen Kube (steffen@blink-dx.com)
 * 
 * @var array cart_content_simple_arr_STRUCT - SIMPLE content of cartridge
 *   array[$pos] = 
*          array(
*              'step' => step NR
*              'key'  => KEY
*              'vol'  => volume (only for REAG)
*              'suc'  => ID , : ID of substance (concrete)
*              'grp'  =>
*              )
 */
class oSOCKET_compos_mod {
	
	private $objid;
	
	function __construct() {
	    $this->absCartLib = new oABS_SOCKET_subs();
	}
	
	
	/**
	 * init object
	 * @param object $sqlo
	 * @param int $objid
	 * @return array
	 *    'ABS_SOCKET_ID'
	 */
	function set_obj(&$sqlo, $objid) {

		$this->objid = $objid;
		$this->assoclib = new  fAssocUpdate();
		$this->assoclib->setObj( $sqlo, 'CART_H_SUC', $this->objid );
	}
	
	function remove_all($sqlo) {
	    $this->assoclib->delMore($sqlo);
	}

	function add($sqlo, $argu) {
	    global $error;
	    $FUNCNAME= __CLASS__.':'.__FUNCTION__;
	    
	    if (!$this->objid) throw new Exception('Class-Error: objid missing.');

	    $need_keys =array(
	        'STEP',
	        'CONCRETE_SUBST_ID',
	        'POS', 
	    );
	    foreach($need_keys as $key) {
	        if (!array_key_exists($key, $argu)) {
	            throw new Exception('key "'.$key.'" missing.');
	        }
	    }
	    
	    $sqlsel = "STEP from CART_H_SUC where SOCKET_ID=".$this->objid . ' and STEP='.$argu['STEP'];
	    $sqlo->Quesel($sqlsel);
	    if ( $sqlo->ReadRow() ) {
	        $error->set( $FUNCNAME, 1, 'STEP_NR '.$argu['STEP'].' already exists.' );
	        return;
	    }
	    
	    $this->assoclib->insert( $sqlo, $argu );
	}
	
	function close($sqlo) {
	    $this->assoclib->close($sqlo);
	}
	
}