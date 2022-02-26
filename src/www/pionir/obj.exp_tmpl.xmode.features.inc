<?php
require_once 'o.EXP_TMPL.subs.inc';


class obj_edit_ext extends gObj_edit_feat_ABS {
    
    function rightWindow( &$sqlo, &$sqlo2) {
        $id = $this->id;
        

        echo "<ul>\n";
        
        if ( file_exists($_SESSION['s_sessVars']['AppLabLibDir'].'/lablib/gui/o.AS_ASR.view.inc')) {
       
            require_once $_SESSION['s_sessVars']['AppLabLibDir'].'/lablib/gui/o.AS_ASR.view.inc';
            
            // get ASR
            $exp_tmpl_lib = new oEXP_TMPL_subs($id);
            $asr_id = $exp_tmpl_lib->get_ASR_lib($sqlo);
            
            echo "Related ASR_lib: ";
            if ($asr_id) {
                
                $objLinkLib = new fObjViewC();
                $asr_html = $objLinkLib->bo_display( $sqlo, 'AS_ASR', $asr_id );
                
                echo $asr_html."<br>";
                echo "<br>\n";
                $asr_form_lib = new oAS_ASR_form($asr_id);
                $asr_form_lib->show_all($sqlo);
            } else {
                echo ' none<br>';
            }
        }
        echo "</ul>\n";
    }

}
