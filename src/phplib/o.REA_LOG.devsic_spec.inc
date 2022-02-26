<?php
/**
 * REA_LOG:DEVSIC_ID specials
 * - transform parameters for REA_LOG:XDATA
 * - show special form for REA_LOG:XDATA if KEY=Service
 * @package o.REA_LOG.devsic_spec.inc
 * @swreq UREQ:UREQ:0001458: g > Ger�tewartung (Concept)
 * @swreq FS-QA04-g
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
require_once ('o.REA_LOG.subs.inc');
require_once 'o.DEVSIC.subs.inc';

/**
 * transformation of form parameters to UPDATE-parameters for NOTES
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq SREQ:0001488: o.REA_LOG > edit one existing entry 
 */
class oREA_LOG_infoarrMod {
	
	
	/**
	 * append special FORM-data to notes
	 * @param $rowx : parameter array for service entries
		array ( index => array(
		 KEY - title
	     STATUS - value
	     NOTES - notes
	     ) )
	 * @param string $xdata_str
	 */
	function formPar2XDATA($rowx) {

		if (!is_array($rowx)) {
			return array();
		}
		
		$xdata=array();
		foreach( $rowx as $valarr) {
			if ($valarr['KEY']==NULL or ($valarr['STATUS']==NULL and $valarr['NOTES']==NULL)) continue; // no data
			$xdata[]=$valarr;
			
		}
		$xdata_str=json_encode($xdata);
		return $xdata_str;
	}
}

