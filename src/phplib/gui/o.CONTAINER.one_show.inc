<?php
require_once ("visufuncs.inc");
require_once 'o.H_UNIT.siSubs.inc';
require_once ("f.objview.inc");	
require_once 'o.CONTAINER.subs.inc';

/**
 * ONE container
 * @package o.CONTAINER.one_show.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class oCONTAINER_one_show {
    
    function __construct($id) {
        $this->id = $id;
    }
    
    /**
     * get COUNT of MAAs
     * @param object $sqlo
     * @return int
     */
    function count_suas($sqlo) {
        $contLib = new oCONTAINER_SubsC();
        $contLib->setContainer($sqlo, $this->id);
        $cnt_CHA_SUA = $contLib->count_CHA_SUA($sqlo);
        return $cnt_CHA_SUA;
    }
    
    
    /**
     * SUAs in CONT_HAS_ASUBST
     * @param object $sqlo
     * @param object $sqlo2
     */
    function get_CHA_Summary($sqlo, $sqlo2) {
        
        $MAX_SHOW_ENTRY=200; // max shown number
        
        $subsel = "select ABSTRACT_SUBST_ID from CONT_HAS_ASUBST where CONTAINER_ID=".$this->id;
        $sqls2 = "ABSTRACT_SUBST_ID from ABSTRACT_SUBST where ABSTRACT_SUBST_ID in (". $subsel.")".
            " order by NAME";
        
        $sqlo2->Quesel($sqls2);
        
        $tabobj  = new visufuncs();
        $headOpt = array( "title" => "Summary of registered MAAs");
        $head1   = array ("#");
        $head3   = array( "MAA *", "unit");
        
        
        $headx = array_merge($head1, $head3);
        $tabobj->table_head($headx,   $headOpt);
        
        $objlnkLib   = new fObjViewC();
        $measUnitLib = new oH_UNIT_siSubs();
        
        $cnt=0;
        while ( $sqlo2->ReadRow() ) {
            
            $suaid     = $sqlo2->RowData[0];
            // $suaname   = $sqlo2->RowData[1];
            if ($cnt>$MAX_SHOW_ENTRY) {
                $dataArr=array("...", "... more ...","");
                $tabobj->table_row ($dataArr);
                break;
            }
            $substlnk = $objlnkLib->bo_display( $sqlo, "ABSTRACT_SUBST", $suaid  );
            $unitName    = $measUnitLib->getUnitNameFromSUA($sqlo, $suaid);
            $dataArr=array( $cnt+1, $substlnk, $unitName);
            $tabobj->table_row ($dataArr);
            $cnt++;
        }
        
        $tabobj->table_close();
        
    }
}