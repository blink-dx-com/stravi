<?php
require_once ("f.objview.inc");	

class obj_edit_ext extends gObj_edit_feat_ABS {
    
    function rightWindow( &$sqlo, &$sqlo2) {
        $argu  = &$this->argu;

    	// get edit-link of BO + NAME + icon (object)
    	$objLinkLib = new fObjViewC();
    	
    	$mo_tab=$argu['MO_TABLE'];
    	$mo_id=$argu['MO_ID'];
    	$html1 = $objLinkLib->bo_display( $sqlo, $mo_tab, $mo_id ); 
    	echo 'Mother: '.$html1.'<br>'."\n";
    	
    	$ch_tab=$argu['CH_TABLE'];
    	$ch_id =$argu['CH_ID'];
    	$html1 = $objLinkLib->bo_display( $sqlo, $ch_tab, $ch_id ); 
    	echo 'Child: '.$html1.'<br>'."\n";
    	
    	return 0;
    }

}
