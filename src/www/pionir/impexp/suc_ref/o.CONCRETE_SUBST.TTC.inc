<?php 
/**
 * manage PROTOCOLS for SUC_REF
 * REF-Bead ==> REF-DOC
 * @package o.CONCRETE_SUBST.TTC.inc
 * @swreq REQ:41 BEADS > REF-Beads > Target-Type CHECKER
 * @author  Steffen Kube (steffen@blink-dx.com)
 * @created 2021-06-14
 * 

 *
 * @var array PROTO_DIFF_STRUCT  difference to Reference of array
 *  [SUC_ID] => array  -- CONCRETE_SUBST_ID
 *    PROTO_NO => array(
 *      'diff'=> array of saingel steps ...
 *        array(  // store only the FIRST difference PER step
 *          'st'=>$step_nr,
 *          'text'=>$diff_txt,
 *          'key'=>$key,
 *          'ref'=>$ref_val,
 *          'now'=>$now_val,
 *          'link_sua_diff'=> [OPTIONAL] array of PROTO_DIFF_STRUCT
 *          )
 *      'ap'=>$pra_id
 *  );
     

 */

require_once ('object.info.inc');
require_once 'o.PROTO.steps.inc';
require_once 'o.ABSTRACT_SUBST.proto.inc';
require_once 'o.CONCRETE_SUBST.proto.inc';
require_once 'o.S_OBJLINK.subs.inc';
require_once 'o.S_VARIO.subs.inc';
require_once 'o.S_VARIO.mod.inc';

require_once dirname(__FILE__).'/o.LINK.TTC.inc';

class o_SUA_TTC_proto {
    function  __construct($sua_id) {
        $this->sua_id = $sua_id;
    }
    
    /**
     * analyse one CONCRETE_PROTO
     * @param object $sqlo
     * @param int $prc_id
     * @return array of
     *   STEP_NR => array()
     */
    private static function one_protocol($sqlo, $pra_id) {
        $all_step_data = gProtoOrg::get_pra_all_steps($sqlo, $pra_id);
        return $all_step_data ;
    }
    
    // return array STEP_NO=array('ap'=>)
    static function get_protolog($sqlo, $sua_id) {
        
        $outarr=array();
        
        $sua_log_lib = new oABSTRACT_SUBST_proto();
        $sua_log_lib->set_obj($sqlo, $sua_id);
        $suc_log = $sua_log_lib->get_protos($sqlo); //  array('ap' => ID of ABSTRACT_PROTO, 'or'=>order number)
        foreach($suc_log as $row) {
            $step_no=$row['or'];
            if ($row['ap']) {
                $outarr[$step_no]=$row;
            }
        }
        return $outarr;
    }
    
    /**
     *
     * @param object $sqlo
     * @return array of
     *  array(
     *     'ap' => ID of ABSTRACT_PROTO,
     *     'or'=>  order number,
     *     'steps'=> array(
     *         STEP => ()
     *     )
     *  )
     */
    static function get_ref_params($sqlo, $sua_id) {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        $outarr=array();
        
        $sua_log_lib = new oABSTRACT_SUBST_proto();
        $sua_log_lib->set_obj($sqlo, $sua_id);
        $suc_log = $sua_log_lib->get_protos($sqlo); //  array('ap' => ID of ABSTRACT_PROTO, 'or'=>order number)
        foreach($suc_log as $row) {
            if ($row['ap']) {
                $outarr[]=$row;
            }
        }
        if (empty($outarr)) return array();
        
        $index_arr = array_keys($outarr);
        foreach($index_arr as $ind) {
            $log_row  = $outarr[$ind];
            $pra_id   = $log_row['ap'];
            $abs_step_data = self::one_protocol($sqlo, $pra_id);
            $outarr[$ind]['steps'] = $abs_step_data;
        }
        
        if ( $_SESSION["userGlob"]["g.debugLevel"]>=2 ) {
            debugOut('(84) outarr: suc_id:'.$sua_id,$FUNCNAME);
            foreach($outarr as $row) {
                echo ' - PROTOCOL: '.$row['or'].' :'."<br>";
                $steps = &$row['steps'];
                foreach($steps as $row) {
                    echo ' --- '.print_r($row,1)."<br>";
                }
            }
        }
        
        
        return $outarr;
    }
    
}

