<?php
require_once('gui/glob.xfunc_abs.inc');

/**
 * @package obj.socket.xfunc.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class cSOCKET extends xfunc_ABS {
var $ident="SOCKET";

function __construct( ) {
    parent::__construct();
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

}

