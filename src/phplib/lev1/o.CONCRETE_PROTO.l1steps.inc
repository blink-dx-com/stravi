<?php
/**
 * generate protocol step output
 * $Header: trunk/src/phplib/lev1/o.CONCRETE_PROTO.l1steps.inc 59 2018-11-21 09:04:09Z $
 * @package o.CONCRETE_PROTO.l1steps
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */

require_once('o.PROTO.steps.inc');

class oCONCRETE_PROTO_l1steps {
	
function __construct() {
	$this->protoObj = new gProtoOrg( );
}
	
/**
 * get protocol steps
 * @return step-array
 */
function getSteps(&$sqlo, $cpid, $a_protoid) {
	
    $this->protoObj->setProto( $sqlo, $a_protoid, $cpid );
    $step_arrayX = $this->protoObj->getStepArray();
    if (!sizeof($step_arrayX)) {
        return;
    }
    
    $retarr = NULL;
    foreach( $step_arrayX as $dummy=>$step_nr) {
        $tmparr = $this->protoObj->cStepSumGet( $sqlo, $step_nr, $cpid );
        $retarr[] = array(
        		"stepnr" => $step_nr, 
                "name"   => $tmparr["name"] , 
                "subst"  => $tmparr["substn"] , 
                "quant"  => $tmparr["quant"], 
                "note"   => $tmparr["note"] 
        	);
    } 
    reset($step_arrayX);
    return ($retarr);
}
}
