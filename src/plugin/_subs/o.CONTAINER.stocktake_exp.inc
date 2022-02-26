<?php
require_once 'lev1/f.exportDataFile.inc';
require_once 'o.CONTAINER.subs.inc';

/**
 * export
 * @package o.CONTAINER.stocktake_exp.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class o_CONTAINER_stocktake_exp {
    
    function __construct($cont_id, $scriptid) {
        $this->cont_id = $cont_id;
        $this->scriptid=$scriptid;
    }
    
    /**
     * what
     *   #
     *   material template id
     *   material template name
     *   MAC-ID
     *   MAC-Name
     *   Quantity
     *   H-unit
     *   Comment
     * @param object $sqlo
     */
    function export($sqlo, $sqlo2) {
        
       $cont_nice=tablename_nice2('CONTAINER');
       $exportObj = new f_exportDataFile_C('xls',  $this->scriptid, $cont_nice.'_ID'.$this->cont_id.'_stock');
        
       $cont_obj = new oCONTAINER_SubsC();
       $cont_obj->setContainer($sqlo,  $this->cont_id );
       
       $sqlsel_suas = $cont_obj->sql_all_sua_of_sucs();
       
       $sqlsel = 'CONCRETE_SUBST_ID from CONCRETE_SUBST where ABSTRACT_SUBST_ID in (select '.$sqlsel_suas.') and '.
           "( CONCRETE_SUBST_ID in (".
                "select CONCRETE_SUBST_ID from CONT_HAS_CSUBST where CONTAINER_ID=" . $this->cont_id. 
                ')'.
           ") order by ABSTRACT_SUBST_ID";
        
       $headerx = array( array('#', 'MAA-ID', 'MAA-Name',  'MAC-ID',  'MAC-Name', 'Quantity', 'Meas-Unit') );
       $exportObj->outputStart( $headerx );
       
       $cnt=0;
       $sqlo2->Quesel($sqlsel);
       while ( $sqlo2->ReadRow() ) {
           
           $suc_id = $sqlo2->RowData[0];
           $quant  = $cont_obj->get_one_suc_QUANTITY($sqlo, $suc_id);
           
           $suc_fea = glob_elemDataGet3( $sqlo, 'CONCRETE_SUBST', array('CONCRETE_SUBST_ID'=>$suc_id), array('ABSTRACT_SUBST_ID', 'NAME') );
           
           $sua_id = $suc_fea['ABSTRACT_SUBST_ID'];
           $sua_fea = glob_elemDataGet3( $sqlo, 'ABSTRACT_SUBST', array('ABSTRACT_SUBST_ID'=>$sua_id), array('NAME', 'H_UNIT_ID') );
           
           $h_unit_id = $sua_fea['H_UNIT_ID'];
           $h_unit='?';
           if ($h_unit_id) {
               $h_unit = obj_nice_name ( $sqlo, 'H_UNIT', $h_unit_id );
           }
           
           
           $tmparr = array(
               ($cnt+1),
               $sua_id,
               $sua_fea['NAME'],
               $suc_id,
               $suc_fea['NAME'],
               $quant,
               $h_unit
           ) ;
           $exportObj->oneRow( $tmparr );
           $cnt++;
       }
       
      
       $exportObj->close();
       $text = $exportObj->getDownloadText();
       
       echo $text."<br>";
    }
}