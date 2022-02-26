<?php

/**
 * BOM analysis methods of ABSTRACT_SUBST: SUA_BOM
 * @package o.ABSTRACT_SUBST.bom.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class oABSTRACT_SUBST_bom {
	
    function __construct($sua_id) {
        $this->sua_id = $sua_id;
    }
    
    // return: 0 or 1
    static function has_bom($sqlo, $sua_id) {
        $has_bom=0;
        $sqlsel = "COMPO_SUA_ID from SUA_BOM where ABSTRACT_SUBST_ID=" . $sua_id;
        $sqlo->Quesel($sqlsel);
        if ( $sqlo->ReadArray() ) {
            $has_bom=1;
        }
        return $has_bom;
    }
    
    /**
     * get array of array(COMPO_SUA_ID, POS, QUANTITY)
     * @param object $sqlo
     * @return NULL[]
     */
    function get_components(&$sqlo) {
        
        $sua_bom = array();
        $sqlsel = "COMPO_SUA_ID, POS, QUANTITY from SUA_BOM where ABSTRACT_SUBST_ID=" . $this->sua_id . ' order by POS';
        $sqlo->Quesel($sqlsel);
        while ( $sqlo->ReadArray() ) {
            $sua_bom[] = $sqlo->RowData;
        }
        
        return $sua_bom;
    }
    
    /**
     * return compos in the Form format
     * @param object $sqlo
     * @return array of compo_form_STRUCT 
     *   $pos => array(
     *   'sua'=>$row['COMPO_SUA_ID'], 
     *   'q'=>$row['QUANTITY'],
     *   'unit.name' => name of unit
     *   )
     */
    function get_compo_for_form(&$sqlo) {
        
        $sua_compos  = $this->get_components($sqlo);
        if (!sizeof($sua_compos)) return array();
        
        $sua_compo_pos=array();
        foreach($sua_compos as $row) {
            $pos = $row['POS'];
            
            $sua_compo_pos[$pos]=array('sua'=>$row['COMPO_SUA_ID'], 'q'=>$row['QUANTITY'] );
            if ($row['COMPO_SUA_ID']) {
                $H_UNIT_ID = glob_elementDataGet( $sqlo, 'ABSTRACT_SUBST',  'ABSTRACT_SUBST_ID', $row['COMPO_SUA_ID'], 'H_UNIT_ID');
                if ($H_UNIT_ID) {
                    $sua_compo_pos[$pos]['unit.id']  = $H_UNIT_ID;
                    $sua_compo_pos[$pos]['unit.name']= obj_nice_name ( $sqlo, 'H_UNIT', $H_UNIT_ID );
                }
            }
        }
        return $sua_compo_pos;
    }
    
    function get_compos_by_pos(&$sqlo) {
        
        $sua_bom = array();
        $sqlsel = "COMPO_SUA_ID, POS, QUANTITY from SUA_BOM where ABSTRACT_SUBST_ID=".$this->sua_id . 'order by POS';
        $sqlo->Quesel($sqlsel);
        while ( $sqlo->ReadArray() ) {
            $pos = $sqlo->RowData['POS'];
            $sua_bom[$pos] = $sqlo->RowData;
        }
        
        return $sua_bom;
    }
}