class o_SUC_TTC_proto {
    
    const DIFF_STEP_KEYS=array('sua');
    
    function  __construct($suc_id) {
        $this->suc_id = $suc_id;
    }
    
    /**
     * analyse one CONCRETE_PROTO
     * @param object $sqlo
     * @param int $prc_id
     * @return array of
     *   STEP_NR => array('st'=>, 'suc'=> 'sua'=>, 'q'=>)
     */
    private static function one_protocol($sqlo, $prc_id) {
        
        $all_step_data = gProtoOrg::get_all_data($sqlo, $prc_id, array('CONCRETE_SUBST_ID', 'QUANTITY'));
        if (empty($all_step_data)) return array();
        
        $abs_step_data=array();
        
        foreach($all_step_data as $row) {
            
            $one_step=array();
            $suc_id = $row['CONCRETE_SUBST_ID'];
            if ($suc_id) {
                $sua_id = glob_elementDataGet( $sqlo, 'CONCRETE_SUBST', 'CONCRETE_SUBST_ID', $suc_id, 'ABSTRACT_SUBST_ID');
                $one_step['suc']=$suc_id;
                $one_step['sua']=$sua_id;
            }
            if ($row['QUANTITY']!==NULL) {
                $one_step['q']=$row['QUANTITY'];
            }
            if (!empty($one_step)) {
                $step=$row['STEP_NR'];
                $one_step['st']=$step;
                $abs_step_data[$step]=$one_step;
            }
        }
        return $abs_step_data;
    }
    
    /**
     *
     * @param object $sqlo
     * @return array of
     *  array(
     *     'cp'=>ID of protocol,
     *     'ap' => ID of ABSTRACT_PROTO,
     *     'or'=>  order number,
     *     'steps'=> array(
     *         STEP => ('st'=>, 'suc'=> 'sua'=>, 'q'=>quantity, )
     *     )
     *  )
     */
    static function get_ref_params($sqlo, $suc_id) {
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        $outarr=array();
        
        $suc_log_lib = new oCONCRETE_SUBST_proto($suc_id);
        $suc_log = $suc_log_lib->get_c_protos($sqlo); //  array('cp'=>ID of protocol, 'ap' => ID of ABSTRACT_PROTO, 'or'=>order number, 'ty'=>1,2)
        foreach($suc_log as $row) {
            if ($row['cp']) {
                $outarr[]=$row;
            }
        }
        if (empty($outarr)) return array();
        
        $index_arr = array_keys($outarr);
        foreach($index_arr as $ind) {
            $log_row  = $outarr[$ind];
            $prc_id   = $log_row['cp'];
            $abs_step_data = self::one_protocol($sqlo, $prc_id);
            $outarr[$ind]['steps'] = $abs_step_data;
        }
        
        if ( $_SESSION["userGlob"]["g.debugLevel"]>=3 ) {
            debugOut('(84) outarr: suc_id:'.$suc_id,$FUNCNAME);
            foreach($outarr as $row) {
                echo ' - PROTOCOL: '.$row['or'].' :'."<br>";
                $steps = &$row['steps'];
                foreach($steps as $row) {
                    echo ' --- '.print_r($row,1)."<br>";
                }
            }
        }
        
        
        return $outarr;
    }
    
    /**
     *
     * @param array $all_steps
     * STEP_NR=> array()
     * @param int $step_nr
     * @return array
     */
    static function get_stepdata_by_step(array &$all_steps, $step_nr) {
        //         $datax = array_column($all_steps, 'st');
        //         $ind   = array_search($step_nr, $datax);
        //         if ($ind===FALSE) return;
        return $all_steps[$step_nr];
    }
    
    static function get_steps_by_PRA(&$obj_proto_log, $pra_id) {
        $datax = array_column($obj_proto_log, 'ap');
        $ind   = array_search($pra_id, $datax);
        if ($ind===FALSE) return array();
        $steps = $obj_proto_log[$ind]['steps'];
        if(empty($steps)) $steps=array();
        return $steps;
    }
    static function get_index_by_ORD(&$obj_proto_log, $order_no) {
        $datax = array_column($obj_proto_log, 'or');
        $ind   = array_search($order_no, $datax);
        return $ind;
    }
    
