<?php
/**
 * subs for REA_PRO_ARCH
 * @swreq UREQ:FS-LIM08 Support device/instrument development
 * @package o.REA_PRO_ARCH.subs.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class oREA_PRO_ARCH_subs {
	/**
	 * get last version-id, flag, if protocol is committed
	 * @return 
	 * @param object $sqlo
	 * @param object $cpid
	 */
	function isActive() {
		if ( glob_table_exists('REA_PRO_ARCH') )  {
			return 1;
		} else return 0;
	}
	
	function getProtInfo(&$sqlo, $devid, $cpid) {
		$isOnArch=0;
		
		$sqlsel = 'CONCRETE_PROTO_ID, VERSID from REA_PRO_ARCH '.
			' where CHIP_READER_ID='.$devid.' and CONCRETE_PROTO_ID='.$cpid;
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$archCpid = $sqlo->RowData[0];
		$thisvers = $sqlo->RowData[1];
		if ($archCpid) {
			$isOnArch=1;
		} else {
			$isOnArch=0;
		}
		
		// last version
		$sqlsel = 'max(VERSID) from REA_PRO_ARCH where CHIP_READER_ID='.$devid;
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$lastVers = $sqlo->RowData[0];
		if (!$lastVers) $lastVers=0;
		return array('vers'=>$lastVers, 'isOnArch'=>$isOnArch, 'cpvers'=>$thisvers);
	}
	
	function getHistorySql($devid) {
		$sqlFrom = 'from REA_PRO_ARCH '.
			' where CHIP_READER_ID='.$devid.' order by VERSID desc';
		return ($sqlFrom);
	}
	
	function getLastProto(&$sqlo, $devid) {
		$sqlsel = 'max(CONCRETE_PROTO_ID) from REA_PRO_ARCH '.
			' where CHIP_READER_ID='.$devid;
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$protoID = $sqlo->RowData[0];
		return $protoID;
	}
}

