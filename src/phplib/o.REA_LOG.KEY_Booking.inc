<?php

/**
 * analyse REA_LOG KEY=Booking for one device
 * @package o.REA_LOG.KEY_Booking.inc
 * @swreq UREQ:9230 Geraete buchen; FS:FS-QA04-e
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/phplib/o.REA_LOG.KEY_Booking.inc 59 2018-11-21 09:04:09Z $
 */
class oREA_LOG_KEY_Booking {
	
	function __construct() {
		$this->KEY = 'Booking';
	}
	
	function initDev(&$sqlo, $devid) {
		$this->devid = $devid;
	}
	
	/**
	 * check Booking dates
	 * examples of possible time ranges: x-start; y-end
	 * ??:      XD x+++++++++y XD2
	 * P2      x+++++y
	 * P3                x+++++y
	 * P4    x+++++++++++++++y
	 * @param unknown $sqlo
	 * @param string $date1
	 * @param string $date2
	 * @param int $ignorePos ignore this existing entry
	 */
	function checkBlock(&$sqlo, $date1, $date2, $ignorePos=0) {
		
		$sqlsel = 'POS from REA_LOG where CHIP_READER_ID='.$this->devid.' and KEY='. $sqlo->addQuotes($this->KEY).
		' and XDATE<='.$sqlo->addQuotes($date2).' and XDATE2>='.$sqlo->addQuotes($date1).' and POS !='.$ignorePos.
		' order by XDATE';
		$sqlo->Quesel($sqlsel);
		
		$timeStamps=NULL;
		$cnt=0; // number of found planned entries
		if ($sqlo->ReadRow()) {
			$pos   = $sqlo->RowData[0];
			return array(1,'Time-Conflict on POS:'.$pos);
		}
		return array(0,'');
	}
	
	
}