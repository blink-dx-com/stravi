<?php
require_once('gui/glob.xfunc_abs.inc');

/**
 * @package obj.bcbatch.xfunc.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class cBCBATCH extends xfunc_ABS {
var $ident="BCBATCH";

function __construct( ) {
    parent::__construct();
    $this->xcolarr[]=array ("CNT(carts)" => "CNT(cartridges)", "rem" => "count of cartridges");
}

/**
 * show following attributes in project
 * @param unknown $sql
 * @param unknown $prim_id
 * @param unknown $options
 * @return string
 */
function inProjShowAtt(&$sql, $prim_id, $options=NULL ) {
    $result = $this->get_abs_object_name($sql, $prim_id);
    return $result;
}

function xColDataGet (&$sqlo, $colname, $idarr ) {
    //   intern Code-EXAMPLE:
    if ($colname=="CNT(carts)") {
        $sqlsel = "count(1) from BCBATCH_H_SOC where BCBATCH_ID=". $idarr[0];
        $sqlo->Quesel($sqlsel);
        $sqlo->ReadRow();
        $output = $sqlo->RowData[0];
    }
    return ( $output );
    
}

}

