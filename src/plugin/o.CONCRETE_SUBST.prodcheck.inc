<?php

require_once 'o.S_VARIO.subs.inc';
/**
 * check production status of substance   
 * @package o.CONCRETE_SUBST.prodcheck.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @param $_REQUEST['id'] id of SUBSTANCE
 * @swreq UREQ:0001143: o.CONCRETE_SUBST > post production check
 */
class o_CONCRETE_SUBST_prodcheck_XPL extends gPlugin {
	
	function __construct(&$sqlo) {
		parent::__construct($sqlo);
	}
	
	function register() {
		$tablename	= 'CONCRETE_SUBST';
		$title		= 'Check production status';
		
		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'obj'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['obj_id']   = $_REQUEST['id'];
		$this->infoarr['checkid']  = 1;
		
		
		$this->infoarr2 = array( 
				'accCheck' => array('tab'=>array('read'), 'obj'=>array('read') )
				);
				
		
	}
	

	function startMain() {
		global $error;
		
		$FUNCNAME='startMain';
		
		$objid   = $_REQUEST['id'];
		$sqlo = &$this->_sqlo;
		$sqlo2 = &$this->_plug_regSql2();
		
		$absSubstID = glob_elementDataGet( $sqlo, 'CONCRETE_SUBST', 'CONCRETE_SUBST_ID', $objid, 'ABSTRACT_SUBST_ID');
		if (!$absSubstID) $this->htmlFoot('WARN', 'substance has no abstract substance');
		
		$key      = 'conc.produced.checked.plugin';
		$varioLib = new oS_VARIO_sub('ABSTRACT_SUBST');
		$pluginMod= $varioLib->getValByKey( $sqlo, $absSubstID, $key );
		if ($pluginMod==NULL) $this->htmlFoot('WARN', 'need a plugin-module-URL for SUA-ID:'.$absSubstID.' in vario-key:'.$key);
		
		$url = 'p.php?mod='.$pluginMod.'&id='.$objid;
		$this->_html->pageForward($url,'forward to plugin');
	}
	
}