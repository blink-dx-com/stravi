<?php
require_once 'o.S_OBJLINK.subs.inc';

/**
 * ABS_CYTCART analysis methods
 * @package o.ABS_CYTCART.subs.inc
 * @swreq UREQ:0001031: o.ABS_CYTCART > model features 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class oABS_CYTCART_subs {
	
	function __construct($abscart_id=0) {
		if ($abscart_id>0) $this->setAbsCart($abscart_id);
	}
	function setAbsCart($abscart_id) {
		$this->abscart_id=$abscart_id;
	}
	
	/**
	 * get product-type array from ABS_CYTCART
	 * @param $sqlo
	 * @param $ABS_CYTCART_ID
	 * @return array ['assayno'], ['platform']
	 */
	function getProductTypeArr(&$sqlo) {
		global $varcol;
		$varcol = & Varcols::get();
		
		if (!$this->abscart_id) sys_error_my( 'class '.__CLASS__.' is not initialized.' );
		$ABS_CYTCART_ID = $this->abscart_id;
		
		$sqlsel = "EXTRA_OBJ_ID from ABS_CYTCART where ABS_CYTCART_ID=".$ABS_CYTCART_ID;
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$extra_obj_id = $sqlo->RowData[0];
		
		if (!$extra_obj_id) return;
		$values_tmp = $varcol->select_by_name  ( $extra_obj_id );
	
		return $values_tmp['values'];
	}
	
	// get ExpiryDays of abs_cart
	function getExpiryDays(&$sqlo) {
		if (!$this->abscart_id) sys_error_my( 'class '.__CLASS__.' is not initialized.' );
		
		$varioLib = new oS_VARIO_sub('ABS_CYTCART');
		$ExpiryDays = floatval($varioLib->getValByKey( $sqlo, $this->abscart_id, 'ExpiryDays' ));
		return $ExpiryDays;
	}
	
	/**
	 * get BOM array (array of ABSTRACT_PROTO) Materialstammsatz
	 * @param $sqlo
	 */
	function getBOM_arr(&$sqlo) {
		if (!$this->abscart_id) sys_error_my( 'class '.__CLASS__.' is not initialized.' );
		
		$objlinkLib = new oS_OBJLINK_subs();
		$stammSatzArr = $objlinkLib->getChiObjByChiTab($sqlo, 'ABS_CYTCART', $this->abscart_id, 'ABSTRACT_PROTO');
		return $stammSatzArr;
	}
}
