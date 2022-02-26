<?php
/**
 * analysis functions for EXTRA_ATTRIB
 * @package o.EXTRA_ATTRIB.subs.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class oEXTRA_ATTRIB_subs {

/**
 * get prefered values (+ notes) from EXTRA_PREF_VAL, order by VALUE
 * @return array(CNT => array (VAL, NOTES))
 */
function getPrefValNoteByAtt(&$sqlo, $EXTRA_ATTRIB_ID ) {
	if (!$EXTRA_ATTRIB_ID ) {
		return;
	}
	$MAXNUM = 50;
	$values = array();
	$cnt    = 0;

	$sqlo->Quesel('value, notes FROM EXTRA_PREF_VAL '.
		' WHERE EXTRA_ATTRIB_ID = '.$EXTRA_ATTRIB_ID.' ORDER BY value');
	while ($sqlo->ReadRow()) {
		if ($cnt >= $MAXNUM ) {
			$values[$cnt] = array('','...');
			break;
		}
		$values[$cnt] = array($sqlo->RowData[0], $sqlo->RowData[1]);
		$cnt++;
	}
	
	return $values;
}  

/**
 * select prefered values (+ notes) from EXTRA_PREF_VAL as <html> <select> array
 * @return array(VAL => "VAL --- NOTES")
 */
function getPrefSelArrByAtt(&$sqlo, $EXTRA_ATTRIB_ID ) {
	if (!$EXTRA_ATTRIB_ID ) {
		return;
	}
	$valNoteArr = $this->getPrefValNoteByAtt($sqlo, $EXTRA_ATTRIB_ID );
	
	$selectArr = array();
	foreach( $valNoteArr as $cnt=>$keyNoteArr) {
		$tmpnote=NULL;
		if ($keyNoteArr[1]!="")  $tmpnote = " --- ".$keyNoteArr[1];
		$val = $keyNoteArr[0] . $tmpnote;
		$selectArr[$keyNoteArr[0]]=$val;
	}
	reset ($valNoteArr); 
	
	return $selectArr;
}  

}

