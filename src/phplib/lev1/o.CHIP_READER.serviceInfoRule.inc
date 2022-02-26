<?php
require_once("date_funcs.inc");
require_once 'o.CCT_ACCLOG.subs.inc';

/**
 * get list of devices (+service-dates) of QC devices with urgent service-dates
 * - use the new structure : REA_LOG: KEY="Service"
 * @package o.CHIP_READER.serviceInfoRule.inc
 * @swreq Task:0001458: g > Geraetewartung (Concept)
 * @swreq UREQ:0000077: o.CHIP_READER > QC_Check > next maintainace 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class oCHIP_READER_servicInfoRule {
	
	var $savedErr;
	var $statusIdFinished;

	function __construct(&$sqlo) {

		$table_name = "CHIP_READER";
		$this->savedErr = NULL;
		$this->statusIdFinished = oH_ALOG_ACT_subs::getH_ALOG_ACT_ID($sqlo, $table_name, 'finished');
	}
	
	/**
	 * get basic SQL-condition regulated devices
	 *
	 * @swreq UREQ:0001287: o.CHIP_READER > QC_Check > finished device 
	 * @return string $sqlAfter
	 */
	function _getDevices_SQL() {
		
		$addAuditSql = NULL;
		if ($this->statusIdFinished) {
			// ignore finished devices
			$addAuditSql = " and (a.H_ALOG_ACT_ID!=".$this->statusIdFinished." or a.H_ALOG_ACT_ID is NULL)";
		}
		$sqlAfter = " from CHIP_READER x ".
			" JOIN CCT_ACCESS a ON x.CCT_ACCESS_ID=a.CCT_ACCESS_ID" .
			" where IS_REGUL>0 " . $addAuditSql;
		return ($sqlAfter);
	}
	
	/**
	 * get ALL devices with NON-accepted Service-entries XDATE < $refDateUnx
	 * @param $sqlo
	 * @param $refDateUnx
	 */
	function getUrgentDevices(&$sqlo, $refDateUnx ) {
		
		$refDateSql = date_unix2datestr(  $refDateUnx,  3 );
		$devinfo = NULL;
		$sqlAfter = $this->_getDevices_SQL( );
		$sqlseREAD = "(select x.CHIP_READER_ID ". $sqlAfter.")";
		
		$sqlsel    = "x.CHIP_READER_ID, x.POS, x.XDATE from REA_LOG x where" .
			" KEY='Service' and ACCEPT<2 and XDATE<" . $sqlo->addQuotes($refDateSql) .
			" and x.CHIP_READER_ID in ". $sqlseREAD . 
			" order by x.CHIP_READER_ID, x.XDATE";
		
		$sqlo->Quesel($sqlsel);
		$lastDevice = 0;
		while ( $sqlo->ReadRow() ) {
			$objid = $sqlo->RowData[0];
			$pos   = $sqlo->RowData[1];
			$maintDate = $sqlo->RowData[2];
			
			if ($objid==$lastDevice) continue; // ignore last same device
			$lastDevice = $objid; // update
			
			$devinfo[$objid] = array("exdate"=>$maintDate, "typ"=>"W");
		}
		return ($devinfo);
	}
	
	/**
	 * get devices selected in $sqlSubSel with:
	 * - NON-finished Service-entries XDATE < $refDateUnx
	 * - device must not have STATUS=finished
	 * @swreq UREQ:0001638: o.DEV > service > List of devices> zeige naechste/nicht durchgefuehrte Service-Termin 
	 * @param $sqlo
	 * @param $sqlSubSel sql-sub-selection, start with FROM: e.g.: FROM CHIP_READER where x.CHIP_READER_ID<500
	 * @param $refDateUnx
	 * @return array of
	 * 	array('dev'=>$objid, 'pos'=>$pos, 'exdate'=>$maintDate, 'name'=>$name, 'substatus'=>$substatus)
	 */
	function getUrgentSelDevs(&$sqlo, $sqlSubSel, $refDateUnx ) {
		
		$refDateSql = date_unix2datestr(  $refDateUnx,  3 );
		$devinfo = NULL;
		
		$addAuditSql = NULL;
		if ($this->statusIdFinished) {
			// ignore finished devices
			$addAuditSql = " and (a.H_ALOG_ACT_ID!=".$this->statusIdFinished." or a.H_ALOG_ACT_ID is NULL)";
		}
		
		$sqlseREAD = "(select x.CHIP_READER_ID from ". $sqlSubSel . $addAuditSql.")";
		
		
		$sqlsel    = "x.CHIP_READER_ID, x.POS, x.XDATE, x.NAME, x.ACCEPT from REA_LOG x where" .
			" KEY='Service' and ACCEPT<4 and XDATE<" . $sqlo->addQuotes($refDateSql) .
			" and x.CHIP_READER_ID in ". $sqlseREAD . 
			" order by x.CHIP_READER_ID, x.XDATE";
		
		$sqlo->Quesel($sqlsel);
		//$lastDevice = 0;
		while ( $sqlo->ReadRow() ) {
			$objid     = $sqlo->RowData[0];
			$pos       = $sqlo->RowData[1];
			$maintDate = $sqlo->RowData[2];
			$name      = $sqlo->RowData[3];
			$substatus = $sqlo->RowData[4];
			
			$devinfo[] = array('dev'=>$objid, 'pos'=>$pos, 'exdate'=>$maintDate, 'name'=>$name, 'substatus'=>$substatus);
		}
		return ($devinfo);
	}
	
	/**
	 * get all active devices of user: CONDITION only
	 * @param object $sqlo
	 * @param int $user_id
	 * @return string
	 */
	public static function getAllDevOneUser_SQL_W($sqlo, int $user_id) {
	    $tablename='CHIP_READER';
	    $addAuditSql = NULL;
	    
	    $statusIdFinished = oH_ALOG_ACT_subs::getH_ALOG_ACT_ID($sqlo, $tablename, 'finished');
	    $addAuditSql = NULL;
	    if ($statusIdFinished) {
	        // ignore finished devices
	        $addAuditSql = " and (a.H_ALOG_ACT_ID!=".$statusIdFinished." or a.H_ALOG_ACT_ID is NULL)";
	    }
	    $sqlWhere  = 'x.RES_USER_ID='.$user_id . $addAuditSql;
	    return $sqlWhere;
	}
	
	/**
	 * get all active devices of user: BASE SQL
	 * @param object $sqlo
	 * @param int $user_id
	 * @return string
	 */
	public static function getAllDevOneUser_SQL_B($sqlo, int $user_id) {
	    $sqlWhere  = self::getAllDevOneUser_SQL_W($sqlo, $user_id);
	    $sql_after = 'CHIP_READER x JOIN CCT_ACCESS a ON x.CCT_ACCESS_ID=a.CCT_ACCESS_ID'.
	   	    ' where '.$sqlWhere;
	    return $sql_after;
	}
	
	/**
	 * get urgent DEVICES of the RES_USER_ID
	 * - get EARLIEST entry in REA_LOG, where
	 * - KEY="Service", ACCEPT<2 (geplant), XDATE < $refDateUnx
	 * @param string &$sqlo
	 * @param int    $user_id
	 * @param string $refDateUnx reference date
	 * @return mixed devices = array( ID=>array( exdate=>expiryDate, typ=>Wartung|Verifizierung ) )
	 */
	function getDevOneContact(&$sqlo, $user_id, $refDateUnx) {
		
		$refDateSql = date_unix2datestr(  $refDateUnx,  3 );
		
		// select devices dor this contact
		$sqlAfter  = $this->_getDevices_SQL( );
		$sqlseREAD =  "select x.CHIP_READER_ID" . $sqlAfter." and ".
			" (x.RES_USER_ID=". $user_id .")";
		
		// es koennen mehrere Antworten pro device X kommen ...
		$sqlsel    = "x.CHIP_READER_ID, x.POS, x.XDATE from REA_LOG x where" .
			" x.KEY='Service' and x.ACCEPT<2 and x.XDATE<" . $sqlo->addQuotes($refDateSql) .
			" and x.CHIP_READER_ID in (". $sqlseREAD . ")" .
			" order by x.CHIP_READER_ID, x.XDATE";
		
		$sqlo->Quesel($sqlsel);
		$lastDevice = 0;
		$devinfo = array();
		
		while ( $sqlo->ReadRow() ) {
			$objid = $sqlo->RowData[0];
			// $pos   = $sqlo->RowData[1];
			$maintDate = $sqlo->RowData[2];
			
			if ($objid==$lastDevice) continue; // ignore last same device
			$lastDevice = $objid; // update
			$devinfo[$objid] = array("exdate"=>$maintDate, "typ"=>"W");
			
		}
		return $devinfo;
	}
	
	/**
	 * get all responsible users RES_USER_ID of devices 
	 * - do NOT analyse the service-dates here !
     *
	 * - analyse all IS_REGUL devices
	 * - daraus Contact-Liste UX
	 * - f�er jeden Contact CONE aus Contact-Liste UX ...
	 *   - alle QC-Geraete holen : REA_LOGS auswerten
	 * @param string &$sqlo
	 * @param string  $refDateUnx reference date
	 * @return array ( DB_USER_ID )
	 */
	function getUrgDevPerUser(&$sqlo, $refDateUnx ) {
	    
		$userinfo = array();
		$sqlAfter = $this->_getDevices_SQL();
		$sqlseREAD =  "(select x.CHIP_READER_ID" . $sqlAfter.")";
		$sqlsel    = "x.RES_USER_ID from CHIP_READER x ".
					 " where x.CHIP_READER_ID in ". $sqlseREAD;
		
		$sqlo->Quesel($sqlsel);
		while ( $sqlo->ReadRow() ) {
			$user_id  = $sqlo->RowData[0];
			if(!in_array($user_id,$userinfo)) $userinfo[]=$user_id;
		}
		return $userinfo;
	}
	
	/**
	 * get service info for one device
	 * - 'open_service_date': is there a sevrice entry, which is still open, but due-date < $refDateUnx ?
	 * @param object $sqlo
	 * @param int  $devid
	 * @param int $refDateUnx
	 * @return array(
	 *  'last_serv_date_ok' : last good service
	 *  'open_service_date' : date, if there is an expected service open)
	 */
	public function oneDevice_serviceInfo1(&$sqlo, $devid, $refDateUnx) {
		$FLAG_FINISHED=4;
		$refDateSql = date_unix2datestr(  $refDateUnx,  3 );
		$devinfo = array();

		$open_service_date = NULL;	
		$sqlsel    = "x.POS, x.XDATE from REA_LOG x where" .
				" KEY='Service' and ACCEPT<2 and XDATE<" . $sqlo->addQuotes($refDateSql) .
				" and x.CHIP_READER_ID=".$devid. ' order by x.POS desc';
		$sqlo->Quesel($sqlsel);
		//$lastDevice = 0;
		if ( $sqlo->ReadRow() ) {
			$pos_bad           = $sqlo->RowData[0];
			$open_service_date = $sqlo->RowData[1];	
		}
		
		$devinfo['open_service_date'] = $open_service_date;
		
		$last_serv_date_ok = NULL;
		$sqlsel    = "x.POS, x.XDATE from REA_LOG x where" .
				" KEY='Service' and ACCEPT=".$FLAG_FINISHED." and XDATE<" . $sqlo->addQuotes($refDateSql) .
				" and x.CHIP_READER_ID=".$devid. ' order by x.POS desc';
		$sqlo->Quesel($sqlsel);
		//$lastDevice = 0;
		if ( $sqlo->ReadRow() ) {
			$pos_good          = $sqlo->RowData[0];
			$last_serv_date_ok = $sqlo->RowData[1];
		}
		
		$devinfo['last_serv_date_ok'] = $last_serv_date_ok;
		
		return $devinfo;
	}

}

