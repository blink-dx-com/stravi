<?php
// @module o.JOUR_ENTRY.gui2.inc
class oJOUR_ENTRY_gui2 {
    
    static function show_list1($sqlo, &$jou_obj_arr) {
        
        $NOTES_MAX_LEN=80;
        $tab_jou='JOUR_ENTRY';
        foreach ($jou_obj_arr as $jou_id) {
            
            $jou_feats = glob_ObjDataGet($sqlo, $tab_jou, $jou_id, array('NOTES') ) ;
            echo  fObjViewC::bo_display( $sqlo, $tab_jou, $jou_id);
            if ($jou_feats['NOTES']!=NULL) {
                $notes = htmlspecialchars( substr($jou_feats['NOTES'], 0, $NOTES_MAX_LEN) .' ...');
                echo ' &nbsp;&nbsp;&nbsp;<span class="yGgray">Content:</span> '.$notes;
            }
            echo '<br>'."\n";
        }
    }
}