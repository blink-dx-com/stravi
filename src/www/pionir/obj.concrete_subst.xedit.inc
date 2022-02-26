<?php

class o_CONCRETE_SUBST_editx extends gObj_editx_ABS {
    
    /**
     * update tab activity
     * @param object $sqlo
     */
    function tab_activity_up($sqlo) {
        
        $this->tab_data=array();

        // analyse protocol
        
        $sqlo->Quesel('count(1) from CS_HAS_PR where CONCRETE_SUBST_ID='.$this->id.' and TYPE=1');
        $sqlo->ReadRow();
        $cnt = $sqlo->RowData[0];
        $this->tab_data['prot'] = $cnt;
        
        $sqlo->Quesel('count(1) from CS_HAS_PR where CONCRETE_SUBST_ID='.$this->id.' and TYPE=2');
        $sqlo->ReadRow();
        $cnt = $sqlo->RowData[0];
        $this->tab_data['qcprot'] = $cnt;
        
        // BOM
        $sqlo->Quesel('count(1) from SUC_BOM where CONCRETE_SUBST_ID='.$this->id);
        $sqlo->ReadRow();
        $cnt = $sqlo->RowData[0];
        $this->tab_data['bom'] = $cnt;
  
    }
    
    function tab_activity_dyn($sqlo) {
        $sqlo->Quesel('count(1) from CONT_HAS_CSUBST where CONCRETE_SUBST_ID='.$this->id);
        $sqlo->ReadRow();
        $cnt = $sqlo->RowData[0];
        $this->tab_data['stor'] = $cnt;
    }
}

function x_modes_get() {
    $xmodes = array( 'modes'=> array(
        "abs"    =>array('n'=> "MAA"),
        "prot"   =>array('n'=> "Prep protocol"),
        "qcprot" =>array('n'=> "QC protocol"),
        "stor"   =>array('n'=> "storage"),
        "bom"    =>array('n'=> "BOM"),
        "qc"     =>array('n'=> "QM"),
        ) );
	
	return $xmodes;
}


