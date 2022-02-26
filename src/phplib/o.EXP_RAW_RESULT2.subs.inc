<?php

/**
 * o.EXP_RAW_RESULT2 sub functions
 * @package o.EXP_RAW_RESULT2.subs.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class oEXP_RAW_RESULT2_subs {
    
    
    function get_column_map() {
        
        $db_col_map = array(
            'VAL01'=> array('nice'=>'contrast'),
            'VAL02'=> array('nice'=>'median'),
            'VAL03'=> array('nice'=>'mean'),
            'VAL04'=> array('nice'=>'sd'),
            'VAL05'=> array('nice'=>'width'),
            'VAL06'=> array('nice'=>'mean_hist'),
            'VAL07'=> array('nice'=>'bg_median'),
            'SPOT_ID'=> array('nice'=>'id', ),
            'CHAN_ID'=> array('nice'=>'img_nr'),
            
        );
        
        return $db_col_map;
    }
}