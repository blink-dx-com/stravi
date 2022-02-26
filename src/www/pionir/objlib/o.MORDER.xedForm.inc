<?php
require_once 'role.inc';


class oMORDER_xedForm  extends gui_xedForm_ABS {


    function x_cols_extra(&$sqlo, &$argu) {
        
        $colopt = array();
        $colopt['cols']=array();
        
        $is_producer = 0;
        $role_right_name = 'o.MORDER.producer';
        $role_right      = role_check_f($sqlo, $role_right_name);
        if ($role_right=="execute") {
            $is_producer = 1;
        }
        
        if (!$is_producer) {
            $colopt['cols']['START_DATE'] = array( 'EDITABLE'=>0 );
            $colopt['cols']['END_DATE']   = array( 'EDITABLE'=>0 );
        }
        
        if ( $argu['ABSTRACT_SUBST_ID'] ) {
            $sqlsel = "h.NAME from ABSTRACT_SUBST s join H_UNIT h on s.H_UNIT_ID=h.H_UNIT_ID".
                " where s.ABSTRACT_SUBST_ID=".$argu['ABSTRACT_SUBST_ID'];
            $sqlo->Quesel($sqlsel);
            $sqlo->ReadRow();
            $hunit_name = $sqlo->RowData[0];
            $colopt['cols']['QUANTITY']    = array( 'colNameAdd'=> ' [<b>'.$hunit_name.'</b>]' );
            $colopt['cols']['RETQUA']      = array( 'colNameAdd'=> ' [<b>'.$hunit_name.'</b>]' );
            $colopt['cols']['QUANTITY_QC'] = array( 'colNameAdd'=> ' [<b>'.$hunit_name.'</b>]' );
        }
       
        
        return $colopt;
    }
    
    /**
     * check input 
     */
    function paramCheck(&$sqlo, &$argu, &$extraobj_o, $action, $showCols ) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $tablename='MORDER';

        if ( $argu["END_DATE"] != NULL and $argu["START_DATE"] != NULL ) {

            if ( $argu["START_DATE"]  > $argu["END_DATE"] ) {
                $warn_flag=1; // just set a WARNING
                $error->set( $FUNCNAME, 1, '"'.columnname_nice2($tablename, 'START_DATE').
                    '" must be before "'.columnname_nice2($tablename, 'END_DATE').'".', $warn_flag );
                
            }
        }
    
    
    }
    

}
