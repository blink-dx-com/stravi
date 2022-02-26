<?php

/**
 * analysis methods of CCT_TABLE_DYN
 * @package o.CCT_TABLE_DYN.subs.inc
 * @swreq SREQ:0002427: o.CCT_TABLE > common management 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 * @version $Header: trunk/src/phplib/o.CCT_TABLE_DYN.subs.inc 59 2018-11-21 09:04:09Z $
 */
class oCCT_TABLE_DYN_subs {
	
	/**
	 * get value by key of table
	 * @param $sqlo
	 * @param $table
	 */
	static function getValByKey( &$sqlo, $table, $key) {
		$sqlsel = 'VALUE from CCT_TABLE_DYN where TABLE_NAME='.
			$sqlo->addQuotes($table).' and KEY='.$sqlo->addQuotes($key);
		$sqlo->Quesel($sqlsel);
		$sqlo->ReadRow();
		$value = $sqlo->RowData[0];
		return $value;
	}
}
