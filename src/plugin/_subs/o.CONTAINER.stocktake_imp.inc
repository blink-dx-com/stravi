<?php
require_once ("import/f.read_file_meta.inc");
require_once ("import/f.spreadsheet_ana.inc");
require_once 'o.CONTAINER.subs.inc';
require_once 'o.CONTAINER.mod.inc';
require_once ("visufuncs.inc");

/**
 * import
 * - each SUC-ID must appear ONCE in file !
 * @package o.CONTAINER.stocktake_imp.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class o_CONTAINER_stocktake_imp {
    
    function __construct($cont_id, $filename) {
        $this->cont_id = $cont_id;
        $this->filename=$filename;
    }
    
    /**
     * 
     * @param object $sqlo
     * @param int $suc_id
     * @param int $pos
     * @param float $newquant
     * @param float $quant_old
     * @return number
     *   -1 - unknown
     *   1  - update quant
     *   2  - new MAC
     *   5  - same quantity, no update
     *   10 no Quantity
     *   15 delete MAC entry
     *   
     *   
     */
    private function _store_update($sqlo, $suc_id, $pos, $newquant, $quant_old) {

        if ($this->go==2) {
        
            $retval=-1;
            
            if (!$pos) {
                if (!$newquant) {
                    return 10;
                } else {
                    $pos = $this->cont_mod_obj->getFreePos($sqlo); 
                    $this->cont_mod_obj->addAliquot($sqlo, $pos, $suc_id, 0, $newquant);
                    
                    return 2;
                }
            } else {
                
                if (!$newquant) {
                    $this->cont_mod_obj->delpos( $sqlo, $pos );
                    $retval = 15;
                    
                } else {
                    
                    if ($newquant==$quant_old) {
                        return 5;
                    }
                    
                    $arguin=array("QUANTITY"=>$newquant);
                    $this->cont_mod_obj->updateAliquot($sqlo, $pos, $arguin);
                    
                    $retval = 1;
                }
            }
            
            return $retval;
        
        }
        
    }
    
    
    private function _row_one($sqlo, $datafields) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        /*
         * search SUC-ID
         * - add or delete from Store
         * */
        $tablename='CONCRETE_SUBST';
        $cont_id  = $this->cont_id;
        $info_str ='';
        $info2_str ='';
        
        $suc_id = $datafields['mac-id'];
        if (!$suc_id) return;
        
        $infoarr=array();
        $infoarr['suc_id']    = $suc_id;
        
        if ( !gObject_exists ($sqlo, $tablename, $suc_id) ) {
            $infoarr['info'] = '<img src="images/i13_err.gif"> MAC not found in DB!';
            return $infoarr;
        }
        
       
        
        $newquant = $datafields['quant'];
        
        $quant_old  = $this->cont_obj->get_one_suc_QUANTITY($sqlo, $suc_id);
        $infoarr['quant.old'] = $quant_old;
        $infoarr['quant'] = $newquant;
        
        $store_param=array( 'substid'=>$suc_id );
        $this->cont_mod_obj->initSubst($store_param);
        $pos = $this->cont_mod_obj->getPosOfSubst($sqlo);
        
        if ($pos) {
            if ($infoarr['quant.old']==$infoarr['quant']) {
                $info_str = '<img src="images/i13_ok.gif">';
            } else {
                if ($infoarr['quant']) {
                    $info_str = '<img src="images/i13_warning.gif"> Quantity correction';
                } else {
                    $info_str = ' <svg width="13" height="13">'.
                        '<rect width="13" height="13" style="fill:rgb(120,120,255);stroke-width:0;" />'.
                        '</svg>  Empty';
                }
            }
        } else {
            $info_str =  ' <img src="images/NEW.logo.gif" title="new batch">';
        }
        
        $suc_fea = glob_elemDataGet3( $sqlo, 'CONCRETE_SUBST', array('CONCRETE_SUBST_ID'=>$suc_id), array('ABSTRACT_SUBST_ID', 'NAME') );
        
        
        $infoarr['suc_name'] = $suc_fea['NAME'];
        
        $sua_id  = $suc_fea['ABSTRACT_SUBST_ID'];
        $sua_fea = glob_elemDataGet3( $sqlo, 'ABSTRACT_SUBST', array('ABSTRACT_SUBST_ID'=>$sua_id), array('NAME', 'H_UNIT_ID') );
        
        $h_unit_id = $sua_fea['H_UNIT_ID'];
        $h_unit='?';
        if ($h_unit_id) {
            $h_unit = obj_nice_name ( $sqlo, 'H_UNIT', $h_unit_id );
        }
        
        // analyse other containers
        $other_cont=array();
        $other_str=array();
        $all_stores = oCONTAINER_many::get_one_SUC_stores($sqlo, $suc_id);
        foreach($all_stores as $cont_loop) {
            if ($cont_loop!=$cont_id) {
                $other_cont[] = $cont_loop;
                $other_str[]  = obj_nice_name ( $sqlo, 'CONTAINER', $cont_loop );
            }
        }
        if(!empty($other_str)) {
            $info2_str = '<img src="images/i13_warning.gif"> Found in other '.tablename_nice2('CONTAINER').'s: '.implode(', ',$other_str);
        }
        
        $infoarr['h_unit'] = $h_unit;
        $infoarr['sua_id'] = $sua_id;
        $infoarr['sua_name'] = $sua_fea['NAME'];
        $infoarr['info']  = $info_str;
        $infoarr['info2'] = $info2_str;

        
        $this->_store_update($sqlo, $suc_id, $pos, $newquant, $quant_old);
        if ($error->Got(READONLY))  {
            $errLast   = $error->getLast();
            $error_txt = $errLast->text;
            $error_id  = $errLast->id;
            $error->reset();
            
            $infoarr['info'] .= ' ERROR: '.$error_txt;
        }
 
        
        return $infoarr;
    }
    
   
    
    /**
     * what
     *   MAC-ID
     *   Quantity
     * @param object $sqlo
     */
    function import($sqlo, $sqlo2, $go) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        //$cont_nice=tablename_nice2('CONTAINER');
        
        $this->go = $go;
       
        $tempfile = $this->filename;
        
        $this->cont_mod_obj = new oContainerModC();
        $this->cont_mod_obj->setContainer($sqlo, $this->cont_id );
        $this->cont_mod_obj->activateModi($sqlo); 
       
        $file_lib = new f_read_file_meta($tempfile);
        $file_lib->open_file();
        $headers  = $file_lib->get_headers();
        $header   = $headers[0];
       
        $this->encoding_key = $file_lib->get_encoding_info();
       
        $colsExpect = array (
           'mac-id'       =>array('col.name'=>'MAC-ID',   'req'=>2 ),
           'quant'        =>array('col.name'=>'Quantity', 'req'=>2 ),
           
        );
        
        
        $spread_lib = new fSpreadheet_ana();
        $spread_lib->set_file_header($header);
        $spread_lib->analyseHeader($colsExpect);
        if ( $error->Got(READONLY) ) {
           $type= $file_lib->get_type();
           echo 'File-Type: '.$type.' Header: '.print_r($header,1)."<br>";
           $error->set( $FUNCNAME, 1, "File-Header analysis failed.");
           return;
        }
       
        $headerColPos = $spread_lib->getHeaderCol2Pos();
        
        $tabobj = new visufuncs();
        $headOpt = array( "title" => "Import data" );
        $headx  = array ('#', 'Info', 'MAA-ID', 'MAA-Name',  'MAC-ID',  'MAC-Name', 'New Quantity', 'Meas-Unit', 'OLD Quantity', 'Info2');
        $tabobj->table_head($headx,   $headOpt);
        
        $infoarr_ord=array('cnt', 'info', 'sua_id', 'sua_name', 'suc_id', 'suc_name', 'quant', 'h_unit',  'quant.old', 'info2');
        $color_col_id=6;
       
        
        $this->cont_obj = new oCONTAINER_SubsC();
        $this->cont_obj->setContainer($sqlo,  $this->cont_id );

        $suc_cache = array();
        $i=0;
        while( $file_lib->read_line(0) )  {
            
            $datafields_raw  = $file_lib->get_data_row();
            $datafields      = $spread_lib->getDataByDefColName( $datafields_raw );
            $has_data=0;
            foreach($datafields as $val) {
                if ($val!=='' and $val!==NULL) {
                    $has_data=1;
                    break;
                }
            }
            
            if (!$has_data) {
                // the end ...
                break;
            }
            
            
            $suc_cache[] = $datafields['mac-id'];
            
            $infoarr = $this->_row_one($sqlo,$datafields);
            $infoarr['cnt'] = ($i+1);
            if ($error->Got(READONLY))  {
                $error->set( $FUNCNAME, 1, 'Line: '+($i+1) );
                return;
            }
            
            $dataArr=array();
            foreach($infoarr_ord as $inf_key) {
                $dataArr[] = $infoarr[$inf_key];
            }
            
            $ropt=array();
            if ($infoarr['quant']!=$infoarr['quant.old']) {
                $ropt = array("td_style_arr" => array($color_col_id=>' style="background-color: #FFFA84;"') );
            }
            
            $tabobj->table_row ($dataArr, $ropt);
            $i++;
            

        }
        
        
        
        /*
         * REMOVE missing batches from stock 
         * 
         */
        $sqlsel = "distinct(CONCRETE_SUBST_ID) from CONT_HAS_CSUBST where CONTAINER_ID=" . $this->cont_id.
            " order by CONCRETE_SUBST_ID";

        $not_found_cnt=0;
        $sqlo2->Quesel($sqlsel);
        while ( $sqlo2->ReadRow() ) {
            
            
            $suc_id = $sqlo2->RowData[0];
            if ( !in_array($suc_id, $suc_cache) ) {
                
                if (!$not_found_cnt) {
                    $dataArr=array('','','','<b>... not found in Excel ...</b>','','','');
                    $tabobj->table_row ($dataArr);
                }
                
                
                $argu = array(
                    'mac-id'=> $suc_id,
                    'quant' => 0
                );
                
                $infoarr = $this->_row_one($sqlo,$argu);
                if ($error->Got(READONLY))  {
                    $error->set( $FUNCNAME, 1, 'Line: '+($i+1) );
                    return;
                }
                    
                $infoarr['cnt'] = ($i+1);
                
                $dataArr=array();
                foreach($infoarr_ord as $inf_key) {
                    $dataArr[] = $infoarr[$inf_key];
                }
                
                $tabobj->table_row ($dataArr);
                $i++;
                $not_found_cnt++;
            }
        }
        
        $tabobj->table_close();
        
        $file_lib->close_file();
        

    }
}