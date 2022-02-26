<?php
require_once 'o.S_OBJLINK.subs.inc';
require_once 'o.S_VARIO.subs.inc';
/**
 * 
 * production line
 * @package o.CHIP_READER.prodline.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class o_CHIP_READER_prodline {
    const SOBJ_KEY ='o.MORDER.stdWorkflow';
    const VARIO_KEY='is.prodline';
    
    function __construct($sqlo, $id) {
        $this->id=$id;
    }
    
    // 0,1
    function is_prod_line($sqlo) {
        return oS_VARIO_sub::getValByTabKey($sqlo, 'CHIP_READER',  $this->id, self::VARIO_KEY);
    }
    
    function get_def_MORDER_wfl($sqlo) {
        $order_ids = oS_OBJLINK_subs::getChiObjByTabOrKey($sqlo, 'CHIP_READER', $this->id, 'MODULE', self::SOBJ_KEY);
        if (empty($order_ids)) return NULL;
        return current($order_ids);
    }
    
    /**
     * get ALL prodlines as array
     * @param object $sqlo
     * @throws Exception
     * @return array of IDs
     */
    static function get_all_prodlines(object $sqlo) {
        $MAX_NUM=300;
        
        $dev_arr=array();
        $variolib = new oS_VARIO_sub('CHIP_READER');
        $subsel = $variolib->getSqlSelOfKey($sqlo, self::VARIO_KEY);
        
        $cnt=0;
        $sqlsel = "CHIP_READER_ID from CHIP_READER where CHIP_READER_ID in (".$subsel.") order by NAME";
        $sqlo->Quesel($sqlsel);
        while ( $sqlo->ReadRow() ) {
            if($cnt>$MAX_NUM) {
                throw new Exception('Too many entries.');
            }
            $tmpid = $sqlo->RowData[0];
            $dev_arr[]=$tmpid;
            $cnt++;
        }
        return $dev_arr;
    }
}