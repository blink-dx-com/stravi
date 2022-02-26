<?php
require_once 'o.S_VARIO.subs.inc';

/**
 * SUA logistics
 * @package o.ABSTRACT_SUBST.logi.inc
 * @author  Steffen Kube (steffen@blink-dx.com)
 */
class oABSTRACT_SUBST_logi {
    
    private $sua_id;
    
    private $keys = array(
        'scrap',
        "store.min",
        "store.warn",
        "store.norm",
        "store.ana.singlebatch"
    );
    
    function __construct($sua_id=NULL) {
        if ($sua_id) {
            $this->set_sua($sua_id);
        }
    }
    
    function get_vario_keys() {
        return $this->keys;
    }
    
    /**
     * get SQL string: "SELECT OBJ_ID FROM S_VARIO ..."
     * @return string
     */
    function get_SQLcmd_allhave_config($sqlo) {
        
        $variolib = new oS_VARIO_sub('ABSTRACT_SUBST');
        $sql_cmd = $variolib->getSqlSelOfKey($sqlo, "store.min", 'VALUE is not NULL');
        return $sql_cmd;
    }
    
    function set_sua($sua_id) {
        $this->sua_id = $sua_id;
    }
    
    /**
     * get data of SUA
     * @param object $sqlo
     * @return array KEY=>val
     */
    function get_data(&$sqlo) {
        
        if ( !$this->sua_id )	sys_error_my( 'class not initialized.' );
        
        
        $variolib = new oS_VARIO_sub('ABSTRACT_SUBST');
        
        $data=array();
        foreach($this->keys as $key) {
            $data[$key] = $variolib->getValByKey( $sqlo,  $this->sua_id, $key );
        }
        
        return $data;
    }
    
    function has_data(&$data) {
        $has_data=0;
        foreach($data as $key=>$val) {
            if ($val!=NULL) $has_data=1;
        }
        return $has_data;
    }
    
    /**
     * get storage and SUC summary of SUA
     * @param object $sqlo
     * @param int $container_id
     * @param int $with_suc_ids 0,1 give all SUCs which have  storage quantities
     * @return array
     *  'store.quant' = quantity in storage
        'suc.cnt'     = count 
        'suc.ids'     = array of SUC-IDs
     */
    function get_storage_sum(&$sqlo, $container_id=0, $with_suc_ids=0) {
        if ( !$this->sua_id )	sys_error_my( 'class not initialized.' );
        
        $sql_pre = " from CONCRETE_SUBST where ABSTRACT_SUBST_ID=" . $this->sua_id;
        $sql_container = ' and (CONTAINER_ID='.$container_id.')';
        
        $sqlsel  = "SUM(QUANTITY) from CONT_HAS_CSUBST where CONCRETE_SUBST_ID in (select CONCRETE_SUBST_ID ". $sql_pre. ")";
        if ($container_id) {
            $sqlsel .= $sql_container;
        }
        $sqlo->Quesel($sqlsel);
        $sqlo->ReadRow();
        $quant_sum = $sqlo->RowData[0];
        if (!$quant_sum) $quant_sum=0;
        
        
        $sqlo->Quesel("count(CONCRETE_SUBST_ID)". $sql_pre);
        $sqlo->ReadRow();
        $cnt_suc = $sqlo->RowData[0];
        
        $result=array();
        $result['store.quant']   = $quant_sum;
        $result['suc.cnt'] = $cnt_suc;
        
        if ($with_suc_ids) {
            
            $sqlsel  = "max(CONCRETE_SUBST_ID) from CONT_HAS_CSUBST where CONCRETE_SUBST_ID in (select CONCRETE_SUBST_ID ". $sql_pre. 
                ") ";
            
            if ($container_id) {
                $sqlsel .= $sql_container;
            }
            $sqlsel  .= "group by CONCRETE_SUBST_ID having SUM(QUANTITY)>0";
            
            $sqlo->Quesel($sqlsel);
            $suc_arr=array();
            while ($sqlo->ReadRow()) {
                $suc_id    = $sqlo->RowData[0];
                $suc_arr[] = $suc_id;
            }
            $result['suc.ids'] = $suc_arr;
        }
        
        return $result;
    }
    
