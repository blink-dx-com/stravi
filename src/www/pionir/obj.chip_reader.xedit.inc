<?php

class o_CHIP_READER_editx extends gObj_editx_ABS {
    
    /**
     * update tab activity
     * @param object $sqlo
     */
    function tab_activity_up($sqlo) {
        
        $this->tab_data=array();
        
        // analyse protocol
        $sqlo->Quesel('count(1) from REA_LOG where CHIP_READER_ID='.$this->id.' ');
        $sqlo->ReadRow();
        $cnt = $sqlo->RowData[0];
        $this->tab_data['logs'] = $cnt;
        
        $sqlo->Quesel('count(1) from REA_PRO_ARCH where CHIP_READER_ID='.$this->id.' ');
        $sqlo->ReadRow();
        $cnt = $sqlo->RowData[0];
        $this->tab_data['arch'] = $cnt;
    }
}

function x_modes_get() {
    $xmodes = array( 'modes'=> array(
        "prot"   => array('n'=>"protocol"),
        "logs"   => array('n'=>"service log"),
        "arch"   => array('n'=>"proto archive"),
        "submod" => array('n'=>"used by"),
    ) );

	// $xmodes["qcprot"] = "QC protocol";
	// $xmodes["arch"] = "archive";
	return $xmodes;
}

