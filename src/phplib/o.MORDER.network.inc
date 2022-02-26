<?php 
require_once 'o.ABSTRACT_SUBST.proto.inc';
require_once 'o.ABSTRACT_PROTO.stepx.inc';
require_once 'o.S_OBJLINK.subs.inc';
require_once 'o.S_OBJLINK.mod.inc';
require_once ('o.S_VARIO.subs.inc');

/**
 * MORDER network methods
 * @author skube
 * @package o.MORDER.network.inc
 */
class oMORDER_network {
    
    private $LINK_KEY='sub-order';
    const VARIO_SUA_CHILD_SUA='MORDER.child_sua';
    
    function __construct($mord_id) {
        $this->mord_id=$mord_id;
    }
    
    /**
     * has parent ID ?
     * @param object $sqlo
     * @return int 0,1
     */
    function has_parent($sqlo) {
        $objlink_lib = new oS_OBJLINK_subs();
        $parents = $objlink_lib->getMoObjsbyTabKey($sqlo, 'MORDER', $this->mord_id, 'MORDER', $this->LINK_KEY);
        if (empty($parents)) return 0;
        return 1; 
    }
    
    /**
     * get parent ID
     * @param object $sqlo
     * @return int
     */
    function get_parent($sqlo) {
        $objlink_lib = new oS_OBJLINK_subs();
        $parents = $objlink_lib->getMoObjsbyTabKey($sqlo, 'MORDER', $this->mord_id, 'MORDER', $this->LINK_KEY);
        if (empty($parents)) return 0;
        return current($parents); // first parent
    }
    
    /**
     * get ALL parent ID
     * @param object $sqlo
     * @return array of int
     */
    function get_parents($sqlo) {
        $objlink_lib = new oS_OBJLINK_subs();
        $parents = $objlink_lib->getMoObjsbyTabKey($sqlo, 'MORDER', $this->mord_id, 'MORDER', $this->LINK_KEY);
        return $parents; 
    }
    
    /**
     * get all SUAs which are HALB
     * - from protocol
     * @param object $sqlo
     * @return array of SUA-IDs
     */
    function get_sub_suas($sqlo) {
        
        $need_usage_name='HALB';
        
        $objInfoLib = new objInfoC();
        $MORDERer_feats = $objInfoLib->getParamsRaw ( $sqlo, 'MORDER', $this->mord_id);
        $sua_id = $MORDERer_feats['vals']['ABSTRACT_SUBST_ID'];
        
        // get PROTOCOLS
        $sua_lib = new oABSTRACT_SUBST_proto();
        $sua_lib->set_obj($sqlo, $sua_id);
        $proto_log = $sua_lib->get_protos($sqlo);
        
        if (empty($proto_log)) return;
        
        $row = current($proto_log);
        $apid = $row['ap'];
        
        // get SUAs
        $pra_lib = new oABSTRACT_PROTO_stepx($apid);
        $suas_raw = $pra_lib->get_SUAs($sqlo);
        if (empty($suas_raw)) return;
        
        
        
        $need_usage_id = glob_elementDataGet( $sqlo, 'H_USAGE', 'NAME', $need_usage_name, 'H_USAGE_ID');
        
        $suas_out = array();
        
        foreach($suas_raw as $row) {
            
            $sua_id = $row['sua_id'];
            if (!$sua_id) continue;
            $sua_feat = glob_elemDataGet3( $sqlo, 'ABSTRACT_SUBST', array('ABSTRACT_SUBST_ID'=>$sua_id), array('H_USAGE_ID') );
            
            if ($need_usage_id==$sua_feat['H_USAGE_ID']) {
                if (!in_array($sua_id, $suas_out)) {
                    $suas_out[]=$sua_id;
                }
            }
        }
        
        return $suas_out;
    }
    
