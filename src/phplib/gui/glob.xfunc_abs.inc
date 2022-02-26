<?php
require_once('glob.obj.conabs.inc');

/**
 * BASE class for files o.{TABLE}.xfunc.inc
 * @author Steffen Kube
 * @package glob.xfunc_abs.inc
 */
class xfunc_ABS {
    
    function __construct() {
        $concabs_lib = new gConcAbsC();
        $this->abs_table = $concabs_lib->getTemplate($this->ident);
    }
    
   
    
    /*
     * show attributes in form
     * function inFormShowAtt(&$sql, $prim_id ) {}
     * 
     * show attributes in PROJECT navigator
     * function inProjShowAtt(&$sqlo, $prim_id, $options=NULL ) { }
     */
    function get_abs_object_name($sqlo, $prim_id) {
        
        if ($this->abs_table=='') return;
        $table     = $this->ident;
        $abs_table = $this->abs_table;
        $pk_c      = PrimNameGet2($table);
        $pk_abs    = PrimNameGet2($abs_table);
        
        $sqlsel = "a.name from ".$table." c, ".$abs_table." a where c.".$pk_c."=" .$prim_id. " AND c.".$pk_abs."=a.".$pk_abs;
        
        $sqlo->Quesel($sqlsel);
        $sqlo->ReadRow();
        $tmp_name =$sqlo->RowData[0];
        if ( $tmp_name == "" ) {
            return;
        }
        $name="(a:" .$tmp_name. ")";

        return $name;
    }
    
    // OVERWRITE
    function xColDataGet ( &$sqlo,$colname,$idarr ) {}
    
}
