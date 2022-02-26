<?php
/**
 * @module: obj.TABLENAME.xedit.inc
 * - get the xmodes for this TABLE
 * - manage tab activity, called by  getXmodes()  
 */
function x_modes_get() {
    $xmodes = array( 'modes'=> array(
        "steps" => array('n'=>"steps"),
        "qc"    => array('n'=>"QC"),
      
    ) );
	return $xmodes;
}