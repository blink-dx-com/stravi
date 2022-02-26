<?php 
require_once ("visufuncs.inc");
require_once ("f.objview.inc");	

/**
 * BOM of SUC
 * @author Steffen
 * @package o.CONCRETE_SUBST.bom.inc
 *
 */
class oCONCRETE_SUBST_bom {
    
    function __construct($suc_id) {
        $this->suc_id = $suc_id;
    }
    
    
    /**
     * get order substances
     * @param $sqlo
     * @param $sqlo2
     * 
     */
    function getSubsts(&$sqlo) {
        
        $suc_id = $this->suc_id;

        $sqlsel = '*  from SUC_BOM where CONCRETE_SUBST_ID	='.$suc_id.' order by POS';
        
        $sqlo->Quesel($sqlsel);
        $cSubstArr = array();
        while ( $sqlo->ReadArray() ) {
            $cSubstArr[] = $sqlo->RowData;
        }
       
        return $cSubstArr;
    }
    
    /**
     * 
     * @param object $sqlo
     * @return array
     *   [POS] => array(COMPO_SUC_ID, POS, ...)
     */
    function get_compos_by_pos(&$sqlo) {
        
        $suc_id = $this->suc_id;
        $sqlsel = '*  from SUC_BOM where CONCRETE_SUBST_ID	='.$suc_id.' order by POS';
        
        $sqlo->Quesel($sqlsel);
        $cSubstArr = array();
        while ( $sqlo->ReadArray() ) {
            $cSubstArr[$sqlo->RowData['POS']] = $sqlo->RowData;
        }
        
        return $cSubstArr;
    }
    
    /**
     * 
     * @param array $cSubstArr 
     *   [POS] => array(COMPO_SUC_ID, POS, ...)
     * @param array $aSubstArr
     *   array(COMPO_SUA_ID, POS, QUANTITY)
     *   @return array = array (
     *     'POS',
     *     'APOS',
     *     'a':
     *     'c':
     */
    function merge_conc_and_abs(&$cSubstArr, &$aSubstArr) {
        
        $posarr = array();
        
        $pos_found_arr=array();
        foreach( $cSubstArr as $pos=>$dummy) {
            $pos_found_arr[$pos]=0;
        }
       
        
        if (!empty($aSubstArr)) {
            foreach($aSubstArr as $a_row) {
                
                $one_row_a = array('APOS'=>$a_row['POS'], 'a'=>$a_row);
                
                $found_c=0;
                if (!empty($cSubstArr)) {
                    
                    foreach($cSubstArr as $c_row) {
                        if ($c_row['APOS']==$a_row['POS']) {
                            $one_row_c = array_merge($one_row_a, array('POS'=>$c_row['POS'], 'c'=>$c_row ));
                            $found_c=1;
                            $posarr[] = $one_row_c;
                            $pos_found_arr[$c_row['POS']] = 1;
                        }
                    }
                }
                
                if (!$found_c) {
                    $posarr[] = $one_row_a;
                }
                
            }
        }
        
       
        // analyse $pos_found_arr
        foreach( $pos_found_arr as $pos=>$flag) {
            if (!$flag) {
                $c_row = $cSubstArr[$pos];
                $posarr[] = array('APOS'=>$c_row['APOS'], 'POS'=>$c_row['POS'], 'c'=>$c_row );
            }
        }
        
        
        return $posarr;
        
    }
}