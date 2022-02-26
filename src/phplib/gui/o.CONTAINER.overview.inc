<?php
require_once 'o.CHIP_READER.subs.inc';
require_once 'o.CONTAINER.subs.inc';
require_once ("visufuncs.inc");

/**
 * container overview, many containers
 * @package o.CONTAINER.overview.inc
 */ 
class oCONTAINER_overview {
    
    
    /**
     * show list of containers and their contained MAAs, initial object is: DEVICE:
     * @param $sqlo
     * @param $projid : home project, where CHIP_READERs are situated
     */
    function show_Dev_Cont_MAA( &$sqlo, $projid, $options=array()  ) {
        
        $MAX_SHOW = 50;
        $table='CHIP_READER';
        $table_nice = tablename_nice2($table);
        $store_nice = tablename_nice2('CONTAINER');
        
        $title=  $store_nice.'-Liste (folder ID:'.$projid.')';
        
        if (!$projid) {
            echo 'Folder-ID not set.';
            return;
        }
        
        $subsql = cProjSubs::getTableSQL($projid,'CHIP_READER');
        // main SQL-query
        $sqlsel_all   = "x.CHIP_READER_ID, x.LOCATION, x.NOTES from CHIP_READER x where CHIP_READER_ID in (".$subsql.")";
        $sqlo->Quesel($sqlsel_all.' order by x.LOCATION, x.NAME');
        $cnt=0;
        $max_reached=0;
        
        $dev_arr=array();
        
        while ( $sqlo->ReadRow() ) {
            
            if ($cnt>$MAX_SHOW) {
                $max_reached=1;
                break;
            }
            $objid = $sqlo->RowData[0];
            $loc   = $sqlo->RowData[1];
            $notes = $sqlo->RowData[2];
            
           
            
            $dev_arr[]=array( 'id'=>$objid, 'loc'=>$loc, 'NOTES'=>$notes );
            
            
            $cnt++;
        }
        
        if ($dev_arr==NULL) {
            echo $title. ': Keine aktuellen Objekte.<br />';
            return;
        }
        
        $objLinkLib = new fObjViewC();
        $tabobj     = new visufuncs();
        $headOpt    = array( "title" => $title);
        $headx      = array ($table_nice, 'MAAs', 'Notes');
        $colspan=sizeof($headx);
        $tabobj->table_head($headx,  $headOpt);
       
        $last_Location='';
        
        $container_lib = new oCONTAINER_SubsC();
        $dev_sub_lib = new oCHIP_READER_subs();
        
        foreach($dev_arr as $dev_row) {
            
            $objid = $dev_row['id'];
            $loc   = $dev_row['loc'];
            
            if ($last_Location!=$loc) {
                $tmp_loc = '<b>Loc: '.$dev_row['loc'].'</b>';
                
                $dataArr=array( $tmp_loc );
                $rowopt =array("colspan"=>$colspan);
                $tabobj->table_row ($dataArr, $rowopt);
            }
           
            $cnt_MAA='';
            
            $dev_sub_lib->set_dev($sqlo, $objid);
            $container_id = $dev_sub_lib->get_container($sqlo);
            if ($container_id) {
                $container_lib->setContainer($sqlo, $container_id);
                $cnt_MAA = $container_lib->count_CHA_SUA($sqlo);
                
                $linkTxt = $objLinkLib->bo_display( $sqlo, 'CONTAINER', $container_id);
            } else {
                $linkTxt = $objLinkLib->bo_display( $sqlo, 'CHIP_READER', $objid);
            }
            
           
            
            $dataArr=array( $linkTxt,  $cnt_MAA, substr($dev_row['NOTES'],0,30) );
            $tabobj->table_row ($dataArr);
            
            $last_Location=$loc; 
        }
       
        $tabobj->table_close();
    }
    
}