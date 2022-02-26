<?php
require_once ("lev1/o.CHIP_READER.sicCalcOne.inc");

/**
 * device: Wartungszeiten autom. Berechnen, fuer ein Geraet
 * @package o.CHIP_READER.siccalc.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq Task:0001458: g > Gerätewartung (Concept); SUBREQ:002 Wartungszeiten autom. Berechnen
 * @param $_REQUEST['id'] id of CYTCART
 */
class o_CHIP_READER_siccalc_XPL extends gPlugin {

	
	function register() {
		$tablename	= 'CHIP_READER';
		$title		= 'Berechne geplante Service Termine';
		
		$this->infoarr			 = NULL;
		$this->infoarr['scriptID'] = '';
		$this->infoarr['title']    = $title;
		$this->infoarr['form_type']= 'obj'; 
		$this->infoarr['design']   = 'norm';
		$this->infoarr['obj_name'] = $tablename;
		$this->infoarr['obj_id']   = $_REQUEST['id'];
		$this->infoarr['checkid']  = 1;
		
		
		$this->infoarr2 = array( 
				'accCheck' => array('tab'=>array('read','write'), 'obj'=>array('insert') )
				);
				
		
	}
	

	function startMain() {
		global $error;
		//$FUNCNAME='startMain';
		
		$this->objid   = $_REQUEST['id'];
		$sqlo = &$this->_sqlo;
		//$sqlo2 = &$this->_plug_regSql2();
		
		$devPlanCalcLib = new oDEV_sicCalcOne();
		$ini_options=array();
		$ini_options['force_planning']=1;
		$devPlanCalcLib->initDevice($sqlo, $this->objid, $ini_options);
		$numEntries = $devPlanCalcLib->createEntries($sqlo);
		
		$this->_html->line_out('Entstandene Termine: '.$numEntries);
		
		$UserInfoArr = $devPlanCalcLib->getUserInfoArr();
		if (is_array($UserInfoArr) ) 
			$this->_html->line_out('Nutzerinfos: <ul>'.implode("<br>",$UserInfoArr).'</ul>');
		
	}
	
}