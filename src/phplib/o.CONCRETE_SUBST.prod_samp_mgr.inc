<?php
require_once ("insertx.inc");
require_once 'o.S_OBJLINK.mod.inc';
require_once 'o.S_OBJLINK.subs.inc';
require_once 'o.CONCRETE_SUBST.proto.inc';

/**
 * helper class for production retain sample creation
 * @package o.CONCRETE_SUBST.prod_samp_mgr.inc
 * @swreq REQ-0014 o.CONCRETE_SUBST > create INTERMEDIATE sample
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @created 2021-06-08
 */
class o_CONCRETE_SUBST_prod_samp {
    
    const SAMPLE_SUC_NAME='production-retain-sample';
    
    const OBJ_SUA_KEY='prod_sample';
    const OBJ_SUC_KEY='prod_sample';
    
    private $suc_fea;
    private $sua_id; // SUA-ID of SUC
    private $sample_sua;
    
    function __construct($sqlo, $suc_id) {
        $this->suc_id=$suc_id;
        $this->suc_fea = glob_ObjDataGet( $sqlo, 'CONCRETE_SUBST', $suc_id, array('ABSTRACT_SUBST_ID', 'NAME'));
        $this->sua_id = $this->suc_fea['ABSTRACT_SUBST_ID'];
        
        $this->sample_sua = $this->get_sample_sua($sqlo);
    }
    
    // return int SUA_ID
    function get_sample_sua($sqlo) {
        $childArr = oS_OBJLINK_subs::getChiObjByTabOrKey($sqlo, 'ABSTRACT_SUBST', $this->sua_id, 'ABSTRACT_SUBST', self::OBJ_SUA_KEY);
        if (!empty($childArr)) return current($childArr);
    }
    
    // return array of SUC_IDs
    function get_all_inter_SUCs($sqlo) {
        $childArr = oS_OBJLINK_subs::getChiObjByTabOrKey($sqlo, 'CONCRETE_SUBST', $this->suc_id, 'CONCRETE_SUBST', self::OBJ_SUA_KEY);
        return $childArr;
    }
    
    function create_sample_sua($sqlo) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        // sample_sua already exists ?
        $this->sample_sua=$this->get_sample_sua($sqlo);
        if ($this->sample_sua) {
            return $this->sample_sua;
        }
        
        $mo_sua_feats = glob_ObjDataGet( $sqlo, 'ABSTRACT_SUBST', $this->sua_id, array('NAME'));
        $inter_sua_name = $mo_sua_feats['NAME'].'-RETAIN-SAMPLE';
        
        $sua_table='ABSTRACT_SUBST';
        $insertlib = new insertC();
        $args= array(
            "vals"=>array(
                'NAME'=>$inter_sua_name,
                'NOTES'=>'identifies a production-retain-sample of db:MAA:'.$this->sua_id
            )
            
        );
        $samp_sua_id = $insertlib->new_meta($sqlo, $sua_table, $args);
        if ($error->Got(READONLY))  {
            $error->set( $FUNCNAME, 1, 'Error on creation of RETAIN-MAA.' );
            return;
        }
        $this->sample_sua = $samp_sua_id;
        
        $link_lib = new oS_OBJLINK_mod();
        $link_lib->setObj($sqlo, $sua_table, $this->sua_id);
        $link_lib->addLink($sqlo, $sua_table, $samp_sua_id, self::OBJ_SUA_KEY);
        
        return $samp_sua_id;
    }
    
   
    
    /**
     * create a SAMPLE-SUC
     * @param object $sqlo
     * @param int $rel_id
     */
    function create_samp_suc($sqlo, int $rel_id) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        // it was checked, that no sample_sua exists ?
        $suc_table='CONCRETE_SUBST';
        
        if (!$this->sample_sua) {
            $this->create_sample_sua($sqlo);
        }
        if (!$this->sample_sua) {
            $error->set( $FUNCNAME, 1, 'Error on creation of RETAIN-MAA.' );
            return;
        }
        
        $suc_proto_lib = new oCONCRETE_SUBST_proto($this->suc_id);
        $proto_log = $suc_proto_lib->get_c_protos($sqlo);
        $proto_arr=array();
        foreach($proto_log as $row) {
            $pra_id=$row['ap'];
            // $pra_nice = obj_nice_name ( $sqlo, 'ABSTRACT_PROTO', $pra_id ); 
            $proto_arr[] = '- db:PRA:'.$pra_id.' ';
        }
        
        $suc_name=$this->suc_fea['NAME'];
        if($suc_name==NULL) $suc_name=$this->suc_id; // fall back
        $suc_name .= '-RETAIN-'.$rel_id;
        
        $notes = '';
        $notes .= 'RETAIN-sample of db:MAC:'.$this->suc_id." \n";
        $notes .= 'Executed protocols: ';
        if (empty($proto_arr)) {
            $notes .= 'none';
        } else {
            $notes .= "\n". implode("\n",$proto_arr);
        }
        $notes .= "\n";
        
        $insertlib = new insertC();
        $args= array(
            "vals"=>array(
                'NAME'=>  $suc_name,
                'ABSTRACT_SUBST_ID'=>$this->sample_sua,
                'NOTES'=> $notes
            )
            
        );
        $samp_suc_id = $insertlib->new_meta($sqlo, $suc_table, $args);
        if ($error->Got(READONLY))  {
            $error->set( $FUNCNAME, 5, 'Error on creation of RETAIN-MAC.' );
            return;
        }
        $link_lib = new oS_OBJLINK_mod();
        $link_lib->setObj( $sqlo, $suc_table, $this->suc_id);
        $link_lib->addLink($sqlo, $suc_table, $samp_suc_id, self::OBJ_SUC_KEY);
        
        return $samp_suc_id;
    }
}
