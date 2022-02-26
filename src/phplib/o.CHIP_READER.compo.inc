<?php

/**
 * CHIP_READER: component analysis
 * @package o.CHIP_READER.compo.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class oCHIP_READER_compo {

    function __construct($dev_id) {
        $this->dev_id=$dev_id;
    }

	/**
	 * get all CHIP_READER where this is a component
	 * @param object $sqlo
	 * @return array of CHIP_READER_IDs
	 */
	function used_by_devs($sqlo) {
	    if (!$this->dev_id) {
	        throw new Exception('Input: dev_id missing.');
	    }
	    $MAX_CNT=200;
	    $i=0;
	    $dev_arr=array();
	    $sqls1 = "select CONCRETE_PROTO_ID from CONCRETE_PROTO_STEP where DEV_ID=".$this->dev_id;
	    $sqls2 = "CHIP_READER_ID from CHIP_READER where CONCRETE_PROTO_ID in (".$sqls1.") order by CHIP_READER_ID";
	    $sqlo->Quesel($sqls2);
	    while ( $sqlo->ReadRow() ) {
	        if ($i>$MAX_CNT) {
	            throw new Exception('Too many results; Max accepted: '.$MAX_CNT);
	        }
	        $dev_id_loop = $sqlo->RowData[0];
	        $dev_arr[] = $dev_id_loop;
	        $i++;
	    }
	    return $dev_arr;
	}
}
