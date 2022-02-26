<?php
require_once 'o.A_CHIP_READER.subs.inc';
require_once 'date_funcs.inc';
require_once ("o.REA_LOG.new.inc");

/**
 * calc service entry for one device
 * @package o.CHIP_READER.sicCalcOne.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @swreq Task:0001458: g > Ger�tewartung (Concept); SUBREQ:002
 * @swreq UREQ:0001489: o.DEV > service > SUBREQ:002 Planungslauf 
 *
 */
class oDEV_sicCalcOne {
	
	var $_infoarr; // infos for user

	function __construct() {
		$this->limitLong     = 60; // from 60 days on, create only one plan entry
		$this->minimumPeriod = 2;  // the minimum plan-period to AUTOMATICLY create entries
		
		/** number of entries to create for short service periods
         *  @swreq UREQ:0001489:XREF:005 wenn PERIOD<60 Tage erzeuge z.B. 5 neue Eintr�ge
		 */
		$this->NUM_CREATE_ENTRIES_SHORT=1;  // changed from 5 to 1 (03.11.2016)
		$this->_debug = $_SESSION["userGlob"]["g.debugLevel"];
		$this->_infoarr = NULL; 
		$this->_dateNowUnx = time(); // date of NOW
	}
	
	/**
	 * - get abstract device
	 * - get DEVSIC-entries
	 * @param $sqlo
	 * @param $devid
	 * @param array $ini_options
	 *   'force_planning' : 0,1
	 */
	function initDevice(&$sqlo, $devid, $ini_options=array() ) {
		$this->devid=$devid;
		$this->devsicList = NULL;
		$this->planDetails= NULL;
		$this->_infoarr   = NULL; 
		
		$this->devFeats = glob_elemDataGet3( $sqlo, 'CHIP_READER', array('CHIP_READER_ID'=>$devid), array('A_CHIP_READER_ID', 'SICACT') );
		
		if ($this->devFeats['A_CHIP_READER_ID']==NULL) return;
		
		if (!$ini_options['force_planning']) {
    		if ($this->devFeats['SICACT']!=1) {
    			$this->_infoarr[]='Attribut "device:service planning active"=1 expected for planning';
    			return;
    		}
		}
		
		$aDevLib    = new oA_CHIP_READER_subs();
		$devsicList = $aDevLib->getDevSicList_service($sqlo, $this->devFeats['A_CHIP_READER_ID']);
		
		if (empty($devsicList)) return;
		$this->devsicList = $devsicList;
		
		foreach( $this->devsicList as $devsic_id) {
			$planFeat = glob_elemDataGet3( $sqlo, 'DEVSIC', array('DEVSIC_ID'=>$devsic_id), array('NAME','PERIOD', 'TYPE', 'SHIFTTYP') );
			if ($planFeat['PERIOD']<=0) continue; // ignore
			
			$this->planDetails[$devsic_id]=$planFeat;
		}
		reset ($this->devsicList); 
		
	}
	
	/**
	 * return number of expected total planned events for one DEVSIC
	 * @param $devsic
	 */
	function _getNumExpectPlan($devsic_id) {
		$planDetails = $this->planDetails[$devsic_id];
		$period      = $planDetails['PERIOD'];
		$expectNum   = 1;
		if ($period < $this->limitLong)     $expectNum = $this->NUM_CREATE_ENTRIES_SHORT;
		if ($period < $this->minimumPeriod) {
			$expectNum = 0; // never create entries; but may be other 
			$this->_infoarr[]='plan-id:'.$devsic_id.': too small period for planning (min:'.$this->minimumPeriod.').';
		}
		return $expectNum;
	}
	
	/**
	 * calc new date
	 * @swreq UREQ:1489:XREF:004  nimm als Referenzdatum zur Datumsberechnung:
	 * - den letzten entry aus REA_LOG; dort den Wert aus der Spalte XDATE (Start Datum)
     * - Beispiel: $newDateUnx = XDATE + PLAN['PERIOD']
	 * @param $sqlo
	 * @param string $xdate     last planned date as YYYY-MM-DD
	 * @param $devsic_id
	 */
	function _calcPlanDate(&$sqlo, $xdate, $devsic_id) {
		$planDetails = $this->planDetails[$devsic_id];
		$entryUnx    = date_str2unix( $xdate, 5 );
		
		$newDateUnx  = $entryUnx + date_unixDay2Sec( $planDetails['PERIOD'] );
		
		// check 5day, 7day
		$shiftyp = $planDetails['SHIFTTYP'];
		if (!$shiftyp) $shiftyp=5;
		
		if ($shiftyp==5) {
			// @swreq UREQ:0001489:XREQ:001 unterscheide: 5-Tagewoche, 7-Tagewoche 
			// DATE not on SAT and SUN => put to monday
			// correct to next monday
			$dayNum = date('w',$newDateUnx); // 1 - MO
			if (!$dayNum) $dayNum=7; // sunday
			if ($dayNum>=6) {	
				$diffDays = 8-$dayNum; // difference from $newDateUnx to next monday
				$newDateUnx = $newDateUnx + date_unixDay2Sec( $diffDays );
			}
		}
		return $newDateUnx;
	}
	