    /**
     * compare steps of ONE protocol  ('st'=>, 'sua'=>, 'q'=>quantity)
     * @param array $ref_steps
     * @param array $this_bead_steps
     * @return array 
     *   'diff'=>$diff, 
     *   'links'=>$linkarr
     */
    static function compare_one_PRA( array &$ref_steps, array &$this_bead_steps ) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $linkarr=array();
        $diff=array();
        if ( $_SESSION["userGlob"]["g.debugLevel"]>=2 ) {
            debugOut('(247) ref_steps:'.print_r($ref_steps,1), $FUNCNAME);
        }
        
        foreach($ref_steps as $ref_row) {
            // get object step
            $step_nr = $ref_row['st'];
            $this_bead_st_data = self::get_stepdata_by_step($this_bead_steps, $step_nr);
            if ( $_SESSION["userGlob"]["g.debugLevel"]>=2 ) {
                debugOut('(265) this_steps:'.print_r($this_bead_st_data,1), $FUNCNAME);
            }
            
            
            if (empty($this_bead_st_data)) {
                $diff[]=array('st'=>$step_nr, 'text'=>'MAC-protocol has no data for step '.$step_nr.'.');
                continue;
            }
            
            $diff_txt=NULL;
            // just compare defined keys like "sua", "q"
            foreach(self::DIFF_STEP_KEYS as $key) {
            
                if (!isset($ref_row[$key])) continue;
                
                $ref_val = $ref_row[$key];
                $now_val = $this_bead_st_data[$key];
                if ($now_val!==$ref_val) {
                    $diff_txt=$key.': expected "'.$ref_val.'", given: "'.$now_val.'"';
                    $diff[]=array('st'=>$step_nr, 'text'=>$diff_txt, 'key'=>$key, 'ref'=>$ref_val, 'now'=>$now_val);
                }
            }
            
            // post analysis
            if ( $ref_row['sua_link']>0 and $ref_row['sua']) {
                if(!$this_bead_st_data['suc']) {
                    $error->set( $FUNCNAME, 1, 'Input SUC missing' );
                    return;
                }
                $linkarr[]=$this_bead_st_data['suc'];
            }
            
        }
        return array('diff'=>$diff, 'links'=>$linkarr);
    }
}

/**
 * the DIFF calculation class
 */
class o_CONCRETE_SUBST_TTC_diff {
    
    private $diff; // PROTO_DIFF_STRUCT
    
    function __construct($sqlo, $suc_id, $doc_ref_id) {
        $this->id = $suc_id;
        $this->doc_ref_id = $doc_ref_id;
        
        
        $this->doc_lib  = new o_LINK_TTC($sqlo, $doc_ref_id);
        $this->doc_ref_data = &$this->doc_lib->get_released_version_data();
    }
    
    /**
     * get protocols calc diff for ONE SUA
     * - call RECURSIVE ....
     * @param object $sqlo
     */
    private function check_protocol_sub(object $sqlo, int $suc_id) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $suc_fea = glob_ObjDataGet( $sqlo, 'CONCRETE_SUBST', $suc_id, array('ABSTRACT_SUBST_ID'));
        $sua_id  = $suc_fea['ABSTRACT_SUBST_ID'];
        
        debugOut('(318) START: SUC-ID:'.$suc_id.' SUA-ID:'.$sua_id, $FUNCNAME, 1);

        if ($this->recursive_cache[$suc_id]) {
            $error->set( $FUNCNAME, 1, 'RECURSIVE call of MAA:'.$sua_id.' detected.' );
            return;
        }
        $this->recursive_cache[$suc_id] = 1;
        $this->diff[$suc_id]=array();
        
        if (!$this->doc_ref_id) {
            $error->set( $FUNCNAME, 2, 'The doc_ref_id is missing!' );
            return;
        }
        
        $this_obj_proto_log = o_SUC_TTC_proto::get_ref_params($sqlo, $suc_id);
        $ref_proto_log_ALL = &$this->doc_ref_data;
        
        // compare ...
       
        if (empty($this_obj_proto_log)) {
            $error->set( $FUNCNAME, 5, 'The MAC-Protocols are empty !' );
            return;
        }
        
