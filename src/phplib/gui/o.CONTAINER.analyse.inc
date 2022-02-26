<?php
require_once 'o.CONTAINER.subs.inc';
require_once 'o.CONCRETE_SUBST.stockwarnC.inc';
require_once ("visufuncs.inc");

/**
 * container analysis
 * @package o.CONTAINER.analyse.inc
 */ 
class oCONTAINER_analyse {
    
    
    /**
     * show list of critical SUCs for a list of containers; but ONLY charge clean flag
     * @param unknown $sqlo
     * @param unknown $sqlo3
     * @param array $container_arr
     */
    function warning_SUCs_chargeclean(&$sqlo, &$sqlo2, $container_arr) {
        
        $stockWarnLib = new o_CONCRETE_SUBST_stockwarnC();
        $variolib = new oS_VARIO_sub('ABSTRACT_SUBST');
        $vario_key = "store.ana.singlebatch";
       
        
        
        $critical_status = array('min', 'warn');
        
        $tabobj = new visufuncs();
        $headOpt = array( "title" => "Quantity warnings: CHARGE CLEAN only; Storages: ". implode(",", $container_arr) );
        $headx  = array ("Storage", "Pos", 'material' ,'material (tmpl)' , 'Storage-Status',	'quant (warn)', 'quantity (all positions)'	);
        $tabobj->table_head($headx,   $headOpt);

        $container_cache=array();
        $objLinkLib = new fObjViewC();
        
        $container_str = implode(', ',$container_arr);
        
        $sqlsel = "c.* from CONT_HAS_CSUBST c where c.CONTAINER_ID in (" . $container_str . ') and c.CONCRETE_SUBST_ID>0 '. 
            ' order by c.CONTAINER_ID, c.POS DESC'; 
        
        $sqlo2->Quesel($sqlsel);
        $cnt=0;
        while ( $sqlo2->ReadArray() ) {
 
            $featLoop = $sqlo2->RowData;
            $cont_id  = $sqlo2->RowData['CONTAINER_ID'];
            $suc_id   = $featLoop['CONCRETE_SUBST_ID'];
            
            if (!is_array($container_cache[$cont_id])) { 
                $container_cache[$cont_id] = $objLinkLib->bo_display( $sqlo, 'CONTAINER', $cont_id );
            }
            $cont_html = $container_cache[$cont_id];
            
            // check SUA
            $sua_id = glob_elementDataGet( $sqlo, 'CONCRETE_SUBST', 'CONCRETE_SUBST_ID', $suc_id, 'ABSTRACT_SUBST_ID');
            $singlebatch_flag = $variolib->getValByKey( $sqlo,  $sua_id, $vario_key );
            
            if (!$singlebatch_flag) {
                continue; // not flagged
            }
            
            
            // storage warnings
            $sqlsel = "SUM(QUANTITY) from CONT_HAS_CSUBST where CONTAINER_ID=" . $cont_id . ' and CONCRETE_SUBST_ID='. $suc_id.
                " group by CONCRETE_SUBST_ID ";
            $sqlo->Quesel($sqlsel);
            $sqlo->ReadRow();
            $quant_sum = $sqlo->RowData[0];
            
            $store_status_arr = $stockWarnLib->get_suc_info($sqlo, $suc_id, $quant_sum);
            
            if ( !in_array($store_status_arr['status'], $critical_status) ) {
                continue; // not critical
            }
            
            $suc_html = $objLinkLib->bo_display( $sqlo, 'CONCRETE_SUBST', $suc_id ); 
            $sua_html = $objLinkLib->bo_display( $sqlo, 'ABSTRACT_SUBST', $sua_id );
            
            $row_features = array( $cont_html, $featLoop['POS'], $suc_html, $sua_html, $store_status_arr['ICON'], $store_status_arr['store.warn'], $quant_sum  );
            
            
            $tabobj->table_row ($row_features);
        }
        
       
        
        $tabobj->table_close();
    }
    
    /**
     * SUA warnings, only NOT charge clean
     * @param unknown $sqlo
     * @param unknown $sqlo2
     * @param unknown $container_arr
     */
    function warning_SUAs(&$sqlo, &$sqlo2, $container_arr) {
        
        $stockWarnLib = new o_CONCRETE_SUBST_stockwarnC();
        
        $critical_status = array('min', 'warn');
        
        $tabobj = new visufuncs();
        $headOpt = array( "title" => "Quantity warnings: analyse quants of same material template; Storages: ". implode(",", $container_arr) );
        $headx  = array ('material (template)' , 'Storage-Status',	'quant (warn)', 'quantity (all positions)'	);
        $tabobj->table_head($headx,   $headOpt);
        
        $objLinkLib = new fObjViewC();
        
        $container_str = implode(', ',$container_arr);
        
        $sqlsel1 = "select c.CONCRETE_SUBST_ID from CONT_HAS_CSUBST c where c.CONTAINER_ID in (" . $container_str . ') and c.CONCRETE_SUBST_ID>0';
        $sqlsel2 = "distinct(x.ABSTRACT_SUBST_ID) from CONCRETE_SUBST x where x.CONCRETE_SUBST_ID in (".$sqlsel1.')';
        $sqlo2->Quesel($sqlsel2);
        $cnt=0;
        
        while ( $sqlo2->ReadArray() ) {
            
 
            $sua_id = $sqlo2->RowData['ABSTRACT_SUBST_ID'];  
            
            // storage warnings
            $sql_pre = "select CONCRETE_SUBST_ID from CONCRETE_SUBST where ABSTRACT_SUBST_ID=".$sua_id;
            $sqlsel  = "SUM(QUANTITY) from CONT_HAS_CSUBST where CONTAINER_ID in (" . $container_str . ') and CONCRETE_SUBST_ID in ('.$sql_pre.")";
            $sqlo->Quesel($sqlsel);
            $sqlo->ReadRow();
            $quant_sum = $sqlo->RowData[0];
            
            $store_status_arr = $stockWarnLib->get_sua_info($sqlo, $sua_id, $quant_sum);
            
            if ( !in_array($store_status_arr['status'], $critical_status) ) {
                continue; // not critical
            }
            
            $sua_html = $objLinkLib->bo_display( $sqlo, 'ABSTRACT_SUBST', $sua_id );
            
            $row_features = array( $sua_html, $store_status_arr['ICON'], $store_status_arr['store.warn'], $quant_sum  );
            
            
            $tabobj->table_row ($row_features);
        }
        
        
        
        $tabobj->table_close();
        
    }
    
}