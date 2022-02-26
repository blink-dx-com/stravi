<?php
class o_EXP_editx extends gObj_editx_ABS {
    
    /**
     * update tab activity
     * @param object $sqlo
     */
    function tab_activity_up($sqlo) {
        
        $this->tab_data=array();
        
        // analyse protocol
        $sqlo->Quesel('count(1) from EXP_HAS_PROTO where EXP_ID='.$this->id.' ');
        $sqlo->ReadRow();
        $cnt = $sqlo->RowData[0];
        $this->tab_data['protocols'] = $cnt;
        
        if (glob_column_exists('EXP','MO_EXP_ID') ) {
            // wellplate sub experiments
            $sqlo->Quesel('count(1) from EXP where MO_EXP_ID='.$this->id.' ');
            $sqlo->ReadRow();
            $cnt = $sqlo->RowData[0];
            if ( $cnt )  $this->tab_data['chi'] = 1;
        }
    }
}

function x_modes_get() {
    $xmodes = array( 'modes'=> array(
        "protocols" => array('n'=>"protocol log"),
    ) );
   
	return $xmodes;
}