        if (empty($ref_proto_log_ALL)) {
            $error->set( $FUNCNAME, 4, 'The '.o_LINK_TTC::REF-DOC.' protocols are empty !' );
            return;
        }
        
        $ref_proto_log = $ref_proto_log_ALL[$sua_id];
        
        debugOut("(352) suc_id:$suc_id sua_id:$sua_id ref_proto_log:".print_r($ref_proto_log,1), $FUNCNAME, 2);
        
        foreach($ref_proto_log as $log_row) {
            $log_step_nr = $log_row['or'];
            $pra_id      = $log_row['ap'];
            $ref_steps   = $log_row['steps'];
            $this_steps  = o_SUC_TTC_proto::get_steps_by_PRA($this_obj_proto_log, $pra_id);
            
            debugOut('(370) pra_id:'.$pra_id.' this_steps:'.print_r($this_steps,1), $FUNCNAME, 1);

            $answer = o_SUC_TTC_proto::compare_one_PRA($ref_steps, $this_steps);
            $one_diff= $answer['diff']; // 'links'
            
            $this->diff[$suc_id][$log_step_nr]=array('diff'=>$one_diff, 'ap'=>$pra_id );
            if (!empty($one_diff)) {
                $error->set( $FUNCNAME, 10, 'Found differences between the protocols!' );
                return;
            }
            
            if (!empty($answer['links'])) {
                foreach($answer['links'] as $suc_id_loop) {
                    $this->check_protocol_sub($sqlo, $suc_id_loop);
                }
            }
            
        }
    }
    
    /**
     * get protocols calc diff
     * @param object $sqlo
     */
    function check_protocol(object $sqlo) {
        
        $suc_id = $this->id ;

        $this->recursive_cache=array();
        $this->diff=array();
        $this->check_protocol_sub($sqlo, $suc_id);
    }
    
    function get_diff() {
        return $this->diff;
    }
}

/**
 * manage one SUC
 * calc DIFFERENCE between SUC and the Config-object
 */
class o_CONCRETE_SUBST_TTC {
    
    
    private $id;           // PRODUCTION BEAD-ID
    private $bead_sua_main;
    private $doc_ref_id;   // SUC-ID of REF-DOC
    
    private $suc_feas;     // SUC features
    // private $def_class_id;    // class of normal BEADS
    private $ref_bead_cls_id; // class of REF BEADS
   
    private $diff; // PROTO_DIFF_STRUCT; build in _protocols_check()
    
    private $doc_lib;
    private $doc_ref_data;
    
    function __construct($sqlo) {
        //global $error;
        //$FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $varcol = & Varcols::get();
        $class_id	= $varcol->class_name_to_id( 'LINK', o_LINK_TTC::DOC_CLASS_REF );

        $this->conf_doc_cls_id   = $class_id;
        $this->conf_doc_SOBJ_KEY = o_LINK_TTC::S_OBJLINK_REF_key;
        $this->conf_doc_MAC_SOBJ_KEY = o_LINK_TTC::S_OBJ_MAC_REF_key;
    }
    
    function set_suc($sqlo, $id) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $this->id = $id;
        $suc_id   = $id;
        $tablename='CONCRETE_SUBST';
        $this->diff=NULL;
        
        $objInfoLib = new objInfoC();
        $this->suc_feas = $objInfoLib->getParamsRaw ( $sqlo, $tablename, $suc_id);
        //$this->suc_feas = glob_ObjDataGet( $sqlo, $tablename, $suc_id, array('ABSTRACT_SUBST_ID'));
        
        // check for MAAs

