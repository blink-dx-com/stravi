<?php

/**
 * subs for A_CHIP_READER
 * @package o.A_CHIP_READER.subs.inc
 * @swreq UREQ:0000019 global
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class oA_CHIP_READER_subs {

	
	/**
	 * get all SERVICE entries of DEVSIC; exclude types like IPC
	 * @param $sqlo
	 * @param $adevid
	 * @return array $planids
	 */
	static function getDevSicList_service(&$sqlo, $adevid) {
		
		// exclude following PLAN-Types
		$non_service_types = array('IPC');
		$non_temp_arr  = array();
		foreach($non_service_types as $onetype) {
			$non_temp_arr[]=$sqlo->addQuotes($onetype);
		}
		$temp_str = implode(',',$non_temp_arr);
		$sql_add = ' and DEVSIC_ID in (select DEVSIC_ID from DEVSIC where TYPE not in ('.$temp_str.'))';
		
		$planids=NULL;
		$sqlsel = "DEVSIC_ID from ADEV_HAS_SIC where A_CHIP_READER_ID=".$adevid. $sql_add. ' order by DEVSIC_ID';
		$sqlo->Quesel($sqlsel);
		while ( $sqlo->ReadRow() ) {
		    $tmpid = $sqlo->RowData[0];
		    $planids[]=$tmpid;
		}
		return $planids;
	}
	
	/**
	 * get entries of DEVSIC by TYPE
	 * @param $sqlo
	 * @param $adevid
	 * @param string $devsic_type
	 *   'W'
	 *   'V'
	 *   'Q'
	 * @return array $planids
	 */
	static function getDevSicList_byType(&$sqlo, $adevid, $devsic_type) {
	
		$sql_add = ' and DEVSIC_ID in (select DEVSIC_ID from DEVSIC where TYPE='.$sqlo->addQuotes($devsic_type).')';
	
		$planids=NULL;
		$sqlsel = "DEVSIC_ID from ADEV_HAS_SIC where A_CHIP_READER_ID=".$adevid. $sql_add. ' order by DEVSIC_ID';
		$sqlo->Quesel($sqlsel);
		while ( $sqlo->ReadRow() ) {
			$tmpid = $sqlo->RowData[0];
			$planids[]=$tmpid;
		}
		return $planids;
	}
}
