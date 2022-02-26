<?php
require_once ('o.H_ALOG_ACT.subs.inc');
/**
 * 
 * @package o.PUR.subs.inc
 * @swreq 
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @created 2021-10-26
 */
class oPUR_sua_subs {
    
    var $sua_id;
    private $full_cnt;
    const table='PUR';
    
    const STATE_CLOSED='closed';
    const STATE_ORDERED='ordered';
    
    const goArray = array(
        1=>'started',
        2=>'sent',
        3=>'ordered',
        4=>'closed',
    );
    
    function __construct (int $sua_id) {
        $this->sua_id=$sua_id;
        $this->full_cnt=0;
    }
    
    // get CNT of last QUERY action, e.g. get_last_objects()
    function get_full_cnt() {
        return $this->full_cnt;
    }
    
    function get_num_PURs($sqlo, $is_open_flag=0) {
        $sqlsel = "count(1) from PUR where ABSTRACT_SUBST_ID=".$this->sua_id;
        if ($is_open_flag>0) {
            $closed_id = oH_ALOG_ACT_subs::getH_ALOG_ACT_ID($sqlo, self::table, self::STATE_CLOSED);
            if ($closed_id) {
                $sqlsel = 'count(1) from PUR x join CCT_ACCESS a on x.CCT_ACCESS_ID=a.CCT_ACCESS_ID'.
                    ' where H_ALOG_ACT_ID!='.$closed_id. " and ABSTRACT_SUBST_ID=".$this->sua_id;
            }
        }
        $sqlo->Quesel($sqlsel);
        $sqlo->ReadRow();
        $cnt = $sqlo->RowData[0];
        return $cnt;
    }
    
    /**
     * get last X elements
     * 
     * @param object $sqlo
     * @param int $max_num
     * @return array
     */
    function get_last_objects($sqlo, int $max_num) {
        
        $this->full_cnt = $this->get_num_PURs($sqlo);
        
        $pur_arr=array();
        $sqlsel = "PUR_ID from PUR where ABSTRACT_SUBST_ID=".$this->sua_id. " order by PUR_ID DESC";
        $sqlo->Quesel($sqlsel);
        $cnt=0;
        while ( $sqlo->ReadRow() ) {
            if ($cnt>=$max_num) {
                break;
            }
            $tmpid = $sqlo->RowData[0];
            $pur_arr[]=$tmpid;
            $cnt++;
        }
        return $pur_arr;
    }
}