        $this->doc_ref_id = 0;
        
  
        $doc_ref_id = $this->get_linked_ref_doc($sqlo);
        if ($doc_ref_id) {
            $this->_set_doc_ref($sqlo, $doc_ref_id);
        }
    }
    
    private function _set_doc_ref($sqlo, $doc_conf_id) {
        $this->doc_ref_id=$doc_conf_id;  // register REF-DOC ...
        $this->doc_lib  = new o_LINK_TTC($sqlo, $doc_conf_id);
        $this->doc_ref_data = &$this->doc_lib->get_released_version_data();
        
        $this->doc_ref_features = objInfoC::getParamsRaw ( $sqlo, 'LINK', $doc_conf_id);
    }
    
    /**
     * check the protocols => calculate the difference
     * produce $this->diff, can get it by get_diff_arr()
     * @param object $sqlo
     * @param int $suc_ref_id
     */
    private function _protocols_check(object $sqlo) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
  
        $this->diff=NULL; 
        $diff_calc_lib = new o_CONCRETE_SUBST_TTC_diff($sqlo, $this->id, $this->doc_ref_id);
        $diff_calc_lib->check_protocol($sqlo);
        $this->diff=$diff_calc_lib->get_diff();
   
    }


    private function get_linked_ref_doc($sqlo) {
        // REFACT:OK
        $ref_doc_id=0;
        $ref_bead_arr = oS_OBJLINK_subs::getChiObjByTabOrKey($sqlo, 'CONCRETE_SUBST', $this->id, 'LINK', $this->conf_doc_MAC_SOBJ_KEY);
        if(!empty($ref_bead_arr)) {
            $ref_doc_id=current($ref_bead_arr);
        }
        return $ref_doc_id;
    }
    
    
    // update the PROCUTION-Bead <== REF-DOC
    private function _update($sqlo, $doc_ref_id)  {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $key=$this->conf_doc_MAC_SOBJ_KEY;
        $tablename='CONCRETE_SUBST';
        
        // last checks

        $ref_version = $this->doc_lib->get_released_version_no();
        if (!$ref_version) {
            $error->set( $FUNCNAME, 1, o_LINK_TTC::REF-DOC.' has no Verion info.' );
            return;
        }
        
        $objlink_lib = new oS_OBJLINK_mod();
        $objlink_lib->setObj($sqlo, $tablename, $this->id);
        
        $add_link_flag=1;
        
        $old_val = $objlink_lib->getChiObjByKey($sqlo, $key);
        if (!empty($old_val)) {
            // entry exists ...
            $childarr = current($old_val);
            $old_ref_id = $childarr[1];
            if ($old_ref_id==$doc_ref_id) {
                $add_link_flag = 0;
            } else {
                $objlink_lib->removeLink_by_key($sqlo, $key);
            }
        }
        
        if ($add_link_flag) {
            $objlink_lib->addLink($sqlo, 'LINK', $doc_ref_id, $key);
            if ($error->Got(READONLY))  {
                $error->set( $FUNCNAME, 5, 'Error on adding '.o_LINK_TTC::REF-DOC.' to this MAC.' );
                return;
            }
        }

        // update SUC: REF-DOC version
        $vario_key = o_LINK_TTC::SUC_VARIO_VERSION_key;
        $args=array(
            "vario" => array(
                $vario_key => $ref_version
            )
        );
        $UpdateLib = new globObjUpdate();
        $UpdateLib->update_meta( $sqlo, $tablename, $this->id, $args );
        
    }
    
    /**
     * search REF-DOC CONFIGs
     * @param object $sqlo
     * @return array of DOC-IDs
     */
    function search_matching_config($sqlo) {
        
        $sua_id = $this->suc_feas['vals']['ABSTRACT_SUBST_ID'];
        $doc_ids=array();
        $sql_objlink = 'select '.oS_OBJLINK_subs::sql_getMobyChiTab($sqlo, 'LINK', 'ABSTRACT_SUBST', $sua_id, $this->conf_doc_SOBJ_KEY );
        
        // OLD: ' o.S01='.$sqlo->addQuotes($target_type).
        $sqlsel = 'LINK_ID from LINK x '.
            ' join CCT_ACCESS a on x.CCT_ACCESS_ID=a.CCT_ACCESS_ID'.
            ' join EXTRA_OBJ o on x.EXTRA_OBJ_ID=o.EXTRA_OBJ_ID'.
            ' where  '.
            ' o.EXTRA_CLASS_ID='.$this->conf_doc_cls_id. ' and '.
            ' x.LINK_ID in ('.$sql_objlink.')';
        $sqlo->Quesel($sqlsel);
        while ($sqlo->ReadRow()) {
            $suc_ref_id = $sqlo->RowData[0];
            $doc_ids[]=$suc_ref_id;
        }
        return $doc_ids;
    }
    
    // search a REF-CONFIG
    // REFACT:OK
    private function check0_suc_target_in($sqlo, $doc_conf_id) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        $this->diff=NULL;
        
        
        // REF-DOC found ....
        // check protocols ...
        $this->_set_doc_ref($sqlo, $doc_conf_id);
        
        $this->_protocols_check($sqlo);
        if ($error->Got(READONLY))  {
            $error->set( $FUNCNAME, 20, 'Differences detected.' );
            return;
        }
        
       
        // now it is ok
        $answer = 1;
        return $answer;
    }
    
    /**
     * check only
     * - NO REF-DOC YET LINKED
     * @param object $sqlo
     * @return void|number
     *  1: ok
     * -1  --  no input given
     * -5 : bad error
     */
    function check0_get_configs($sqlo) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        if (!$this->id) throw new Exception('Class not initialized.');
        
        $doc_ids=array();
       
        // 1. get possible documents 
        $doc_ids = $this->search_matching_config($sqlo);
        if (empty($doc_ids)) {
            // $sua_id = $this->suc_feas['vals']['ABSTRACT_SUBST_ID'];
            $error->set( $FUNCNAME, 10, 'No '.o_LINK_TTC::CONFIG_NAME_USER.' found for this MAC' );
            return;
        }
        
        
        return $doc_ids;
    }
    
    /**
     * check given document
     * @param object $sqlo
     * @param int $doc_id
     * @throws Exception
     * @return void|number
     */
    function check0_this_doc($sqlo, $doc_id) {   
        if (!$this->id) throw new Exception('Class not initialized.');
        return $this->check0_suc_target_in($sqlo, $doc_id);
    }
    
    
    /**
     * check after REF-DOC was already linked to SUC ...
     * @param object $sqlo
     */
    function check1_suc($sqlo) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        
        if (!$this->id) throw new Exception('Class not initialized.');
        
