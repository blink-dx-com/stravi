<?php
function x_modes_get() {
    $xmodes = array( 
        'default'=> 'labview',
        'modes'  => array(
        "features"=> array('n'=>"features", 'sh'=>false),
        "labview" => array('n'=>"Editor"),
    ) );
    
	return $xmodes;
}
