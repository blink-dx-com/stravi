<?php

/**
 * module: obj.TABLENAME.xmode.features.inc
 * extend an the OBJECT "feature area" for edit.tmpl.php
 */
class obj_edit_ext extends gObj_edit_feat_ABS {
    
    function rightWindow( &$sqlo, &$sqlo2) {
        $id       = $this->id;
        $argu     = &$this->argu;
        $o_rights = &$this->o_rights;
        $viewmode = $this->viewmode;
        $colNames_ori = &$this->colNames_ori;
        $extraobj_o   = &$this->extraobj_o; 
        
        // your code
    }
    
}

/**
 * EXAMPLE for ASSOC-extension
 * show samples : Dealing with 'IS_SAMPLE'-flag:
 * @param object $sql
 * @param int $id
 * @return array
 */
function x_assoc_show( &$sql, $id ) {
    $dataarr=array();
    
    $samplesIDs = oEXP_subs::getSamples($sql, $id);
    if ( sizeof($samplesIDs) ) {
        $colname  = "sample";
        $icon     = "images/icon.SAMPLE.gif";
        $tmpval   = "";
        $samples  = oEXP_subs::getSampleNamesOfArr($sql, $samplesIDs);
        foreach($samples as $sample_id => $sample_name) {
            if ( empty($sample_name) ) $sample_name = "sample without name";
            $tmpval .= "<a class=xl href=\"edit.tmpl.php?t=CONCRETE_SUBST&id=$sample_id\"><B>$sample_name</B></a><br>\n";
        }
        $dataarr[0] = array( $icon, $colname, $tmpval, "samples of experiment" );
    }
    return ($dataarr);
}