    /**
     * get storage details of SUA
     * - only analyse SUCs with container.QUANTITY
     * - get max $MAX_SUC_ANA SUCs
     * @param object $sqlo
     * @param int $max_ret_num : max number of results
     * @return array
     *  'suc.cnt' => num of total sucs in CONTAINER
     *  'data'=> array ... details, ORDER by SUC_ID DESC
     *     'sucid' => $suc_id, 
     *     'q'     => $quant_sum
     *     'cont'
     */
    function get_storage_details(&$sqlo, $MAX_SUC_ANA=20) {
        
        if ( !$this->sua_id )	sys_error_my( 'class not initialized.' );
        
        $result_arr=array();
        
        $result=array();
        $result['data']=array();
        
        $sql_pre = " from CONCRETE_SUBST where ABSTRACT_SUBST_ID=" . $this->sua_id;
        
        $sqlsel  = "count(distinct(CONCRETE_SUBST_ID)) from CONT_HAS_CSUBST where ".
           " QUANTITY>0 and ".
           " CONCRETE_SUBST_ID in (select CONCRETE_SUBST_ID ". $sql_pre. ")";
        $sqlo->Quesel($sqlsel);
        $sqlo->ReadRow();
        $suc_sum = $sqlo->RowData[0];
        $result['suc.cnt'] = $suc_sum;
        
        if (!$suc_sum) return $result;

        $cnt=0;
        $sqlsel  = "SUM(QUANTITY), max(CONCRETE_SUBST_ID) from CONT_HAS_CSUBST".
            " where CONCRETE_SUBST_ID in (select CONCRETE_SUBST_ID ". $sql_pre. 
            ") group by CONCRETE_SUBST_ID having SUM(QUANTITY)>0 order by max(CONCRETE_SUBST_ID) DESC ";
        $sqlo->Quesel($sqlsel);
        while ($sqlo->ReadRow() ) {
            
            if($cnt>$MAX_SUC_ANA) {
                break; // stop here
            }
            
            $quant_sum = $sqlo->RowData[0];
            $suc_id = $sqlo->RowData[1];
            if ($quant_sum<=0) continue;
            
            $result_arr[]=array('sucid'=>$suc_id, 'q'=>$quant_sum);
            $cnt++;
        }

        if (!sizeof($result_arr)) return $result;
        
        
        foreach($result_arr as $row) {
            $suc_id = $row['sucid'];
            $sqlsel  = "SUM(QUANTITY), MAX(CONTAINER_ID) from CONT_HAS_CSUBST where CONCRETE_SUBST_ID=".$suc_id." group by CONTAINER_ID";
            $sqlo->Quesel($sqlsel);
            while ($sqlo->ReadRow() ) {
                $quant_sum = $sqlo->RowData[0];
                $cont_id   = $sqlo->RowData[1];
                if ($quant_sum<=0) continue;
                
                $result['data'][]=array('sucid'=>$suc_id, 'q'=>$quant_sum, 'cont'=>$cont_id);
            }
        }
        
        return $result;
    }
    
    /**
     * get storages of SUA
     * @param object $sqlo
     * @return array of CONTAINER_Ds
     * 
     */
    function get_storage_CHA(&$sqlo) {
        if ( !$this->sua_id )	sys_error_my( 'class not initialized.' );
        
        $cont_arr=array();
        
        $sqlsel  = "distinct(CONTAINER_ID) from CONT_HAS_ASUBST where ABSTRACT_SUBST_ID=".$this->sua_id;
        $sqlo->Quesel($sqlsel);
        while ($sqlo->ReadRow() ) {    
            $cont_arr[] = $sqlo->RowData[0];
        }

        return $cont_arr;
    }
    
}