//         $xvals = $this->suc_feas["xobj"]["values"];
//         if ($xvals['Target-Type']==NULL) {
//             // no target type given
//             return -1; // no input given
//         }
//         $target_type = $xvals['Target-Type'];
        
        
        $suc_ref_id  = $this->get_linked_ref_doc($sqlo);
        // REF-DOC found ....
        $this->_set_doc_ref($sqlo, $suc_ref_id);
        
        
        // first check
//         $ref_xvals = $this->doc_ref_features['xobj']["values"];
//         $ref_target_type = $ref_xvals['Target-Type'];
//         if ($ref_target_type!=$target_type) {
//             $error->set( $FUNCNAME, 10, 'Bizarre: Target-Type of this bead and the REF-DOC differ!' );
//             return;
//         }

        // check protocols ...
        
        $this->_protocols_check($sqlo);
        if ($error->Got(READONLY))  {
            $error->set( $FUNCNAME, 20, 'Protocol differences detected.' );
            return;
        }
        
        
        // now it is ok
        $answer = 1;
        return $answer;
    }
    
    
    
    /**
     * check and update
     * @param object $sqlo
     
     */
    function update_suc($sqlo) {
        global $error;
        $FUNCNAME= __CLASS__.':'.__FUNCTION__;
        if (!$this->id) throw new Exception('Class not initialized.');
        
        
        if (!$this->doc_ref_id) {
            $error->set( $FUNCNAME, 5, 'No REF-DOC-ID given.' );
            return;
        }
        
        // set link
        $this->_update($sqlo, $this->doc_ref_id);
        
    }
    
    function get_diff_arr() {
        return $this->diff;
    }
    function unset_ref_bead() {
        return  $this->doc_ref_id=0;
    }
    function get_ref_doc() {
        return  $this->doc_ref_id;
    }
    function get_target_type() {
        if (!$this->id) throw new Exception('Class not initialized.');
        $xvals = $this->suc_feas["xobj"]["values"];
        return $xvals['Target-Type'];
    }
    function get_curr_ref_version($sqlo) {
        if (!$this->id) throw new Exception('Class not initialized.');
        return oS_VARIO_sub::getValByTabKey($sqlo, 'CONCRETE_SUBST', $this->id, o_LINK_TTC::SUC_VARIO_VERSION_key);
    }
}


