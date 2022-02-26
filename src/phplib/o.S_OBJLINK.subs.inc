<?php

/**
 * S_OBJLINK analysis functions
 * @package o.S_OBJLINK.subs.inc
 * @swreq SREQ:0001012: o.S_OBJLINK > sub functions to analyse this table 
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com) 
 */
class oS_OBJLINK_subs {

/**
 * get ONE mother-object-id by (child table, id, key)
 * STATIC
 * @return array($moTable,$moID)
 */
static function getMoObjbyKey(&$sqlo, $childTable, $childId, $key) {
	if (!$childId) return array(NULL,0);
	
	$sqlsel = "MO_TABLE, MO_ID from S_OBJLINK where CH_TABLE=".$sqlo->addquotes($childTable).
		' and CH_ID='.$childId. ' and KEY='.$sqlo->addquotes($key);
	$sqlo->Quesel($sqlsel);
	$sqlo->ReadRow();
	$moTable = $sqlo->RowData[0];
	$moID    = $sqlo->RowData[1];
	return array($moTable,$moID);
}

/**
 * get MANY mother-object-id by (child table, child id, mother tab, key)
 * STATIC
 * @return array($moID)
 */
static function getMoObjsbyTabKey(&$sqlo, $childTable, $childId, $motherTable, $key) {
	
	if (!$childId) return array();
	
	$moArr = array();
	$sqlsel = "MO_ID from S_OBJLINK where CH_TABLE=".$sqlo->addquotes($childTable).
	' and CH_ID='.$childId. ' and MO_TABLE='.$sqlo->addquotes($motherTable).' and KEY='.$sqlo->addquotes($key).' order by MO_ID';
	$sqlo->Quesel($sqlsel);
	while ($sqlo->ReadRow() ) {
		$moArr[] = $sqlo->RowData[0];
	}
	return $moArr;
}

/**
 * get ALL distince KEYS of MOTHER
 * @param object $sqlo
 * @param string $motherTable
 * @param int $motherID
 * @return array of KEYs
 */
static function ALLMoObjKeys(&$sqlo, string $motherTable, int $motherID) {
    $sqlsel = "distinct(KEY) from S_OBJLINK where ".
        ' MO_ID='.$motherID. ' and MO_TABLE= '.$sqlo->addquotes($motherTable).' order by KEY';
    $sqlo->Quesel($sqlsel);
    $key_arr=array();
    while ($sqlo->ReadRow()) {
        $key    = $sqlo->RowData[0];
        $key_arr[] = $key;
    }
    return $key_arr;
    
}

/**
 * get child-objects by (mother table, mother id, $childTable)
 * @param $motherTable
 * @param $motherID
 * @param $childTable
 * @return array of children-ID
 */
static function getChiObjByChiTab(&$sqlo, $motherTable, $motherID, $childTable) {
	
	if (!$motherID) return NULL;
	
	$sqlsel = "CH_ID from S_OBJLINK where CH_TABLE=".$sqlo->addquotes($childTable).
		' and MO_ID='.$motherID. ' and MO_TABLE= '.$sqlo->addquotes($motherTable).' order by CH_ID';
	$sqlo->Quesel($sqlsel);
	$childArr=NULL;
	while ($sqlo->ReadRow()) {
		$childID    = $sqlo->RowData[0];
		$childArr[] = $childID;
	}
	return $childArr;

}

/**
 * get child-objects by (mother table, mother id, $childTable or $key)
 * @param $motherTable
 * @param $motherID
 * @param $childTable
 * @param $key
 * @return array of children-ID
 */
static function getChiObjByTabOrKey(&$sqlo, $motherTable, $motherID, $childTable=NULL, $key=NULL) {
	global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
	
	if (!$motherID) return NULL;

	if ($childTable!=NULL and $key==NULL) return self::getChiObjByChiTab($sqlo, $motherTable, $motherID, $childTable);
	
	if ($childTable==NULL and $key!=NULL) {
		$sqlsel = "CH_ID from S_OBJLINK where KEY=".$sqlo->addquotes($key).
		' and MO_ID='.$motherID. ' and MO_TABLE= '.$sqlo->addquotes($motherTable).' order by CH_ID';
		$sqlo->Quesel($sqlsel);
		$childArr=NULL;
		while ($sqlo->ReadRow()) {
			$childID    = $sqlo->RowData[0];
			$childArr[] = $childID;
		}
		return $childArr;
	}
	
	if ($childTable!=NULL and $key!=NULL) {
		$sqlsel = "CH_ID from S_OBJLINK where CH_TABLE=".$sqlo->addquotes($childTable).
		' and KEY='.$sqlo->addquotes($key).' and MO_ID='.$motherID. ' and MO_TABLE= '.$sqlo->addquotes($motherTable).' order by CH_ID';
		$sqlo->Quesel($sqlsel);
		$childArr=NULL;
		while ($sqlo->ReadRow()) {
			$childID    = $sqlo->RowData[0];
			$childArr[] = $childID;
		}
		return $childArr;
	}
	
	$error->set($FUNCNAME, 1, 'At least one of the following attributes is needed: "childTable" , "key" ');
	return;
}

/**
 * get child-objects by (mother table, mother id, $key)
 * @param $motherTable
 * @param $motherID
 * @param $key
 * @return array of (CHI-table, children-ID)
 */
static function getChiObjByKey(&$sqlo, $motherTable, $motherID, $key) {
	//global $error;
	// $FUNCNAME = 'getChiObjByKey';

	if (!$motherID) return NULL;
	if (!$key) return NULL;

	
	$sqlsel = "CH_TABLE, CH_ID from S_OBJLINK where KEY=".$sqlo->addquotes($key).
	' and MO_ID='.$motherID. ' and MO_TABLE= '.$sqlo->addquotes($motherTable).' order by CH_ID';
	$sqlo->Quesel($sqlsel);
	$childArr=NULL;
	while ($sqlo->ReadRow()) {
		$childTAB   = $sqlo->RowData[0];
		$childID    = $sqlo->RowData[1];
		$childArr[] = array($childTAB, $childID);
	}
	return $childArr;

}

/**
 * child-objects already linked ? (mother table, mother id, $childTable, $childID)
 * @param $motherTable
 * @param $motherID
 * @param $childTable
 * @param $childID
 * @return $pos
 */
static function chiObjLinked(&$sqlo, $motherTable, $motherID, $childTable, $childID) {
	
	if (!$motherID) return NULL;
	
	$sqlsel = "POS from S_OBJLINK where CH_TABLE=".$sqlo->addquotes($childTable).
		' and MO_ID='.$motherID. ' and MO_TABLE= '.$sqlo->addquotes($motherTable).' and CH_ID='.$childID;
	$sqlo->Quesel($sqlsel);
	$sqlo->ReadRow();
	$pos  = $sqlo->RowData[0];
	return $pos;
}

/**
 * get child-object by (mother table, mother id, $pos)
 * @param $motherTable
 * @param $motherID
 * @param $pos
 * @return array(table,children-ID)
 */
static function getChiObjByPos(&$sqlo, $motherTable, $motherID, $pos) {
	$sqlsel = "CH_TABLE, CH_ID from S_OBJLINK where ".
		' MO_ID='.$motherID. ' and MO_TABLE= '.$sqlo->addquotes($motherTable).
		' and POS='.$pos;
	$sqlo->Quesel($sqlsel);
	$childArr=NULL;
	if ( $sqlo->ReadRow() ) {
		$childTable = $sqlo->RowData[0];
		$childID    = $sqlo->RowData[1];
		$childArr = array($childTable,$childID);
	}
	return $childArr;

}

/**
 * get the SQL-string of this parameters ($motherTable, $childTable, $childID, KEY (OPTIONAL) )
 */
static function sql_getMobyChiTab(&$sqlo, $motherTable, $childTable, $childID, $key=NULL) {
	$sqlsel = "MO_ID from S_OBJLINK where CH_TABLE=".$sqlo->addquotes($childTable).' and CH_ID='.$childID.
		' and MO_TABLE='.$sqlo->addquotes($motherTable);
	if ($key) {
	    $sqlsel .= ' and KEY='.$sqlo->addquotes($key);
	}
	return $sqlsel;
}

}
