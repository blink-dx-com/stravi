<?php
require_once $_SESSION['s_sessVars']['AppLabLibDir'].'/lablib/gui/o.AS_ASR.c_form.inc';

/**

 */
class obj_edit_ext extends gObj_edit_feat_ABS {
    
    function rightWindow( &$sqlo, &$sqlo2) {
        $id = $this->id;
        global $error;
        echo "<ul>\n";

        $extra_title_text = '';
        //$extra_title_text  .= ' &nbsp;&nbsp; [<a href="p.php?mod=LAB/o.AS_ASR.imp&id='.$id.'">Import from CSV file</a>]';
        $extra_title_text .= ' &nbsp;&nbsp; [<a href="p.php?mod=LAB/o.AS_ASR.exp&id='.$id.'">Export as JSON</a>]';
        $extra_title_text .= ' &nbsp;&nbsp;  [<a href="http://intranet/dokuwiki/product/assay/bsd" target="_new">Help BeadLibDataSpec</a>]';
        
        $extra_title_text .= "<br><br>\n";
        
        echo $extra_title_text. "\n";
        
        $parx=array();
        
        $mainlib = new oAS_ASR_c_form($sqlo, $id, $parx, 0);
        $mainlib->show($sqlo);
        $error->printAll();
        
        
//         $asr_form_lib = new oAS_ASR_form($id);
//         $asr_form_lib->show_all($sqlo);
//         echo "<br>\n";
//         $asr_form_lib->help();
        
        echo "</ul>\n";
    }

}


