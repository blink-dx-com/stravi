<?php

/**
 * analyse REA_LOG service entries for one device
 * @package o.REA_LOG.servAna.inc
 * @swreq   Task:0001458: g > Geraetewartung (Concept)
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class oREA_LOG_servAna {
	
	function __construct() {
		
		
		$this->AcceptDefs=array(
			'planned' =>1,
			'finished'=>4
		);
	}
	
	function initDev(&$sqlo, $devid) {
		$this->devid = $devid;
	}
	
	/**
	 * 
	 * get service entries of REA_LOG for a time range
	 *  - KEY='Service'
	 *  - ACCEPT= ???
	 * @param $sqlo
	 * @param $dateArr
	 *   [0] time start UNIX
	 *   [1] time end   UNIX
	 * @return array timeStamps
	 *    [POS]=> time (as string)
	 */
	function getSrvOfTimeRange(&$sqlo, $dateArr) {
		
		$dateArrSql=NULL;
		$dateArrSql[0]=date_unix2datestr( $dateArr[0], 3);
		$dateArrSql[1]=date_unix2datestr( $dateArr[1], 3);
		
		$sqlsel = 'POS, XDATE from REA_LOG where CHIP_READER_ID='.$this->devid.' and KEY='. $sqlo->addQuotes('Service').
			' and XDATE>='.$sqlo->addQuotes($dateArrSql[0]).' and XDATE<='.$sqlo->addQuotes($dateArrSql[1]).
			' order by XDATE';
		$sqlo->Quesel($sqlsel);
		
		$timeStamps=NULL;
		$cnt=0; // number of found planned entries
		while ($sqlo->ReadRow()) {
			$pos   = $sqlo->RowData[0];
			$xdate = $sqlo->RowData[1];
			$timeStamps[$pos]=$xdate;
			
			$cnt++;
		}
		return $timeStamps;
	}
	
	/**
	 * 
	 * get service entries of REA_LOG for a time range for MANY devices
	 *  - take care, if too many answers !!!!
	 *  - use ONLY for small time range ?
	 *  - KEY='Service'
	 *  - ACCEPT= ???
	 * @param $sqlo
	 * @param $dateArr
	 *   [0] time start UNIX
	 *   [1] time end   UNIX
	 * @param $deviceSubSql subQuery to select the devices
	 * @return array timeStamps2
	 *    array of array($xdate, $pos, $devid);
	 */
	function getOfRangeManyDev(&$sqlo, $dateArr, $deviceSubSql) {
		
		$dateArrSql=NULL;
		$dateArrSql[0]=date_unix2datestr( $dateArr[0], 3);
		$dateArrSql[1]=date_unix2datestr( $dateArr[1], 3);
		
		$sqlsel = 'POS, XDATE, CHIP_READER_ID from REA_LOG where CHIP_READER_ID in (' .$deviceSubSql. ') and KEY='. $sqlo->addQuotes('Service').
			' and XDATE>='.$sqlo->addQuotes($dateArrSql[0]).' and XDATE<='.$sqlo->addQuotes($dateArrSql[1]).
			' order by XDATE';
		$sqlo->Quesel($sqlsel);
		
		$timeStamps=NULL;
		$cnt=0; // number of found planned entries
		while ($sqlo->ReadRow()) {
			$pos   = $sqlo->RowData[0];
			$xdate = $sqlo->RowData[1];
			$devid = $sqlo->RowData[2];
			$timeStamps2[]=array($xdate, $pos, $devid);
			
			$cnt++;
		}
		return $timeStamps2;
	}
	
}