    /**
     * get all SUAs where THIS SUA is configured as potential managed by MORDER
     * - from protocol
     * @param object $sqlo
     * @return array of SUA-IDs
     */
    function get_mo_suas($sqlo) {

        $MORDERer_feats = glob_ObjDataGet( $sqlo, 'MORDER', $this->mord_id, array('ABSTRACT_SUBST_ID') );
        $sua_id = $MORDERer_feats['ABSTRACT_SUBST_ID'];
       
        $vario_lib   = new oS_VARIO_sub('ABSTRACT_SUBST');
        $mother_suas  = $vario_lib->getObjectsOfKeyVal($sqlo, self::VARIO_SUA_CHILD_SUA, $sua_id);
        return $mother_suas;
        
    }
    
    /**
     * get IDs of direct sub orders
     * @param object $sqlo
     */
    function get_orders($sqlo) {
        
        $objlink_lib = new oS_OBJLINK_subs();
        $children = $objlink_lib->getChiObjByChiTab($sqlo, 'MORDER', $this->mord_id, 'MORDER');
        return $children;
    }
    
    /**
     * get planned SUAs versus active ORDERs
     * @param array $sua_arr
     * @param array $ord_arr
     * @return array $sua_v_ord
     *   [SUA_ID] = array('ord'=>ID)
     */
    static function suas_versus_subs_ords($sqlo, &$sua_arr, &$ord_arr) {

        
        $sua_v_ord=array();
        
        if (!empty($ord_arr)) {
            foreach($ord_arr as $ord_id) {
    
                $ord_feat = glob_elemDataGet3( $sqlo, 'MORDER', array('MORDER_ID'=>$ord_id), array('ABSTRACT_SUBST_ID') );
                $ord_sua_id = $ord_feat['ABSTRACT_SUBST_ID'];
                $sua_v_ord[$ord_sua_id]=array('ord'=>$ord_id);
            }
        }
        
        foreach($sua_arr as $sua_id) {
            if ( !is_array($sua_v_ord[$sua_id]) ) {
                $sua_v_ord[$sua_id]=array('ord'=>0);
            }
        }
        
        return $sua_v_ord;
    }
    
    /**
     * get USER_GROUP of MORDER
     * @param object $sqlo
     */
    function get_prod_grp_of_order($sqlo) {
        //TBD:
    }
}

class oMORDER_network_mod {
    
    private $LINK_KEY='sub-order';
    
    function __construct($sqlo, $mord_id) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        if (!$mord_id) {
            $error->set( $FUNCNAME, 1, 'Input-Error: ID not given.' );
            return;
        }
        
        $this->mord_id=$mord_id;
        $this->obj_lib = new oS_OBJLINK_mod();
        $this->obj_lib->setObj($sqlo, 'MORDER', $mord_id);
    }
    
    // add sub order
    function add($sqlo, $new_ord_id) {
        $this->obj_lib->addLink($sqlo, 'MORDER', $new_ord_id, $this->LINK_KEY);
    }
    
    // delete link by MO-ID (MOTHER-ID)
    function del_by_ID($sqlo, $mo_ord_id) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $this->obj_lib = new oS_OBJLINK_mod();
        $this->obj_lib->setObj($sqlo, 'MORDER', $mo_ord_id);
        if ($error->Got(READONLY))  {
            $error->set( $FUNCNAME, 1, 'Error on init.' );
            return;
        }
        $this->obj_lib->removeLinkByObjID($sqlo, 'MORDER', $this->mord_id);
    }
    
    /**
     * add sub order
     * @param object $sqlo
     * @param int $new_ord_id : can NULL
     */
    function update_mother_link($sqlo, $new_ord_id) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;

        $sub_lib = new oMORDER_network($this->mord_id);
        
        if ($new_ord_id==$this->mord_id) {
            $error->set( $FUNCNAME, 1, 'This MOR and Mother-MOR must be different.' );
            return;
        }
        
        $old_mo_id = $sub_lib->get_parent($sqlo);
        if ($old_mo_id>0) {
            $this->del_by_ID($sqlo, $old_mo_id);
        }
        
        if ($new_ord_id) {
            
            $this->obj_lib = new oS_OBJLINK_mod();
            $this->obj_lib->setObj($sqlo, 'MORDER', $new_ord_id);
            if ($error->Got(READONLY))  {
                $error->set( $FUNCNAME, 1, 'Error on init.' );
                return;
            }
            $this->add($sqlo, $this->mord_id);
        }
    }

}
