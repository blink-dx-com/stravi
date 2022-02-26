<?php
require_once('glob.obj.create.ABS.inc');

/**
 * object specific modification class
 * $Header: trunk/src/phplib/o.EXTRA_ATTRIB.xadd.inc 59 2018-11-21 09:04:09Z $
 * @package o.EXTRA_ATTRIB.xadd.inc
 * @author  Steffen Kube (steffen.kube@gmx.de, steffen@blink-dx.com)
 */
class oEXTRA_ATTRIB_xmodC extends gObjCreateC_ABS {

/**
 * this function considers, that any attribute is can be copied
    would be nice to have $oriargu["EXTRA_CLASS_ID"]
 * @param $sql
 * @param $args
 */
function copy_prep( &$sql, &$args) {
    global $error;
	$FUNCNAME= __CLASS__.':'.__FUNCTION__;
    
    $retval  = 0;
    $oriargu = $args["vals"];
        
    $argusAll = $oriargu; /* just loaded from DB */
    
    $pos_max = 0;
    $mapcol_s="S00";
    $mapcol_n="N00";
    
    if ( $argusAll["EXTRA_CLASS_ID"] ) {
        $sqls="name from EXTRA_ATTRIB where EXTRA_CLASS_ID=".$argusAll["EXTRA_CLASS_ID"]. " AND
                name=". $sql->addQuotes($argusAll['NAME']) ;
        $sql->Quesel($sqls);
        if ( $sql->ReadRow() ) {
            $errtxt = "name '". $argusAll["NAME"] ."' already exists.<br>\n";
            $error->set( $FUNCNAME, 1, $errtxt );
            return;
        }
        $sqls="max(MAP_COL) from EXTRA_ATTRIB where EXTRA_CLASS_ID=".$argusAll["EXTRA_CLASS_ID"]. " AND MAP_COL
        like 'S%'"  ;
        $sql->Quesel($sqls);
        if ($sql->ReadRow()) {
            $mapcol_s = $sql->RowData[0];
        } 
        $sqls="max(MAP_COL) from EXTRA_ATTRIB where EXTRA_CLASS_ID=".$argusAll["EXTRA_CLASS_ID"]. " AND MAP_COL
        like 'N%'"  ;
        $sql->Quesel($sqls);
        if ($sql->ReadRow()) {
            $mapcol_n = $sql->RowData[0]; 
        } 
        $sqls="max(POS) from EXTRA_ATTRIB where EXTRA_CLASS_ID=".$argusAll["EXTRA_CLASS_ID"];
        $sql->Quesel($sqls);
        if ($sql->ReadRow()) {
            $pos_max = $sql->RowData[0]; 
        } 
    }
    unset ($argusAll["EXTRA_ATTRIB_ID"]); // automatic index

    $tmpvar = $argusAll["MAP_COL"];
    $tmptype= substr($tmpvar,0,1);
    if ($tmptype=="")  $tmptype="S";
    $mapcol_max=substr($mapcol_n,1);
    if ($tmptype=="S") $mapcol_max=substr($mapcol_s,1);
    
    if ($mapcol_max>=20) {
        $errtxt = "WARNING: number of columns for '" .$tmptype. "' exceeded.";
        $error->set( $FUNCNAME, 2, $errtxt );
        return;
    } else {
        $tmpsub=$mapcol_max+1;    
        $tmpsub=str_pad( $tmpsub, 2, "0", STR_PAD_LEFT );
        $argusAll["MAP_COL"]=$tmptype.$tmpsub;
        
    }
    $tmppos = $pos_max + 1;
    $argusAll["POS"] = $tmppos; // new position

    if ($retval>=0) {
        $args["vals"] = $argusAll; // write back !!!
    }
        
}

}

