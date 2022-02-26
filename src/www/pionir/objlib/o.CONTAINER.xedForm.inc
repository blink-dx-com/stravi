<?php


class oCONTAINER_xedForm  extends gui_xedForm_ABS {
    /**
     * get options for columns
     * @param $sqlo
     * @param $arguByKey
     * @return array $colopt
     */
    function x_cols_extra( &$sqlo, &$arguByKey ) {
        $assoc=array();
        $assoc['CONT_HAS_CSUBST'] = array('tab'=>'CONT_HAS_SUB_VIEW', 'txt'=>'with alias');
        $assoc['CONT_LOG']        = array('url'=>'p.php?mod=DEF/o.CONTAINER.entrylog&id='.$arguByKey['CONTAINER_ID'], 'txt'=>'show log');
        $colopt = array('assoc'=> $assoc);
        return($colopt);
    }

    function getColFeatures(&$sqlo, &$arguByCol, &$extraobj_o, $showCols) {
        $colInfoArr=array();
        if($arguByCol['CHIP_READER_ID']) {
            $colInfoArr['LOCATION']=array();
            $colInfoArr['LOCATION']['EDITABLE'] = -1;
            $colInfoArr['LOCATION']['COMMENTS'] = '(managed by the equipment)';
        }
        return $colInfoArr;
    }
    
    function paramCheck(&$sqlo, &$argu, &$extraobj_o, $action, $showCols ) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;

        // keep LOCATION in sync with CHIP_READER_ID.LOCATION
        if ( $argu["CHIP_READER_ID"] != NULL  and glob_column_exists('CONTAINER', 'LOCATION') ) {
            $dev_loc = glob_elementDataGet( $sqlo, 'CHIP_READER', 'CHIP_READER_ID', $argu["CHIP_READER_ID"], 'LOCATION');
            if ($dev_loc!=$argu['LOCATION']) {
                $argu['LOCATION']=$dev_loc;
            }
        } else {
            if ( $argu["LOCATION"] != NULL ) {
                // do the check
                $allow_pattern1 = '/^(M [0-9]\.[0-9].*)|(EXTERN:.*)/';
                $result1 = preg_match ( $allow_pattern1 , $argu["LOCATION"] );
                
                $allow_pattern2 = '/^([ABC] [0-4]\.[0-9][0-9].*)|(EXTERN:.*)/';
                $result2 = preg_match ( $allow_pattern2 , $argu["LOCATION"] );
                
                if (!$result1 and !$result2) {
                    $error->set( $FUNCNAME, 1, 'location contains bad pattern! example: '.
                        '"B 1.02 (S3)" or "M 2.433 (S3)" or "EXTERN: uni_Honolu"');
                }
            }
        }
    }

}
