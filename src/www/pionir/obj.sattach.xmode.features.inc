<?php
/**
 * show related BO
 */

require_once ("f.objview.inc");	

class obj_edit_ext extends gObj_edit_feat_ABS {
    
    function rightWindow( &$sqlo, &$sqlo2) {
 
        $argu  = &$this->argu;
        $TABLE_NAME = $argu["TABLE_NAME"];
        $OBJ_ID     = $argu["OBJ_ID"];
        
        echo "<ul>\n";
        
        $ret = fObjViewC::bo_display( $sqlo, $TABLE_NAME, $OBJ_ID );
        echo " - ".$ret."<br>";
        
        echo "</ul>";
        return 0;
    }

}
