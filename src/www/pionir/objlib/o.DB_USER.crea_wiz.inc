<?php
/**
 * extend for Crea_WIZ
 * @package o.DB_USER.crea_wiz.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */

require_once 'lev1/glob.obj.crea_wiz_objabs.inc';

class oDB_USER_objCreaWiz_G extends objCreaWiz_OBJ_gui {
    
    function form0_extra_rows(&$sqlo, &$htmlXObj) {
        $row_cnt=0;
        if ( column_remark2( "DB_USER", "FULL_NAME") !="" ) {
            $htmlXObj->htmlRow("full name", "<input name=\"newparams[FULL_NAME]\" value=\"\" size=40>", "the full name");
            $row_cnt++;
        }
        $htmlXObj->htmlRow("email", "<input name=\"newparams[EMAIL]\" value=\"\" size=40>", "the email-address");
        $row_cnt++;
        
        return $row_cnt;
    }
    
}


class oDB_USER_objCreaWiz extends objCreaWiz_OBJ {
}
