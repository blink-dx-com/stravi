<?php

class o_ABSTRACT_SUBST_editx extends gObj_editx_ABS {
    
    /**
     * update tab activity
     * @param object $sqlo
     */
    function tab_activity_up($sqlo) {
        
        $this->tab_data=array();
        
        // analyse protocol
        $sqlo->Quesel('count(1) from AS_HAS_PR where ABSTRACT_SUBST_ID='.$this->id.' ');
        $sqlo->ReadRow();
        $cnt = $sqlo->RowData[0];
        $this->tab_data['prot'] = $cnt;
        
        // BOM
        $sqlo->Quesel('count(1) from SUA_BOM where ABSTRACT_SUBST_ID='.$this->id.' ');
        $sqlo->ReadRow();
        $cnt = $sqlo->RowData[0];
        $this->tab_data['bom'] = $cnt;
        
        // MSDS
        $val = oS_VARIO_sub::getValByTabKey($sqlo, 'ABSTRACT_SUBST', $this->id, 'MSDS');
        if ($val!=NULL) {
            $this->tab_data['xlab_msds'] = 1;
        }
    }
    
    function tab_activity_dyn($sqlo) {
        $sqlo->Quesel('count(1) from CONCRETE_SUBST where ABSTRACT_SUBST_ID='.$this->id);
        $sqlo->ReadRow();
        $cnt = $sqlo->RowData[0];
        $this->tab_data['xlablog'] = $cnt;
    }
}

function x_modes_get() {
    // $xmodes["chem"] = "chem";
    $xmodes = array( 'modes'=> array(
        "prot" => array('n'=>"protocols"),
        "bom"  => array('n'=>"BOM"),
        "vers" => array('n'=>"Version"),
    ) );
	return $xmodes;
}