	/**
	 * create one entry
	 * @param $sqlo
	 * @param $newDateUnx unix-time-stamp
	 * @param $devsic_id
	 */
	function _createOne( &$sqlo, $newDateUnx, $devsic_id ) {
		
		$planDetails = $this->planDetails[$devsic_id];
		$newDateStr  = date_unix2datestr( $newDateUnx,3 );
		
		$argus = array(
			'DEVSIC_ID'=>$devsic_id,
			'XDATE'   => $newDateStr,
			'NAME'    => $planDetails['NAME'],
			'KEY'	  => 'Service',
			'ACCEPT'  => 1 // planned
		);
		
		$ReaLogNew = new oREA_LOG_new();		
		$ReaLogNew->setDevice($this->devid);
		$ReaLogNew->addEntry( $sqlo, $argus);
	}
	
	function _getLastEntryDate(&$sqlo, $devsic_id) {
		//global $error;
		//$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		
		$sqlsel = 'POS, XDATE from REA_LOG where CHIP_READER_ID='.$this->devid.' and DEVSIC_ID='.$devsic_id.' order by XDATE DESC';
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		//$pos   = $sqlo->RowData[0];
		$xdate = $sqlo->RowData[1];
		
		if ($xdate==NULL) {
			// !!!!!! TBD: is this allowed ???
			$xdate = $this->TodayStr;
		}
		return $xdate;
	}
	
	/**
	 * generate maximal X new entries
	 * @param $sqlo
	 * @param $devsic_id
	 */
	function _checkManyEntry(&$sqlo, $devsic_id) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		$createFlag=0;
		
		$expectNum = $this->_getNumExpectPlan($devsic_id);
		
		if ($this->_debug>0) debugOut('new devsic_id:'.$devsic_id.'; expectNum: '.$expectNum);
		
		// ob DEVICE:letzter service Eintrag (Status=egal) XDATE< HEUTE+1 AND (Anzahl service log in ZUKUNFT <5)
		// sammle die letzten 5 Eintr�ge f�r ACCEPT>=3; wenn weniger als 5; fuelle auf 5 auf
		// @swreq UREQ:0001489:XREQ:003 plane neue log-entries, wenn Status <=2 (Accepted); 
		//    es darf nur neu geplant werden, wenn der Service schon durchgefuehrt wurde (Status>3)
		$sqlsel = 'POS, XDATE from REA_LOG where CHIP_READER_ID='.$this->devid.' and DEVSIC_ID='.$devsic_id.
			' and ACCEPT<=3 order by XDATE DESC';
		$sqlo->Quesel($sqlsel);
		$cnt=0; // number of found planned entries
		while ($sqlo->ReadRow()) {
			//$pos   = $sqlo->RowData[0];
			$xdate = $sqlo->RowData[1];
			
			if ($cnt>=$expectNum) break;
			$cnt++;
		}
		
		if ( $cnt>=$expectNum ) {
			if ($expectNum>0)
				$this->_infoarr[]='plan-id:'.$devsic_id.': enough planned service-log-entries ('.$expectNum.')';
			return; // enough in database
		}
		
		// create !
		$tmpInfo=NULL;
		$i = 0;
		
		while ( $i<($expectNum-$cnt) ) {
			$xdate = $this->_getLastEntryDate($sqlo, $devsic_id);
			$newDateUnx = $this->_calcPlanDate($sqlo, $xdate, $devsic_id);
			if ($this->_debug>0) debugOut('last DATE: '.$xdate);
			$tmpInfo[]=date_unix2datestr( $newDateUnx,3 );
			$this->_createOne($sqlo, $newDateUnx, $devsic_id);
			$createFlag++;
			$i++;
		}
		
		if (sizeof($tmpInfo)) {
			$this->_infoarr[]='- neue Eintraege: '.implode(',',$tmpInfo);
		}
		
		return $createFlag;
	}
	
	
	/**
	 * create the entries now
	 * @param $sqlo
	 */
	function createEntries(&$sqlo) {
		global $error;
		$FUNCNAME= __CLASS__.':'.__FUNCTION__;
		
		$this->numNewEntries=0;
		$numEntries = 0;
		
		if (empty($this->planDetails)) {
			$this->_infoarr[] = 'no plans exist';
			return $numEntries; 
		}
		
		$now = time();
		$calcTimeUnx = $now + date_unixDay2Sec(1);
		$this->TodayStr = date_unix2datestr( $now, 3 );
		
		
		$this->refTimeStr = date_unix2datestr( $calcTimeUnx, 3 );
		
		
		foreach( $this->devsicList as $devsic_id) {
			
			$createFlag = $this->_checkManyEntry($sqlo, $devsic_id);
	
			if ($error->Got(READONLY))  {
				$error->set( $FUNCNAME, 1, 'error for one entry' );
				return;
			}
			$numEntries = $numEntries + $createFlag;
		}
		reset ($this->devsicList);
		
		$this->numNewEntries=$numEntries;
		return $numEntries;
	}
	
	function getUserInfoArr() {
		return $this->_infoarr;
	}
}
